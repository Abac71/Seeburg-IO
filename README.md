# Seeburg-IO
Interfacing Rasberry PI to Seeburg 3w1

Intro

Based on Phil Levin's Raspberry PI GPIO Interrupt demo code. Interfacing the RasPi with my 1950s Seeburg Wall-O-Matic 100 (3w1) to decode the pulse train into the key combination that was pressed. Later on forward the pulse to a library of music titles and select right one.

Interfacing with the PI
The wallbox works on 25V AC. A circuit is used from a hardware project of Scott M. Baker to pass pulses at this voltage to the GPIO pins of the RasPi at the RasPi's required small current (around 2mA) @ 3.3V DC.

Diagram is as follows:
![image](https://user-images.githubusercontent.com/85778633/121767230-50048980-cb57-11eb-9314-8462704bcdc8.png)
SMBaker Seeburg circuit

Compiling
Clone the repository
Follow the instructions at http://wiringpi.com/download-and-install/ to install the WiringPi library on your Pi
gcc -lwiringPi -o pi-seeburg pi-seeburg.c

Settings
There's a number of defines at the top of the code. You can tweak the GPIO pin that the wallbox circuit is connected to as well as the timings the code works to. Timings may differ between wall boxes, because of the motor arm RPM, significantly enough that these need tweaking.

The pulse train
The code analyses and decodes the pulse train. The train comprises of a number of pulses, a noticable time gap and a number of additional pulses. The code is set up to ignore electrical jitter and pulses unrelated to the train (e.g. when a coin is inserted).

On this model wall box, the pulse train is sequential to represent A1 through to K0. If X represents the number of pulses before the gap and Y represents the number of pulses after the gap, X increments from 1 to 20 whilst Y stays at 1. Y then increments and X resets back to 1. The cycle repeats through to X=20, Y=4 for K0.

The decoding is simple maths as shown in the code.

Running
Must be run as root

./pi-seeburg
You can run with debug:

./pi-seeburg -d
You can pass the key combo through to an external programme:

./pi-seeburg -p 'sonos/sonos-wallbox "Dining Room"'
