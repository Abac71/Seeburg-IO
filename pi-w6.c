/**
* Code to decode the pulses from a 1946 Seeburg w6-L56 remote control  into the pressed key combination, transfer of combo to VLC player
* Original code by Phil Lavin <phil@lavin.me.uk>.
* Changes marked by change_abac71
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
#define PIN 21 //change_abac71 from 2 to 21

// How much time a change must be since the last in order to count as a change
#define IGNORE_CHANGE_BELOW_USEC 10000 //change_abac71 from 0.01 to 0.05 sec
// What is the minimum time since the last pulse for a pulse to count as "after the gap"
#define MIN_GAP_LEN_USEC 220000 //change_abac71 from 0.25 to 0.24 sec
// What is the mimimum time since the last pulse for a pulse to count as a new train
#define MIN_TRAIN_BOUNDARY_USEC 1250000 //change_abac71 from 0.5 to 1.25 sec
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
			if (pre_gap_pulses) {
				// 0 base the counts without changing the originals
				pre = pre_gap_pulses - 10;
                                post = post_gap_pulses;

				if (debug)
					printf("Locking\n");

				lock = 1;

				if (debug) {
					printf("Locked\n");
					printf("Before calc. Pre: %d Post: %d\n", pre, post);
				}

				number = pre / 2;								//insert_abac71	
				letter = 'W';								//insert_abac71	

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

// Concernate directory and name of sound file and populate shell command
	sprintf(combo, "%c%d", letter, number);
	char cvlc_cmd[100];																						//insert_abac71
	strcpy(cvlc_cmd, "cvlc --one-instance --playlist-enqueue --play-and-exit /home/pi/Music/w6/");			//insert_abac71
	strcat(cvlc_cmd, combo);																				//insert_abac71
	strcat(cvlc_cmd, " &");																					//insert_abac71
	system(cvlc_cmd);																						//insert_abac71

}

// Returns the time difference, in usec, between two provided struct timevals
unsigned long get_diff(struct timeval now, struct timeval last_change) {
	return (now.tv_sec * 1000000 + now.tv_usec) - (last_change.tv_sec * 1000000 + last_change.tv_usec);
}
