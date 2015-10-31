
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include <math.h>
#include <stdio.h>
#include "util/eeprom.h"
#include "radio.h"
#include "commands.h"
#include "fbuf.h"
#include "hdlc.h"
#include "afsk.h"
#include "defines.h"
#include "ui.h"


/*************************************************************
 * Set up interrupt driven GPIO
 *************************************************************/

static const EXTConfig extcfg = {
  {
    BUTTON_EXTCFG // , TRX_SQ_EXTCFG
  }
};

static void ext_init() {
   setPinMode(BUTTON, BUTTON_MODE);
   extStart(&EXTD1, &extcfg);
   extChannelEnable(&EXTD1, 0);
//   extChannelEnable(&EXTD1, 1);
}


/******************************************************
 * Application entry point.
 ******************************************************/

int main(void) 
{     
   thread_t *shelltp = NULL;
   
   halInit();
   chSysInit();
   eeprom_initialize(); 
   hdlc_init_encoder(afsk_tx_init());
   afsk_tx_start(); // Call this only when needed and stop it when not needed??
   ext_init();
   usb_initialize();
   radio_init(&TRX_SERIAL);
   ui_init();
   shellInit();

   while (!chThdShouldTerminateX()) {
     if (!shelltp && usb_active())
        shelltp = myshell_start();
     else if (chThdTerminatedX(shelltp)) {
        chThdRelease(shelltp);    
        shelltp = NULL;   
     }       
     chThdSleepMilliseconds(1000);
   }

   
   return 0;
}
