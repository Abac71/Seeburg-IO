# Seeburg-IO
Interfacing Rasberry PI to a Seeburg 3w1

Intro

I was inspired by two projects found in the internet. Thanks to Phil and Scott for publishing their great work.
https://www.smbaker.com/converting-a-seeburg-3wa-wallbox-into-a-remote-for-a-modern-music-player
https://phil.lavin.me.uk/2013/11/raspberry-pi-project-a-1960s-wallbox-interfaced-with-sonos/

Based on Phil Levin's Raspberry PI GPIO Interrupt demo code. Interfacing the RasPi with my 1950s Seeburg Wall-O-Matic 100 (3w1) to decode the pulse train into the key combination that was pressed. Later on forward the pulse to a library of music titles and select right one. The wallbox works on 25V AC. A circuit is used from a project published by Scott M. Baker to pass pulses at this voltage to the GPIO pins of the RasPi at the RasPi's required small current (around 2mA) @ 3.3V DC.

Diagram for interfacing is as follows:

![image](https://user-images.githubusercontent.com/85778633/121767230-50048980-cb57-11eb-9314-8462704bcdc8.png)
SMBaker Seeburg circuit

Compiling

Clone the repository

Follow the instructions at http://wiringpi.com/download-and-install/ to install the WiringPi library on your Pi (also by Phil Lavin)
gcc -lwiringPi -o pi-seeburg pi-seeburg.c


Settings

There are a number of defines at the top of the code. E.g. pin code and timings.
In my case GPIO5 is pin that the wallbox circuit is connected to as well as the timings the code works to. Timings are differing between wall boxes, because of the motor arm RPM, significantly enough that these need tweaking. I am using a US model in Germany. The 50hz frequency in Germany makes the motor rotation slower than in the US. Originaly it has been built for the 60hz frequency. Also depends on the lubrication how fast the motor rotates.


The pulse train

The code analyses and decodes the pulse train. The train comprises of a number of pulses, a noticable time gap and a number of additional pulses. The original code from Phil is set up to ignore electrical jitter and pulses unrelated to the train (e.g. when a coin is inserted). As the circuit of SMBaker is much cleaner these sections of the code can be removed because the electrical jitter is filtered by the hardware.

On this model wall box, the pulse train is sequential to represent A1 through to K0. If X represents the number of pulses before the gap and Y represents the number of pulses after the gap, X increments from 1 to 20 whilst Y stays at 1. Y then increments and X resets back to 1. The cycle repeats through to X=20, Y=4 for K0.

The maths was thus (see also in the code):
- Decrement both the pre and post gap counts as 0 is represented by 1 pulse
- The letter is (2 * post-gap-count) + (1 if pre-gap-count is > 10)
- If letter was worked out to be > ‘H’, add 1 as the wallbox doesn’t have an I
- The number is the pre-gap-count modulus 10 (i.e. pre-gap-count % 10)

Running

Must be run as root                                                 ./pi-seeburg
You can run with debug:                                             ./pi-seeburg -d
You can pass the key combo through to an external programme:        ./pi-seeburg -p 'sonos/sonos-wallbox "Dining Room"'

Pass through to Spotify

The key combo will be hand over to ...
