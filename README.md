# Seeburg-IO
1st release: Interfacing Rasberry PI (version 3b+) to a Seeburg 3w1 (German Hecker style with extra cash box for housing of Raspi, transformer, etc.)

2nd release: Interfacing Raspberry PI Zero 2 W / Waveshare Soundcard Hat to several Seeburg wallboxes (e.g. 3w1/3w100, 3wA, SC1). Housing in an external encloser -> Jukebox simulator. Switching between different scripts for different pulse trains of the wallboxes is realized. I have used a turn-switch (3 positions) mounted to GPIO16,26,27 for switching between the scripts (back wall of the jukebox simulator). A python script (seeburg-start.py) runs in the background to handle the switch position.  

3rd release: Interfacing Raspberry PI 1.3 / Waveshare Soundcard Hat to a Seeburg Consolette SC1 (Housing inside instead of APU10 - album pricing unit)


Intro

I was inspired by three projects found in the internet. Thanks to Scott, Phil, and Derek for publishing their great work.
https://www.smbaker.com/converting-a-seeburg-3wa-wallbox-into-a-remote-for-a-modern-music-player

https://phil.lavin.me.uk/2013/11/raspberry-pi-project-a-1960s-wallbox-interfaced-with-sonos/

https://hecgeek.blogspot.com/2017/10/wall-o-matic-interface-1.html

I am interfacing the RasPi with different Seeburg wallboxes using a small board layout for a Pi hat found at Scott's blog. Based on Phil Lavin's Raspberry PI GPIO Interrupt demo code the pulse train gets transferred into the key combination that was pressed. Later on forward the combo to a library of music titles, select right one, and play with VLC player.
The Seeburg wallboxes are working on 25VAC. The circuit used (published by Scott M. Baker) to pass pulses at this voltage to the GPIO pins of the required small current (around 2mA) @ 3.3V DC.

![image](https://user-images.githubusercontent.com/85778633/121767230-50048980-cb57-11eb-9314-8462704bcdc8.png)
Picture 1: SMBaker Seeburg circuit

![image](https://user-images.githubusercontent.com/85778633/133966388-b3921bbc-9de0-4360-9e74-1f9b1a4c414d.png)
Picture 2: PiHat for pulse transfer based on SMBaker circuit

![image](https://user-images.githubusercontent.com/85778633/134880764-8301e8b3-980d-4eb3-806d-dec3f7e8aa02.png)
Picture 3: First release realised with German hecker model housing hardware in cashbox

![image](https://user-images.githubusercontent.com/85778633/192135013-3df9e5b9-0da5-4c59-9a43-cb01941560bb.png)
Picture 4: Second release realized in an external encloser for interfacing different Seeburg wallbox types and external speakers (Jukebox simulator).

![image](https://user-images.githubusercontent.com/85778633/167696490-ed3a80ea-5647-4ccc-8ff1-96f35bcd1bf2.png)
Picture 5: Third release for a Seeburg Consolette SC1 using space inside the wallbox where typically the album pricing unit is placed.

Code implementation: Clone the repository and Compiling

The circuit is using WiringPi. In case it is not installed follow the instructions at http://wiringpi.com/download-and-install/ to install the WiringPi library on your Pi ( published by Gordon Henderson). For projects to be realized under Pi4 please see the version update information for release 2.52.

Use following command to clone the repository: git clone https://github.com/Abac71/Seeburg-IO.git

Use following command to compile the code to a working program: gcc pi-seeburg.c -lwiringPi -o pi-seeburg (for 3w1/3w100) or gcc pisc1.c -lwiringPi -o pi-seeburg (for Consolette SC1)


Settings

There are a number of defines at the top of the code. E.g. pin code and timings.
In my case GPIO21 (wiringPi definition) is pin that the wallbox circuit is connected to as well as the timings the code works to. Timings are differing between wall boxes, because of the motor arm RPM, significantly enough that these need tweaking. I am using a US model in Germany. The 50hz frequency in Germany makes the motor rotation slower than in the US. Originaly it has been built for the 60hz frequency. Also depends on the lubrication how fast the motor rotates.


Transferring and decoding the pulse train

The code analyses and decodes the pulse train. The train comprises of a number of pulses, a noticable time gap and a number of additional pulses. The original code from Phil is set up to ignore electrical jitter and pulses unrelated to the train (e.g. when a coin is inserted). As the circuit of SMBaker is much cleaner these sections of the code can be removed because the electrical jitter is filtered by the hardware. For the combo calculation some code from Derek was helpful.

For a 3w1/3w100 the first group of pulses has 1-10, 12-21 pulses. The second group has 1-5 pulses. Mapping this information to the selection buttons of the 3w1/3w100, ends up into the following sequence:    
    A1 ( 1, 1), A2 ( 2, 1), ..., A10 (10, 1)
    B1 (12, 1), B2 (13, 1), ..., B10 (21, 1)
    C1 ( 1, 2), C2 ( 2, 2), ..., C10 (10, 2)
    D1 (12, 2), D2 (13, 2), ..., D10 (21, 2)
    ...
    (Note: The letter 'I' is skipped.)

For a 3wA-200 the first group of pulses has 1-10 pulses. The second group has 1-20 pulses. Mapping this information to the selection buttons of the 3wA, ends up into the following sequence:
    A1 ( 1, 1), A2 ( 2, 1), ..., A10 (10, 1)
    B1 ( 1, 2), B2 ( 2, 2), ..., B10 (10, 2)
    ...
    V1 ( 1, 20), V2 ( 2,20), ...,V10 (10,20)
    (Note: The letters 'I' and 'O' are skipped.)

For a Consolette SC1 the first group of pulses has 1-8 pulses. The second group has 1-20 pulses. Mapping this information to the selection buttons of the Consolette, ends up into the following sequence:
    A1 ( 8,20), A2 ( 7,20), ..., A8 ( 1,20)
    B1 ( 8,19), B2 ( 7,19), ..., B8 ( 1,19)
    ...
    V1 ( 8, 1), V2 ( 7, 1), ..., V8 ( 1, 1)
    (Note: The letters 'I' and 'O' are skipped.)

Because of different pulse trains different scripts have to be started to translate the analog pulse train for the raspi. See the different scripts: pi-3wa, pi-3w1, pi-SC1.

Pass through to VLC player

I used a quite easy way to concernate the file directory and the combo calculated to start the corresponding music file from the raspberry directly.
Using following command is selecting files and queues into a playlist:

cvlc --one-instance --playlist-enqueue --play-and-exit /home/pi/Music/{combo} &


Music database

Music files should be stored in "/home/pi/music". Each music file should be named like combo (e.g. A1, A2, etc.). Just add the directory to a SAMBA server installed on the Raspi so you can easily change the track records. SAMBA server will not work with the PI Zero V1.3 as no WLAN/LAN connection can be established.
![image](https://user-images.githubusercontent.com/85778633/135421903-fba164da-939d-4be4-85f9-c14d69750d88.png)
Picture 6: Format of music files


Running

- As VLC cannot be run under root use automatic log-on with pi and populate ./pi-seeburg using /etc/profile. Configure automatic log-on with sudo raspi-config.
- You can also run in debug mode: ./pi-seeburg -d


Title strips

For the track cards I have used an Excel sheet. Just fill in your track records, print, and cut out. A register for your tracks at once. One file is for the 3w1/3w100 including 100 selection. The other one for the SC1 - 160 possible selection.
