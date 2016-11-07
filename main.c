
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "util/shell.h"
#include <math.h>
#include <stdio.h>
#include "util/eeprom.h"
#include "radio.h"
#include "gps.h"
#include "fbuf.h"
#include "hdlc.h"
#include "afsk.h"
#include "defines.h"
#include "ui/ui.h"
#include "ui/wifi.h"
#include "ui/commands.h"
#include "adc_input.h"
#include "tracker.h"
#include "digipeater.h"


static void ext_init(void);
extern void usb_initialize(void);
extern bool usb_active(void);
extern void mon_init(Stream*);

extern SerialUSBDriver SDU1;

fbq_t *outframes, *inframes;  



/********************************
 * Set up SPI 
 ********************************/

static const SPIConfig spicfg = {
    NULL,                        /* Callback */
    SPI_CS_PORT,                 /* Chip select line port */
    SPI_CS_PIN,                  /* Chip select line pad number */
    KINETIS_SPI_TAR_8BIT_SLOW    /* SPI initialization data. */
};

extern SPIDriver SPID1;


static void spi_init() {
  palSetPadMode(SPI_SCK_PORT, SPI_SCK_PIN, PAL_MODE_ALTERNATIVE_2);      /* SCK  */
  palSetPadMode(SPI_MOSI_PORT, SPI_MOSI_PIN, PAL_MODE_ALTERNATIVE_2);    /* MOSI */
  palSetPadMode(SPI_MISO_PORT, SPI_MISO_PIN, PAL_MODE_ALTERNATIVE_2);    /* MISO */
  palSetPadMode(SPI_CS_PORT, SPI_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL);      /* SS   */  
  spiStart(&SPID1, &spicfg);
}



/*************************************************************
 * Set up interrupt driven GPIO
 *************************************************************/

static const EXTConfig extcfg = {
  {
    BUTTON_EXTCFG, TRX_SQ_EXTCFG
  }
};

static void ext_init() {
   extStart(&EXTD1, &extcfg);
   setPinMode(BUTTON, BUTTON_MODE);
   setPinMode(TRX_SQ, TRX_SQ_MODE);
   extChannelEnable(&EXTD1, 0);
   extChannelEnable(&EXTD1, 1);
}



/******************************************************
 * Application entry point.
 ******************************************************/

int main(void) 
{     
   thread_t *shelltp = NULL;
   halInit();
   chSysInit();
   ext_init();
   spi_init();
   usb_initialize();
   radio_init(&TRX_SERIAL);
   ui_init();
   eeprom_initialize(); 
   adc_init();
   hdlc_init_decoder(afsk_rx_init());
   outframes = hdlc_init_encoder(afsk_tx_init());
   afsk_tx_start(); // Call this only when needed and stop it when not needed??
   // FIXME: Rename to afsk_tx_enable
   gps_init(&GPS_SERIAL, (Stream*) &SHELL_SERIAL);
   tracker_init(); 
   sleep(100);
   digipeater_init();
   mon_init((Stream*) &SHELL_SERIAL);
   wifi_init(&WIFI_SERIAL);
   igate_init();
   
   shellInit();

   while (!chThdShouldTerminateX()) {
     if (!shelltp && usb_active()) {
        sleep(100);
        shelltp = myshell_start();
     }
     else if (chThdTerminatedX(shelltp)) {
        chThdRelease(shelltp);    
        shelltp = NULL;   
     }       
     chThdSleepMilliseconds(1000);
   }

   
   return 0;
}
