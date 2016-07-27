# ArcticTracker

Arctic Tracker is an APRS tracker platform based on the Teensy 3.2
MCU module, a SR_FRS_1W VHF transceiver module and a ESP8266 WIFI module
(ESP-12) with NodeMCU. The ESP module will function as a WIFI interface, 
a webserver and possibly a storage for data files. A small display
and a PA module will also be condidered. 

See http://www.hamlabs.no for more info about this project. 

This is the firmware for Teensy 3.2 MCU. It uses ChibiOS/RT RTOS. It is partly 
based on the Polaric Tracker firmware source code. Implemented features
include sending/receiving of APRS packets and communication with the
WIFI module. Code for the WIFI module is implemented in LUA/NodeMCU 
and can be found in a separate GIT repository.

Build requirements: 
  * make
  * gcc-arm-none-eabi
  * ChibiOS/RT and ChibiOS/HAL with the contrib addition to add the 
    port for Teensy 3.
    
You will also need a tool to install firmware on a Teensy.
