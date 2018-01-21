
Warning! Files included in the zip package are the HD44780 Library. It's not my autorship, and it's under own proprietary license.

# Second-D High Power PWM driver for low-resistance loads.
Warning! Please use copper with core which have min 2mm diameter for power circuit. 

Reinforcement:
2x 18650 high-current Cell(at least 20A)

Features:
-Program lock
-30kHz or 4kHz pwm mode
-LCD Display
-Menu
-Voltometer
-Amperometer(value readed from voltage decrease, calibratable from menu)
-Anti-short(also based on voltage decrease, calibratable from menu)
-PWM mode
-Preheat mode(configurable time and power)
-Mechanical mode
-Can pull Loads like 0.2Ohm
-Up to 250W Power

How to make this:
Open hardware folder, there is a schematic. Build it.
Use Eclipse with AVR plugin.
Make new AVR project
Add files with source code and display library to main project folder
Go to project settings>C/C++ Build>Settings>AVR C Linker.
In general, add this other arguments: "-Wl,-u,vfprintf "
In Libraries, add "m" and "printf_flt".
Set fuse bits for low as "E2"
Set MCU Clock frequency as "8000000"
Build project and load to atmega via your programmer.

Inteface:
LEDs: You have two leds. Green standby and red warming. First blinks if device is locked, and shines when unlocked. Second one shines only on warming, or setting duty.
Buttons: You have three buttons. Fire, plus and minus.
Display: You have LCD alphanumeric display with 2 lines and 8 colums.

How to use:
If you put in cells, Display will light up and show you the splash screen. Also both Leds will light up. It's made for show you, thats all works correctly. after this Driver goes to lock mode. It's set by default after put in cells.

Unlock device by press and hold both fire and minus button, until you see again splash screen.

Set display contrast using potentiometer.

at first run after programming, you must set amperometer and max decrease.

Run menu by pressing fire and minus button. 

Using + and - go to option "c" and press fire.

hold - and fire for restore default cell indicator value. If it's appear, press fire.

Go again to the meu and repeat that for the "a" option, setting max decrease.

Press fire and check, that powet crcuit works correctly.

Connect load with a known resistance, and calibrate amperometer. After that press fire for save that.

Set maximum voltage dropout at which driver have to cut off power circuit. Remember, that its checked for the maximum possible duty. I recommned first set it for low value and test, if it works correctly. After that press fire for save that.

Now driver are ready to use.

In menu except a and c you have lock(L) and three run modes:
N- PWM mode
P- preheat mode. After choosing this, you have to set preheat duty and preheat time.
M- mechanical mode. Note that in this mode anti short system is turned off.

Chooset L for lock device.

Enjoy!


