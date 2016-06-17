#include "ch.h"
#include "hal.h"
#include "defines.h"
#include "config.h"
#include "chprintf.h"
#include <stdio.h>
#include "string.h"

// static bool mon_on = false;
static Stream*  _serial;
static Stream*  _shell = NULL;


THREAD_STACK(wifi_monitor, 64);


static const SerialConfig _serialConfig = {
  115200
};



/**************************************************************
 * Connect to shell on WIFI module
 **************************************************************/

void wifi_shell(Stream* chp) {
  chprintf(chp, "***** WIFI DEVICE. Ctrl-D to exit *****\r\n");
  setPin(WIFI_ENABLE);
  sleep(200);
  _shell = chp;
  while (true) { 
    char c; 
    if (streamRead(chp, (uint8_t *)&c, 1) == 0)
      return;
    if (c == 4) {
      chprintf(chp, "^D");
      return;
    }
    streamPut(_serial, c);
  }
  _shell = NULL;
  clearPin(WIFI_ENABLE);
}



static THD_FUNCTION(wifi_monitor, arg)
{
   (void) arg;
   chRegSetThreadName("WIFI module monitor");
   while (true) {  
      char c;
      if (streamRead(_serial, (uint8_t *)&c, 1) != 0)
        if (_shell != NULL)
          streamPut(_shell, c);
   }
}



void wifi_init(SerialDriver* sd)
{  
  _serial = (Stream*) sd;
  setPinMode(WIFI_SERIAL_RXD, PAL_MODE_ALTERNATIVE_3);
  setPinMode(WIFI_SERIAL_TXD, PAL_MODE_ALTERNATIVE_3);
  clearPin(WIFI_ENABLE);
  sdStart(sd, &_serialConfig);   
  THREAD_START(wifi_monitor, NORMALPRIO+1, NULL);
}

