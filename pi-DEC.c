/**
* Code to convert the digital pulse from a Seeburg DEC into a Seeburg Consolette SC1 key combination, transfer of combo to VLC player
* Original code by Joop van den Heuvel in Arduino c++. Thanks for providing the code together with the converter board.
* Released under the BSD license.
*
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <wiringPi.h>

// Which GPIO pins we're using:

#define DEC_A0_IN 22  // GPIO22 DEC A 0 
#define DEC_B1_IN 23  // GPIO23 DEC B 1 
#define DEC_C2_IN 24  // GPIO24 DEC C 2 
#define DEC_D3_IN 25  // GPIO25 DEC D 3 

//Init
    wiringPiSetup();

// Set pins definition to input:

    pinMode(DEC_A0_IN, INPUT);  // DEC A 22
    pullUpDnControl(DEC_A0_IN, PUD_UP);    
    pinMode(DEC_B1_IN, INPUT);  // DEC B 23
    pullUpDnControl(DEC_B1_IN, PUD_UP);
    pinMode(DEC_C2_IN, INPUT);  // DEC C 24
    pullUpDnControl(DEC_C2_IN, PUD_UP);
    pinMode(DEC_D3_IN, INPUT);  // DEC D 25
    pullUpDnControl(DEC_D3_IN, PUD_UP);


// The waitloop waiting for signalpulse to happen:

    if (config.input_device == DEVICE_DEC_D) {
      now = millis();
      do {
        delayMicroseconds(15);  // 15 to 20; 15 for faster response
        dec_check = (int)((int)(1 - digitalRead(DEC_A0_IN)) + (int)(1 - digitalRead(DEC_B1_IN)) + (int)(1 - digitalRead(DEC_C2_IN)) + (int)(1 - digitalRead(DEC_D3_IN)));
      } while (((millis() - now) < (unsigned long)timeout) and (dec_check == 0) and (digitalRead(INPINCANCEL) != HIGH));
    }


===================================================================================
... read dec pulse:


byte ReadDecPulse() {
  byte quadribit = 0;
  byte quadribit_previous;
  int i = 0;
  delayMicroseconds(5);  // debounce 5ms
  do {
    quadribit_previous = quadribit;
    int dec_input_a0 = digitalRead(DEC_A0_IN);
    int dec_input_a1 = digitalRead(DEC_B1_IN);
    int dec_input_a2 = digitalRead(DEC_C2_IN);
    int dec_input_a3 = digitalRead(DEC_D3_IN);
    quadribit = (8 * (1 - dec_input_a3)) + (4 * (1 - dec_input_a2)) + (2 * (1 - dec_input_a1)) + (1 - dec_input_a0);
    delayMicroseconds(10);  // debounce 10ms
  } while ((quadribit != quadribit_previous) and (i++ <= 3));  // ensures read when signal stable
  return (quadribit);
}

========================================================================
... wait for DEC inputs to get to certain level until timout happens:

int WaitDecLevel(int level, unsigned int timeout) {
  long counts = 0;
  int any_signal;
  //
    do {
      delayMicroseconds(1);
      counts++;
      if ((digitalRead(DEC_A0_IN) + digitalRead(DEC_B1_IN) + digitalRead(DEC_C2_IN) + (digitalRead(DEC_D3_IN))) != 4) {
        any_signal = LOW;
      } else
        any_signal = HIGH;
    } while ((any_signal != level) and (counts < timeout));

  return (counts);
}

==============================================================================
... read content of DEC pulse:

    if ((config.input_device==DEVICE_DEC_D) and (dec_input)) {
      dec_byte1 = ReadDecPulse();
      delay_counter_1 = WaitDecLevel(HIGH, 10000);
      if (delay_counter_1 < 1000) {
        delay_counter_2 = WaitDecLevel(LOW, 10000);
        dec_byte2 = ReadDecPulse();
        delay_counter_3 = WaitDecLevel(HIGH, 10000);
        delay_counter_4 = WaitDecLevel(LOW, 10000);
        dec_byte3 = ReadDecPulse();
      } else {
        SerialPrintLogging("Coin inserted 0x%2.2x", dec_byte1);
        if (dec_byte1==0x0C) {
          DisplayText(0, MODE_LOG, "25c inserted");  // added line  letter and digit
        }
        if (dec_byte1==0x0D) {
          DisplayText(0, MODE_LOG, "10c inserted");  // added line  letter and digit
        }
        dec_byte1 = dec_byte2 = dec_byte3 = 0;
      }
      SerialPrintLogging("DEC_S received three bytes  %d %d %d", dec_byte1, dec_byte2, dec_byte3);  // reading 3 pulses
      SerialPrintLogging("DEC_S received converted  %d %d %d", dec_conversion_list[dec_byte1], dec_conversion_list[dec_byte2], dec_conversion_list[dec_byte3]);  // reading 3 pulses
      SerialPrintLogging("DEC_S delay counters 1>L:%d >H:%d 2>L:%d >H%d", delay_counter_1, delay_counter_2, delay_counter_3, delay_counter_4);  // reading 3 pulses
      wallbox_number_index = (100 * dec_conversion_list[dec_byte1]) + (10 * dec_conversion_list[dec_byte2]) + dec_conversion_list[dec_byte3];
      SerialPrintLogging("DEC_S received # %d", wallbox_number_index);  // reading 3 pulses
      dec_byte1 = dec_byte2 = dec_byte3 = 0;
      if ((wallbox_number_index >= 100) and (wallbox_number_index < 280)) {
        input_received_status = VALID;
        wallbox_letter_index = 0;
      }
    }


