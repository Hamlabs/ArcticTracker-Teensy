
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


THREAD_STACK(Thread1, 164);
extern SerialUSBDriver SDU1;



__attribute__((noreturn))
static THD_FUNCTION(Thread1, arg)
{
    (void)arg;
    
    chRegSetThreadName("LEDBlinker");
    while (TRUE) {
        palSetPad(IOPORT3, PORTC_TEENSY_PIN13);
        sleep(50);
        palClearPad(IOPORT3, PORTC_TEENSY_PIN13);
        sleep(950);
    }
}





#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(700)
/*
 * Application entry point.
 */
int main(void) {   
  
   thread_t *shelltp = NULL;
   halInit();
   chSysInit();
   eeprom_initialize(); 
   usb_initialize();
   shellInit();
 //  radio_init(&RADIO_SERIAL); 

   hdlc_init_encoder(afsk_tx_init());
   afsk_tx_start(); // Call this only when needed and stop it when not needed??

   
   THREAD_START(Thread1, NORMALPRIO+1, NULL);

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
