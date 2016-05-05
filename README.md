# ArcticTracker
APRS tracker based on Teensy 3.1/3.2 and ChibiOS/RT.

This is firmware for the Teensy. 
Build requirements: 
  * make
  * gcc-arm-none-eabi
  * ChibiOS/RT and ChibiOS/HAL with support for Teensy 3.1. 
    See https://github.com/flabbergast/ChibiOS/tree/kinetis
    
You will also need a tool to install firmware on a Teensy.

It is currently work in progress, and not yet a fully functional tracker. 
It is partly based on the Polaric Tracker firmware source code. 
Currently both sending and receiving of APRS packets has been implemented,
tested and works. 

The idea is to connect it to a SR_FRS_1W VHF transceiver module and a 
ESP8266 module, probably ESP12 with NodeMCU. The ESP module will function 
as a WIFI interface, a webserver and perhaps a storage for data files. 

What remains to be done:

* Hardware. Design a PCB. Enclosure. Battery. Antenna. LEDs, button. 
* GPS decoding and tracker functionality (port from Polaric Tracker)
* Digipeater functionality (port from Polaric Tracker)
* Implement WIFI connection and logging on ESP module. With NodeMCU
  this can be done with LUA scripts. Communicate with Teensy 
  over serial or SPI. 
* Consider combined functionality like igate, pushing logs to 
  internet servers, etc.. 