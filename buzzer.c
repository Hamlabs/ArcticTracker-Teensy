/* 
 * Generate beeps, etc, using the buzzer
 */

#include "hal.h"
#include "defines.h"
#include "ui.h"


#define CLOCK_FREQ 200000
#define FREQ(f) (CLOCK_FREQ/(f*2))


static void buzzer_toggle(GPTDriver *gptp); 
static void buzzer_start(uint16_t freq);
static void buzzer_stop(void);


    
static const GPTConfig gpt_cfg = {
   CLOCK_FREQ,  
   buzzer_toggle    /* Timer callback.*/
};
    


static void buzzer_toggle(GPTDriver *gptp) {
    (void)gptp;
    togglePin(BUZZER);
}


void _beep(uint16_t freq, uint16_t time) {
   buzzer_start(freq);
   sleep(time);
   buzzer_stop();
}


/**************************************************
 *  short double beep using two frequencies
 *   up (sucess or on) or down (failure or off)
 **************************************************/

void blipUp() {
  lbeep(60);
  beep(60);
}

void blipDown() {
  beep(60);
  lbeep(60);
}


/**************************************************
 *  "Telephone ring"
 **************************************************/

void ring() {
  for (int i=0; i<8; i++) {
     beep(25);
     lbeep(25);
  }
}


/**************************************
 *  Morse code 
 **************************************/

void beeps(char* s)
{
  while (*s != 0) {
    if (*s == '.')
      beep(50);
    else if (*s == '-')
      beep(150);
    else
      sleep(100);
    sleep(50);  
    s++;
  }
}


   
/**************************************************************************
 * Start generating a tone using the DAC.
 **************************************************************************/

static void buzzer_start(uint16_t freq) {
   gptStart(&BUZZER_GPT, &gpt_cfg);
   gptStartContinuous(&BUZZER_GPT, FREQ(freq));
}


/**************************************************************************
 * Stop generating a tone. 
 **************************************************************************/

static void buzzer_stop() {
   gptStopTimer(&BUZZER_GPT);
   clearPin(BUZZER);
//   gptStop(&AFSK_TONEGEN_GPT);
}


