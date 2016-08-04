#include "ch.h"
#include "hal.h"
#include "defines.h"
#include "config.h"
#include "chprintf.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "util/shell.h"
#include "commands.h"
#include "text.h"
#include "wifi.h"


// static bool mon_on = false;
static Stream*  _serial;
static Stream*  _shell = NULL;
static bool wifiEnabled = false;

static void wifi_command(void);
static void cmd_getParm(char* p);
static void cmd_setParm(char* p, char* val);
static void wifi_start_server(void);
char* parseFreq(char* val, char* buf, bool tx);

MUTEX_DECL(wifi_mutex);
#define MUTEX_LOCK chMtxLock(&wifi_mutex)
#define MUTEX_UNLOCK chMtxUnlock(&wifi_mutex)

BSEMAPHORE_DECL(response_pending, true);
#define WAIT_RESPONSE chBSemWait(&response_pending)
#define SIGNAL_RESPONSE chBSemSignal(&response_pending)


THREAD_STACK(wifi_monitor, STACK_WIFI);


static const SerialConfig _serialConfig = {
  115200
};


void wifi_enable() {
   if (!wifiEnabled) {
      wifiEnabled = true;
      SET_BYTE_PARAM(WIFI_ON, 1);
      setPin(WIFI_ENABLE);
      sleep(2000);
//      wifi_start_server();
   }
}

void wifi_disable() {
  if (wifiEnabled)
      clearPin(WIFI_ENABLE);
  wifiEnabled = false; 
  SET_BYTE_PARAM(WIFI_ON, 0);
}


bool wifi_is_enabled() {
  return wifiEnabled;
}


void wifi_restart() {
  if (wifiEnabled) {
    wifi_disable();
    sleep(100);
    wifi_enable(); 
  }
}


void wifi_external() {
  setPinMode(WIFI_SERIAL_RXD, PAL_MODE_UNCONNECTED);
  setPinMode(WIFI_SERIAL_TXD, PAL_MODE_UNCONNECTED);
}

void wifi_internal() {
  setPinMode(WIFI_SERIAL_RXD, PAL_MODE_ALTERNATIVE_3);
  setPinMode(WIFI_SERIAL_TXD, PAL_MODE_ALTERNATIVE_3);
}


/***************************************************************
 * Start (or resume) server on WIFI module
 ***************************************************************/

static void wifi_start_server() {
  sleep(200);
  char uname[32], passwd[32];
  GET_PARAM(HTTP_USER, uname);
  GET_PARAM(HTTP_PASSWD, passwd);
  if (GET_BYTE_PARAM(HTTP_ON))
    chprintf(_serial, "start_http_server('%s','%s')\r", uname, passwd);
  sleep(100);
  chprintf(_serial, "coroutine.resume(listener)\r");
  sleep(100);
}



/***************************************************************
 * Invoke command on WIFI module
 ***************************************************************/

static char** client_buf;
static bool client_active = false; 


char* wifi_doCommand(char* cmd, char* buf) {
  if (wifi_is_enabled()) {
     MUTEX_LOCK;
     chprintf(_serial, "%s\r", cmd);
     client_active=true;
     client_buf = &buf; 
     WAIT_RESPONSE;
     client_active=false;
     MUTEX_UNLOCK;
  }
  else
    sprintf(buf, "-");
  return buf; 
}

char* wifi_status(char* buf) {
   char res[8];
   int n;
   wifi_doCommand("STATUS", res);
   n = atoi(res);
   switch(n) {
     case 0: strcpy(buf, "Idle"); break;
     case 1: strcpy(buf, "Connecting.."); break;
     case 2: strcpy(buf, "Wrong password"); break;
     case 3: strcpy(buf, "AP not found"); break;
     case 4: strcpy(buf, "Failed"); break;
     case 5: strcpy(buf, "Connected ok"); break;
   }
   return buf;
}
  
  
  
/**************************************************************
 * Connect to shell on WIFI module
 **************************************************************/

void wifi_shell(Stream* chp) {
  bool was_enabled = wifi_is_enabled();
  wifi_enable();
  
  MUTEX_LOCK;
  chprintf(_serial, "SHELL\r\r");
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
  chprintf(_serial, "\r");
  wifi_start_server();
  MUTEX_UNLOCK;
  
  if (!was_enabled)
      wifi_disable();
}



/* FIXME: Should check thread safety when using this */
static char cbuf[129]; 

/*****************************************************************
 * Process commands coming from WIFI module that read parameters
 *****************************************************************/

