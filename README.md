# DawnOfAV

A sketch and instructions for emitting NTSC video color patterns and mono audio via arduino

MANDATORY SETUP FOR ARDUINO IDE
//  One time setup - 
    Add the esp32 URL to Arduino IDE with:
    File → Preferences → Additional Boards Manager URLs:
    https://dl.espressif.com/dl/package_esp32_index.json
//  Must use esp32 by Espressif Systems V1.0.4 in boards manager
//  Must use "No OTA (2MB APP/2MB SPIFFS)" or "Huge APP (3MB No OTA/1MB SPIFFS)"
//  In Arduino IDE, select "ESP32 Dev Module" as the board.

Bill of materials 
2 buttons
1 project box
1 toggle switch
1 3.7V rechargable lipo
1 J5019 or similar lipo recharge circuit
1 arduino ESP32 Dev Kit V1

wiring 

composite video jack 
center pin GPIO 25
ground/shell pin GND

audio jack 
pin 1 GND
pin 2 GPIO 27

pattern button - cycles through 11 test patterns in a loop, one move per press
pin 1 GND
pin 2 GPIO 14

audio button - toggle on/off a continuous 1 Khz tone 
pin 1 GND
pin 2 GPIO 13 

J5019 
read the pcb. other than that, the toggle switch should bridge ground of the voltage source to the arduino ground when on. 