#include "ch.h"
#include "hal.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "config.h"
#include "gps.h"
#include "ui/ui.h"
#include "ui/gui.h"


#define NSCREENS 6
#define LINE1 15
#define LINE2 26
#define LINE3 37

MUTEX_DECL(gui_mutex);
#define GUI_MUTEX_LOCK chMtxLock(&gui_mutex)
#define GUI_MUTEX_UNLOCK chMtxUnlock(&gui_mutex)

static int current = 0;
static char buf[30];

static void status_heading(char* label);
static void status_screen1(void);
static void status_screen2(void);
static void status_screen3(void);
static void status_screen4(void);
static void status_screen5(void);


/****************************************************************
 * Show current status screen 
 ****************************************************************/

void status_show() {
    GUI_MUTEX_LOCK;
    switch (current) {
        case 0:  gui_welcome();
                 break;
        case 1:  status_screen1(); 
                 break;
        case 2:  status_screen2(); 
                 break;       
        case 3:  status_screen3(); 
                 break;        
	    case 4:  status_screen4(); 
                 break;    	
        case 5:  status_screen5(); 
                 break;           
    }
    GUI_MUTEX_UNLOCK;
}


/****************************************************************
 * Cycle to next status screen 
 ****************************************************************/

void status_next() { 
    current = (current + 1) % NSCREENS; 
    status_show();
}



/****************************************************************
 * Display heading. 
 ****************************************************************/

static void status_heading(char* label) {
    gui_label(0,0, label);
    gui_flag(32,0, "i", GET_BYTE_PARAM(WIFI_ON));
    gui_flag(41,0, "g", GET_BYTE_PARAM(IGATE_ON)); 
    gui_flag(50,0, "d", GET_BYTE_PARAM(DIGIPEATER_ON));
//    Next position is 59,0 
    
    uint16_t batt = adc_read_batt(); 
    uint8_t bi; 
    if (batt > 8000)      bi = 4;
    else if (batt > 7800) bi = 3; 
    else if (batt > 7400) bi = 2;  
    else if (batt > 7200) bi = 1;  // Low
    else bi = 0;
             
    gui_battery(70,3,bi);
    gui_hLine(0,10,66);
}




/****************************************************************
 * 1. APRS status
 ****************************************************************/
  
static void status_screen1() {
    Dword f; 
    
    gui_clear();
    status_heading("APRS");

    GET_PARAM(MYCALL, buf);
    GET_PARAM(TRX_TX_FREQ, &f);
    gui_writeText(0, LINE1, buf);
    
    sprintf(buf, "%03lu.%03lu MHz%c", f/10000, (f/10)%1000, '\0');
    gui_writeText(0, LINE2, buf);
    gui_writeText(0, LINE3, "W1,W2-1,SAR");  
    gui_flush();
}


/****************************************************************
 * 2. GPS position status
 ****************************************************************/

static void status_screen2() {
    gui_clear();
    status_heading("GPS");
    if (gps_is_fixed()) {
       gui_writeText(0, LINE1, pos2str_lat(buf, gps_get_pos()));
       gui_writeText(0, LINE2, pos2str_long(buf, gps_get_pos()));
       gui_writeText(0, LINE3, datetime2str(buf, gps_get_date(), gps_get_time()));
    }		     
    else
       gui_writeText(0, LINE1, "Searching...");
    gui_flush();
}



/****************************************************************
 * 3. WIFI status
 ****************************************************************/

static void status_screen3() {
    gui_clear();
    status_heading("WIFI");
    
    gui_writeText(0, LINE1, wifi_status(buf));
        
    wifi_doCommand("CONF", buf);
    char *i = index(buf, '(');
    if (i != NULL)
      *i = '\0';

    gui_writeText(0, LINE2, buf);
    gui_writeText(0, LINE3, wifi_doCommand("IP", buf));    
    gui_flush();
}


/****************************************************************
 * 4. WIFI softap status
 ****************************************************************/

static void status_screen4() {
    gui_clear();
    status_heading("W-AP");
    gui_writeText(0, LINE1, (wifi_is_enabled() ? "Enabled" : "Disabled"));
    gui_writeText(0, LINE2, wifi_doCommand("AP.SSID", buf));
    gui_writeText(0, LINE3, wifi_doCommand("AP.IP", buf));  
    gui_flush();
}



/****************************************************************
 * 5. Battery status
 ****************************************************************/

static void status_screen5() {
    gui_clear();
    status_heading("BATT");
    uint16_t batt = adc_read_batt();
    sprintf(buf, "%1.02f V%c", ((float)batt)/1000, '\0');
    gui_writeText(0, LINE1, buf);
    if (batt > 8500) { 
        gui_writeText(0, LINE2, "Ext power");
        gui_writeText(0, LINE3, "Not charging.");
    }
    else if (batt > 8350)
        gui_writeText(0, LINE2, "Max/Charging..");
    else if (batt > 7800) 
        gui_writeText(0, LINE2, "Full.");
    else if (batt > 7400) 
        gui_writeText(0, LINE2, "Ok.");
    else if (batt > 7200) {
        gui_writeText(0, LINE2, "Low.");  
        gui_writeText(0, LINE3, "Need charging.");
    }
    else {
        gui_writeText(0, LINE2, "Empty.");
        gui_writeText(0, LINE3, "Charge ASAP!");
    }    
    gui_flush();
}