static void cmd_getParm(char* p) { 
   if (strcmp("MYCALL", p) == 0) {
      addr_t x;
      GET_PARAM(MYCALL, &x);
      chprintf(_serial, "%s\r", addr2str(cbuf, &x)); 
   }
   else if (strcmp("DEST", p) == 0) {
      addr_t x;
      GET_PARAM(DEST, &x);
      chprintf(_serial, "%s\r", addr2str(cbuf, &x)); 
   }
   else if (strcmp("DIGIS", p) == 0) {
      __digilist_t digis;
      GET_PARAM(DIGIS, &digis);
      uint8_t n = GET_BYTE_PARAM(NDIGIS);
      chprintf(_serial, "%s\r", digis2str(cbuf, n, digis)); 
   }
   else if (strcmp("REPORT_COMMENT", p) == 0) {
      GET_PARAM(REPORT_COMMENT, cbuf);
      chprintf(_serial, "%s\r", cbuf);
   }
   else if (strcmp("SYMBOL", p) == 0) {
       char tab = GET_BYTE_PARAM(SYMBOL_TAB);
       char sym = GET_BYTE_PARAM(SYMBOL); 
       chprintf(_serial, "%c%c\r", tab, sym); 
   }
   else if (strcmp("TIMESTAMP", p) == 0)
     chprintf(_serial, "%s\r", PRINT_BOOL(TIMESTAMP_ON, cbuf));
   
   else if (strcmp("COMPRESS", p) == 0)
     chprintf(_serial, "%s\r", PRINT_BOOL(COMPRESS_ON, cbuf));
   
   else if (strcmp("ALTITUDE", p) == 0)
     chprintf(_serial, "%s\r", PRINT_BOOL(ALTITUDE_ON, cbuf));
   
   else if (strcmp("TRX_TX_FREQ", p) == 0) {
      uint32_t x;
      GET_PARAM(TRX_TX_FREQ, &x);
      chprintf(_serial, "%lu\r", x);
   }
   else if (strcmp("TRX_RX_FREQ", p) == 0) {
      uint32_t x;
      GET_PARAM(TRX_RX_FREQ, &x);
      chprintf(_serial, "%lu\r", x);
   }
   else if (strcmp("TRACKER_TURN_LIMIT", p) == 0) {
      uint16_t x; 
      GET_PARAM(TRACKER_TURN_LIMIT, &x);
      chprintf(_serial, "%u\r", x);
   }
   else if (strcmp("TRACKER_MAXPAUSE", p) == 0)
      chprintf(_serial, "%u\r", GET_BYTE_PARAM(TRACKER_MAXPAUSE));
   
   else if (strcmp("TRACKER_MINPAUSE", p) == 0)
      chprintf(_serial, "%u\r", GET_BYTE_PARAM(TRACKER_MINPAUSE));
   
   else if (strcmp("TRACKER_MINDIST", p) == 0)
     chprintf(_serial, "%u\r", GET_BYTE_PARAM(TRACKER_MINDIST));
   
   else if (strcmp("HTTP_ON", p) == 0)
     chprintf(_serial, "%s\r", PRINT_BOOL(HTTP_ON, cbuf));
   else
      chprintf(_serial, "ERROR. Unknown setting\r");
}



/*****************************************************************
 * Process commands coming from WIFI module that write parameters
 *****************************************************************/

