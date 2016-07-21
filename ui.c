#include "hal.h"
#include "defines.h"
#include "ui.h"

static void chandler(void *p);
static void _rgb_led_off(void);
static void bphandler(void* p);
static void onoffhandler(void* p);



/*****************************************************************
 * Mix three parts of RGB LED by using virtual timer.
 *****************************************************************/
 
 static uint8_t _red, _green, _blue, _off; 
 static int8_t cstate = -1; 
 static virtual_timer_t vt; 
 
 
 
 static void chandler(void *p)
 {  
     (void)p;
     
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
     setPin(LED_R);
   if (green)
     setPin(LED_G);
   if (blue)
     setPin(LED_B);
 }
 
 
 /************************************************************************
  * Turn off all RGB leds
  ************************************************************************/
 
 static void _rgb_led_off()
 {
   clearPin(LED_R);
   clearPin(LED_G);
   clearPin(LED_B);
 }
 
 void rgb_led_off() {
   cstate = -1;
   _rgb_led_off();
 }
 
 
 /*********************************************************************
  * DCD LED
  *********************************************************************/
 
 void dcd_led_on() {
    setPin(LED_DCD);
 }
 
 void dcd_led_off() {
    clearPin(LED_DCD);
 }
 
 
 /*********************************************************************
  * Main UI thread. LED blinking to indicate that it is alive
  *********************************************************************/
 uint16_t blink_length, blink_interval;
 
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
   sleep(300);
   dcd_led_off();
   blipUp();
   sleep(500);
   beeps("--.- .-. ...-");

   
   /* Blink LED */
   BLINK_NORMAL;
   while (TRUE) {
     setPin(TEENSY_PIN13);
     sleep(blink_length);
     clearPin(TEENSY_PIN13);
     sleep(blink_interval);
   }
 }
 
 
 /*************************************************************
  * Pushbutton handler
  *************************************************************/
 
 static bool buttdown = false;
 static bool blight; 
 static virtual_timer_t vtb, vtOnoff; 
 

 void button_handler(EXTDriver *extp, expchannel_t channel) {
   (void)extp;
   (void)channel;
   
   buttdown = !pinIsHigh(BUTTON); 
   chVTResetI(&vtb);
   chVTSetI(&vtb, MS2ST(10), bphandler, NULL);
 }

 
 static void bphandler(void* p)
 {
    (void) p;
    
    if (!pinIsHigh(BUTTON) && buttdown) {
       chVTResetI(&vtOnoff);
       chVTSetI(&vtb, MS2ST(1000), onoffhandler, NULL);
    }
 }
 
 
static void onoffhandler(void* p) {
   (void) p; 
   
   if (!pinIsHigh(BUTTON)) { 
     if (!blight) {
       blight = true; 
       rgb_led_on(false, true, false);
     }
     else {
       blight = false; 
       rgb_led_off();
     } 
   }
}
 
 
 /*****************************************
  * UI init
  *****************************************/
 
 void ui_init()
 {   
   rgb_led_off();   
   THREAD_START(ui_thread, NORMALPRIO+1, NULL);
 }
 