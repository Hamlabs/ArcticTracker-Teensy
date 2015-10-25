#include "hal.h"
#include "defines.h"
#include "ui.h"


/*****************************************************************
 * Mix three parts of RGB LED by using virtual timer.
 *****************************************************************/
 
 static uint8_t _red, _green, _blue, _off; 
 static int8_t cstate = -1; 
 static virtual_timer_t vt; 
 
 static void _rgb_led_off(void);
 
 
 static void chandler()
 {  
     register bool loop;
     if (cstate == -1) 
        return;
     do {
       loop=false;
       if (cstate==1) {
          if (_red > 0) {
            rgb_led_on(true, false, false);
            chVTSetI(&vt, MS2ST(_red), chandler, NULL);
          } else cstate=2;
       }
       if (cstate==2) {
          if (_green > 0) {
            rgb_led_on(false, true, false);
            chVTSetI(&vt, MS2ST(_green), chandler, NULL);
          } else cstate=3;
       }   
       if (cstate==3) {
          if (_blue > 0) {
            rgb_led_on(false, false, true);
            chVTSetI(&vt, MS2ST(_blue), chandler, NULL);
          } else cstate=0;
       }
       if (cstate==0) {
         if (_off > 0) {
           _rgb_led_off();
           chVTSetI(&vt, MS2ST(_off), chandler, NULL);   
         } else {loop=true; cstate=1; }
       }
     }
     while (loop);
     cstate = (cstate+1) % 4;  
 }
 
 
 void rgb_led_mix(uint8_t red, uint8_t green, uint8_t blue, uint8_t off)
 {
    _red=red; _green=green; _blue=blue; _off=off;
   
     cstate = 1; 
     chVTSet( &vt, MS2ST(_off==0 ? 1 : _off), chandler, NULL);
 }
 
 
 /************************************************************************
  * Turn on specified RGB led(s)
  ************************************************************************/
 
 void rgb_led_on(bool red, bool green, bool blue)
 {
   _rgb_led_off();
   if (red)
     palSetPad(LED_R_PORT, LED_R_PIN);
   if (green)
     palSetPad(LED_G_PORT, LED_G_PIN);
   if (blue)
     palSetPad(LED_B_PORT, LED_B_PIN);
 }
 
 
 /************************************************************************
  * Turn off all RGB leds
  ************************************************************************/
 
 static void _rgb_led_off()
 {
   palClearPad(LED_R_PORT, LED_R_PIN);
   palClearPad(LED_G_PORT, LED_G_PIN);
   palClearPad(LED_B_PORT, LED_B_PIN);
 }
 
 void rgb_led_off() {
   cstate = -1;
   _rgb_led_off();
 }
 
 
 /*********************************************************************
  * DCD LED
  *********************************************************************/
 
 void dcd_led_on() {
    palSetPad(LED_DCD_PORT, LED_DCD_PIN);
 }
 
 void dcd_led_off() {
   palClearPad(LED_DCD_PORT, LED_DCD_PIN);
 }
 
 
 /*********************************************************************
  * Main UI thread. LED blinking to indicate that it is alive
  *********************************************************************/
 
 THREAD_STACK(ui_thread, 164);
 
 __attribute__((noreturn))
 static THD_FUNCTION(ui_thread, arg)
 {
   (void)arg;
   
   chRegSetThreadName("LED Blinker");
   
   /* Test RGB LED */
   rgb_led_on(true, false, false);
   sleep(300);
   rgb_led_on(false, true, false);
   sleep(300);
   rgb_led_on(false, false, true);
   sleep(300);
   rgb_led_off();
   sleep(300);
   dcd_led_on();
   sleep(1000);
   dcd_led_off();
   
   /* Blink LED every 2 second */
   while (TRUE) {
     palSetPad(IOPORT3, PORTC_TEENSY_PIN13);
     sleep(50);
     palClearPad(IOPORT3, PORTC_TEENSY_PIN13);
     sleep(1950);
   }
 }
 
 
 /*************************************************************
  * Pushbutton handler
  *************************************************************/
 
 static bool buttpushed; 
 void button_handler(EXTDriver *extp, expchannel_t channel) {
   (void)extp;
   (void)channel;
   buttpushed = (buttpushed ? false : true); 
   if (buttpushed)
      rgb_led_on(false, true, false);
   else
      rgb_led_off();
  
 }
 
 
 /*****************************************
  * UI init
  *****************************************/
 
 void ui_init()
 {   
   rgb_led_off();   
   THREAD_START(ui_thread, NORMALPRIO+1, NULL);
 }
 