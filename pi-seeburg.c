/**
* Code to to decode the pulses from a 1960s Seeburg Wall-O-Matic 100 into the pressed key combination
* by Phil Lavin <phil@lavin.me.uk>.
*
* Released under the BSD license.
*
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <wiringPi.h>

// Which GPIO pin we're using
#define PIN 2

// How much time a change must be since the last in order to count as a change
#define IGNORE_CHANGE_BELOW_USEC 10000
// What is the minimum time since the last pulse for a pulse to count as "after the gap"
#define MIN_GAP_LEN_USEC 250000
// What is the mimimum time since the last pulse for a pulse to count as a new train
#define MIN_TRAIN_BOUNDARY_USEC 400000 // 0.4 sec
// How often to update the last change value to stop diff overflowing
#define OVERFLOW_PROTECTION_INTERVAL_USEC 60000000 // 60 secs

// Time of last change
struct timeval last_change;
// Which side of "the gap" we're on
int pre_gap = 1;
// Pulse counters
int pre_gap_pulses = 0;
int post_gap_pulses = 0;
// Settings
char *pass_to = NULL;
int debug = 0;
// Locked?
int lock = 0;

// Predefines
unsigned long get_diff(struct timeval now, struct timeval last_change);
void handle_gpio_interrupt(void);
void handle_key_combo(char letter, int number);

int main(int argc, char **argv) {
	int c;
	struct timeval now;
	unsigned long diff;
	char letter;
	int number;
	int pre, post;

	// CLI Params
	while ((c = getopt(argc, argv, "dp:")) != -1) {
		switch (c) {
			case 'd':
				debug = 1;
				break;
			// Programme to pass the generated key combo to for handling
			case 'p':
				pass_to = strdup(optarg);
				break;
		}
	}

	// Init
	wiringPiSetup();

	// Set pin to output in case it's not
	pinMode(PIN, OUTPUT);

	// Init last change to be now
	gettimeofday(&last_change, NULL);

	// Bind to interrupt
	wiringPiISR(PIN, INT_EDGE_BOTH, &handle_gpio_interrupt);

	// The loop...
	for (;;) {
		// Time now
		gettimeofday(&now, NULL);

		// Time difference in usec
		diff = get_diff(now, last_change);

		// Should reset counters?
		if (diff > MIN_TRAIN_BOUNDARY_USEC) {
			// Have we registered a full pulse train (i.e. seen a gap)?
			if (!pre_gap && pre_gap_pulses && post_gap_pulses) {
				// 0 base the counts without changing the originals
				pre = pre_gap_pulses - 1;
				post = post_gap_pulses - 1;

				if (debug)
					printf("Locking\n");

				lock = 1;

				if (debug) {
					printf("Locked\n");
					printf("Before calc. Pre: %d Post: %d\n", pre, post);
				}

				// Calc the key combination...
				letter = 'A' + (2 * post) + (pre > 10); // A plus the offset plus 1 more if pre gap pulses > 10
				letter += (letter > 'H'); // Hax for missing I
				number = pre % 10;

				// Hand off to the handler
				handle_key_combo(letter, number);
			}

			// Reset counters
			if (pre_gap_pulses || post_gap_pulses) {
				if (debug)
					printf("Reset! %lu\n", diff);

				pre_gap_pulses = 0;
				post_gap_pulses = 0;
				pre_gap = 1;
			}

			if (lock) {
				if (debug)
					printf("Unlocking\n");

				lock = 0;

				if (debug)
					printf("Unlocked\n");
			}
		}

		// Should update time to stop diff overflowing?
		if (diff > OVERFLOW_PROTECTION_INTERVAL_USEC) {
			if (debug)
				printf("Overflow protection\n");

			gettimeofday(&last_change, NULL);
		}

		// Waste time but not CPU whilst still allowing us to detect finished pulses
		usleep(10000);
	}

	if (pass_to) {
		free(pass_to);
	}

	return 0;
}

// Handler for interrupt
void handle_gpio_interrupt(void) {
	struct timeval now;
	unsigned long diff;

	if (!lock) {
		gettimeofday(&now, NULL);

		// Time difference in usec
		diff = get_diff(now, last_change);

		// Filter jitter
		if (diff > IGNORE_CHANGE_BELOW_USEC) {
			// Should switch to post gap? It's a gap > gap len but less than train boundary. Only when we're doing numbers, though.
			if (pre_gap && diff > MIN_GAP_LEN_USEC && diff < MIN_TRAIN_BOUNDARY_USEC) {
				pre_gap = 0;
			}

			// Increment the right counter
			if (pre_gap) {
				pre_gap_pulses++;
			}
			else {
				post_gap_pulses++;
			}

			if (debug)
				printf("Pulse! Pre: %d Post: %d Diff: %lu\n", pre_gap_pulses, post_gap_pulses, diff);
		}

		// Record when the last change was
		last_change = now;
	}
	else {
		printf("Locked. Ignoring interrupt\n");
	}
}

// Handler for the completed key combination
void handle_key_combo(char letter, int number) {
	char combo[3];
	char *sys_cmd;

	printf("Combo: %c%d\n", letter, number);

	if (pass_to) {
		// Make a string representation of our key combo
		sprintf(combo, "%c%d", letter, number);

		// Concat the supplied command and the key combo
		sys_cmd = strdup(pass_to);
		sys_cmd = realloc(sys_cmd, strlen(sys_cmd) + sizeof(combo)); // Cause we lose a \0 we don't need to add 1 for the space
		strcat(sys_cmd, " \0");
		strcat(sys_cmd, combo);

		// Run the command. Return 0 is good.
		if (!system(sys_cmd)) {
			printf("Passed key combo through to the specified programme\n");
		}

		// Can has memory?
		free(sys_cmd);
	}
}

// Returns the time difference, in usec, between two provided struct timevals
unsigned long get_diff(struct timeval now, struct timeval last_change) {
	return (now.tv_sec * 1000000 + now.tv_usec) - (last_change.tv_sec * 1000000 + last_change.tv_usec);
}
