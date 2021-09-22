# Seeburg-IO
Interfacing Rasberry PI to a Seeburg 3w1

Intro

I was inspired by three projects found in the internet. Thanks to Scott, Phil, and Derek for publishing their great work.
https://www.smbaker.com/converting-a-seeburg-3wa-wallbox-into-a-remote-for-a-modern-music-player

https://phil.lavin.me.uk/2013/11/raspberry-pi-project-a-1960s-wallbox-interfaced-with-sonos/

https://hecgeek.blogspot.com/2017/10/wall-o-matic-interface-1.html

I am interfacing the RasPi with my 1950s Seeburg Wall-O-Matic 100 (3w1) using a small board layout for a Pi hat found at Scott's blog. Based on Phil Lavin's Raspberry PI GPIO Interrupt demo code the pulse train gets transferred into the key combination that was pressed. Later on forward the combo to a library of music titles, select right one, and play with VLC player.
The wallbox works on 25VAC. The circuit used (published by Scott M. Baker) to pass pulses at this voltage to the GPIO pins of the required small current (around 2mA) @ 3.3V DC.


Diagram for interfacing PiHat is as follows:

![image](https://user-images.githubusercontent.com/85778633/121767230-50048980-cb57-11eb-9314-8462704bcdc8.png)
SMBaker Seeburg circuit

![image](https://user-images.githubusercontent.com/85778633/133966388-b3921bbc-9de0-4360-9e74-1f9b1a4c414d.png)


Clone the repository and Compiling

The circuit is using WiringPi. In case it is nit installed follow the instructions at http://wiringpi.com/download-and-install/ to install the WiringPi library on your Pi (also published by Phil Lavin).
Use following command to clone the repository: git clone 
Use following command to compile the code to a working program: gcc -lwiringPi -o pi-seeburg pi-seeburg.c


Settings

There are a number of defines at the top of the code. E.g. pin code and timings.
In my case GPIO21 (wiringPi definition) is pin that the wallbox circuit is connected to as well as the timings the code works to. Timings are differing between wall boxes, because of the motor arm RPM, significantly enough that these need tweaking. I am using a US model in Germany. The 50hz frequency in Germany makes the motor rotation slower than in the US. Originaly it has been built for the 60hz frequency. Also depends on the lubrication how fast the motor rotates.


Transferring and decoding the pulse train

The code analyses and decodes the pulse train. The train comprises of a number of pulses, a noticable time gap and a number of additional pulses. The original code from Phil is set up to ignore electrical jitter and pulses unrelated to the train (e.g. when a coin is inserted). As the circuit of SMBaker is much cleaner these sections of the code can be removed because the electrical jitter is filtered by the hardware.

For the combo calculation some code from Derek was helpful as he also used a 3w1 instead a 3w100 (like Phil did).

The first group of pulses has 1-10, 12-21 pulses. The second group has 1-5 pulses. Mapping this information to the selection buttons of the 3w1, ends up into the following sequence:
    A1 ( 1, 1), A2 ( 2, 1), ..., A10 (10, 1)
    B1 (12, 1), B2 (13, 1), ..., B10 (21, 1)
    C1 ( 1, 2), C2 ( 2, 2), ..., C10 (10, 2)
    D1 (12, 2), D2 (13, 2), ..., D10 (21, 2)
    (Note: The letter 'I' is skipped.)


Pass through to VLC player

I used a quite easy way to concernate the file directory and the combo calculated to start the corresponding music file from the raspberry directly.
Using following command is selecting files and queues into a playlist:

cvlc --one-instance --playlist-enqueue --play-and-exit /home/pi/Music/<combo> &


Music database

Music files should be stored in "/home/pi/music". Each music file should be named like combo (e.g. A1, A2, etc.)


Running

- Must be run as root:                                                ./pi-seeburg
- You can run with debug:                                             ./pi-seeburg -d
