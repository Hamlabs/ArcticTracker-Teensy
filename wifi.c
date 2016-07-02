#include "ch.h"
#include "hal.h"
#include "defines.h"
#include "config.h"
#include "chprintf.h"
#include <stdio.h>
#include "string.h"
#include "util/shell.h"
#include "commands.h"
#include "wifi.h"


// static bool mon_on = false;
static Stream*  _serial;
static Stream*  _shell = NULL;
static uint16_t wifiEnabled = 0;

static void wifi_command(void);
static void cmd_getParm(char* p);
static void cmd_setParm(char* p, char* val);


THREAD_STACK(wifi_monitor, 256);


static const SerialConfig _serialConfig = {
  115200
};


void wifi_enable() {
   if (wifiEnabled++ == 0)
      setPin(WIFI_ENABLE);
}

void wifi_disable() {
  if (--wifiEnabled == 0)
      clearPin(WIFI_ENABLE);
}

void wifi_external() {
  setPinMode(WIFI_SERIAL_RXD, PAL_MODE_UNCONNECTED);
  setPinMode(WIFI_SERIAL_TXD, PAL_MODE_UNCONNECTED);
}

void wifi_internal() {
  setPinMode(WIFI_SERIAL_RXD, PAL_MODE_ALTERNATIVE_3);
  setPinMode(WIFI_SERIAL_TXD, PAL_MODE_ALTERNATIVE_3);
}


/**************************************************************
 * Connect to shell on WIFI module
 **************************************************************/

void wifi_shell(Stream* chp) {
  wifi_internal();
  wifi_enable();
  sleep(200);
  _shell = chp;
  while (true) { 
    char c; 
    if (streamRead(chp, (uint8_t *)&c, 1) == 0)
      break;
    if (c == 4) {
      chprintf(chp, "^D");
      break;
    }
    streamPut(_serial, c);
  }
  _shell = NULL;
  wifi_disable();
}


/* NOTE: Should check thread safety when using this */
static char cbuf[64]; 

/**************************************************************
 * Connect to shell on WIFI module
 **************************************************************/

static void cmd_getParm(char* p) { 
  if (strcmp("MYCALL", p) == 0) {
     addr_t x;
     GET_PARAM(MYCALL, &x);
     chprintf(_serial, "%s\r\n", addr2str(cbuf, &x)); 
  }
  else if (strcmp("DEST", p) == 0) {
    addr_t x;
    GET_PARAM(DEST, &x);
    chprintf(_serial, "%s\r\n", addr2str(cbuf, &x)); 
  }
  else if (strcmp("DIGIS", p) == 0) {
    __digilist_t digis;
    GET_PARAM(DIGIS, &digis);
    uint8_t n = GET_BYTE_PARAM(NDIGIS);
    chprintf(_serial, "%s\r\n", digis2str(cbuf, n, digis)); 
  }
  else if (strcmp("TRX_TX_FREQ", p) == 0) {
    uint32_t x;
    GET_PARAM(TRX_TX_FREQ, &x);
    chprintf(_serial, "%lu\r\n", x);
  }
  else if (strcmp("TRX_RX_FREQ", p) == 0) {
    uint32_t x;
    GET_PARAM(TRX_RX_FREQ, &x);
    chprintf(_serial, "%lu\r\n", x);
  }
}


static void cmd_setParm(char* p, char* val)
  { chprintf(_serial, "OK\r\n"); }



/*************************************************************************** 
 * Get and execute a command from the WIFI module
 * 
 * FIXME: There should be a mutex that prevents the shell from 
 * being started in the middle of executing a command.
 *
 * A command is one character followed by arguments and ended with a newline
 * Get parameter: #R PARM
 * Set parameter: #W PARM VALUE 
 ***************************************************************************/

static void wifi_command() {
   char line[32];
   char *tokp; 
   
   readline(_serial, line, 32);
   if (line[0] == 'R') 
      /* Read parameter */
      cmd_getParm((char*) _strtok((char*) line+1, " ", &tokp));
   if (line[0] == 'W')
      /* Write parameter */
      cmd_setParm((char*) _strtok((char*) line+1, " ", &tokp), (char*) _strtok(NULL, " ", &tokp));
}



/*
 * Main thread to get characters from the WIFI module over 
 * the serial line. 
 */
static THD_FUNCTION(wifi_monitor, arg)
{
   (void) arg;
   chRegSetThreadName("WIFI module monitor");
   while (true) {  
      char c;
      if (streamRead(_serial, (uint8_t *)&c, 1) != 0) {
        if (_shell != NULL)
           /* If shell is active, just pass character on to the shell */
           streamPut(_shell, c);
        else if (c == '#')
          /* If shell is not active and if the incoming character is a #, it is
           * initiating a command. All other characters are ignored.  
           */
          wifi_command();
      }
   }
}



void wifi_init(SerialDriver* sd)
{  
  _serial = (Stream*) sd;
  wifi_internal();
  clearPin(WIFI_ENABLE);
  wifiEnabled = 0;
  sdStart(sd, &_serialConfig);   
  THREAD_START(wifi_monitor, NORMALPRIO+2, NULL);
}