static void cmd_setParm(char* p, char* val) { 
    if (strcmp("MYCALL", p) == 0) {
       addr_t x;
       str2addr(&x, val, false);
       SET_PARAM(MYCALL, &x);
       chprintf(_serial, "OK\r"); 
    }
    else if (strcmp("DEST", p) == 0) {
       addr_t x;
       str2addr(&x, val, false);
       SET_PARAM(DEST, &x);
       chprintf(_serial, "OK\r"); 
    }
    else if (strcmp("DIGIS", p) == 0) 
       chprintf(_serial, "%s\r", parseDigipath(val, cbuf));
      
    else if (strcmp("SYMBOL", p) == 0) 
       chprintf(_serial, "%s\r", parseSymbol(val, cbuf));
    
    else if (strcmp("REPORT_COMMENT", p) == 0) {
      /* FIXME: Sanitize input */ 
      SET_PARAM(REPORT_COMMENT, val);
       chprintf(_serial, "OK\r");
    }
    else if (strcmp("TIMESTAMP", p) == 0)
       chprintf(_serial, "%s\r", PARSE_BOOL(TIMESTAMP_ON, val, cbuf));  

    else if (strcmp("COMPRESS", p) == 0)
      chprintf(_serial, "%s\r", PARSE_BOOL(COMPRESS_ON, val, cbuf));  

    else if (strcmp("ALTITUDE", p) == 0)
      chprintf(_serial, "%s\r", PARSE_BOOL(ALTITUDE_ON, val, cbuf));  
    
    else if (strcmp("TRX_TX_FREQ", p) == 0) 
       chprintf(_serial, "%s\r", parseFreq(val, cbuf, true));
    
    else if (strcmp("TRX_RX_FREQ", p) == 0) 
       chprintf(_serial, "%s\r", parseFreq(val, cbuf, false));
    
    else if (strcmp("TRACKER_TURN_LIMIT", p) == 0) 
       chprintf(_serial, "%s\r", parseTurnLimit(val, cbuf));
    
    else if (strcmp("TRACKER_MAXPAUSE", p) == 0)
       chprintf(_serial, "%s\r", PARSE_BYTE(TRACKER_MAXPAUSE, val, 0, 100, cbuf));
    
    else if (strcmp("TRACKER_MINPAUSE", p) == 0)
      chprintf(_serial, "%s\r", PARSE_BYTE(TRACKER_MINPAUSE, val, 0, 100, cbuf));
    
    else if (strcmp("TRACKER_MINDIST", p) == 0)
      chprintf(_serial, "%s\r", PARSE_BYTE(TRACKER_MINDIST, val, 0, 250, cbuf));
    
    else
       chprintf(_serial, "ERROR. Unknown setting\r");
}

 



static ap_config_t wifiap;


static void cmd_checkAp(char* ssid) {
   for (int i=0; i<N_WIFIAP; i++) {
     GET_PARAM_I(WIFIAP, i, &wifiap);
     if (strcmp(ssid, wifiap.ssid) == 0) {
        chprintf(_serial, "%d,%s\r", i, wifiap.passwd);
        return;
     }
   }
   /* Index 999 means that no config is found */
   chprintf(_serial, "999,_NO_\r");
   
}



/*************************************************************************** 
 * Get and execute a command from the WIFI module
 * 
 * A command is one character followed by arguments and ended with a newline
 * Get parameter: #R PARM
 * Set parameter: #W PARM VALUE 
 ***************************************************************************/

static void wifi_command() {
   char *tokp; 
   
   MUTEX_LOCK;
   readline(_serial, cbuf, 128);
   if (cbuf[0] == 'R') 
      /* Read parameter */
      cmd_getParm((char*) _strtok((char*) cbuf+1, " ", &tokp));
   else if (cbuf[0] == 'W')
      /* Write parameter */
      cmd_setParm((char*) _strtok((char*) cbuf+1, " ", &tokp), (char*) _strtok(NULL, "\0", &tokp));
   else if (cbuf[0] == 'A')
      /* Check access point */
      cmd_checkAp((char*) _strtok((char*) cbuf+1, " ", &tokp));
     
   MUTEX_UNLOCK;
}



/*****************************************************************************
 * Main thread to get characters from the WIFI module over 
 * the serial line. 
 *****************************************************************************/

static THD_FUNCTION(wifi_monitor, arg)
{
   (void) arg;
   chRegSetThreadName("WIFI module listener");
   while (true) {  
      char c;
      if (streamRead(_serial, (uint8_t *)&c, 1) != 0) {
         if (_shell != NULL)
            /* If shell is active, just pass character on to the shell */
            streamPut(_shell, c);
         else if (c == '$') {
            readline(_serial, cbuf, 8);
            if (strcmp(cbuf, "__BOOT__") == 0)
                wifi_start_server();
        }
         else if (c == '#') {
            /* If shell is not active and if the incoming character is a #, it is
             * initiating a command or a response. All other characters are ignored.  
             */
	    if (client_active) {
	       readline(_serial, *client_buf, 128);
	       SIGNAL_RESPONSE;
	    }
            else 
	       wifi_command();
	 }
      }
   }
}



void wifi_init(SerialDriver* sd)
{  
  _serial = (Stream*) sd;
  wifi_internal();
  clearPin(WIFI_ENABLE);
  sdStart(sd, &_serialConfig);  
  THREAD_START(wifi_monitor, NORMALPRIO+2, NULL);
  if (GET_BYTE_PARAM(WIFI_ON))
     wifi_enable();
}

