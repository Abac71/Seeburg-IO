# Import Bibliothek
from gpiozero import Button
from signal import pause
from os import system

#Initialisierung GPIO16, 26 und 27
button3w1 = Button(27)
button3wA = Button(16)
buttonSC1 = Button(26)

#Funktion definieren
def start_3w1():
  print("Script pi-3w1 is starting")
  system("/home/pi/Seeburg-IO/./pi-3w1")
def start_3wA():
  print("Script pi-3wa is starting")
  system("/home/pi/Seeburg-IO/./pi-3wa")
def start_SC1():
  print("Script pi-sc1 is starting")
  system("/home/pi/Seeburg-IO/./pi-sc1")

#Selektion auslesen (Starten einer Funktion, abhängig von der Position des Drehschalters)
#Position Links (GPIO..) -> SC1
#Position Mitte (GPIO..) -> 3wA
#Position Rechts (GPIO..) -> 3w1/3w100

if button3w1.is_pressed:
   start_3w1()
else:
  if button3wA.is_pressed:
    start_3wA()
  else:
    if buttonSC1.is_pressed:
      start_SC1()
    else:
      print ("No selection.")

pause()
