/* 
 * Generate a tone (sine wave) using the DAC.
 */

#include "hal.h"
#include "util/DAC.h"
#include "afsk.h"
#include "defines.h"


#define STEPS 16

/* 
 * Use the least common multiplier (LCM) of the sampling frequencies 
 * (16 x 1200 and 16 x 2200) as the clock frequency of the timer 
 */
#define CLOCK_FREQ 211200
#define FREQ(f) (CLOCK_FREQ/(f*STEPS))

/* The sine wave is generated in 16 steps */
#define STEPS 16

static bool _toneHigh = false;
static bool _on = false; 
static uint8_t i = 0;
static const uint16_t sine[STEPS] = 
  { 2059, 2815, 3464, 3898, 4050, 3898, 3464, 2815, 2050, 
    1285, 636, 202, 50, 202, 636, 1285 };

static void sinewave(GPTDriver *gptp); 
    
    
    
static const GPTConfig gpt_cfg = {
   CLOCK_FREQ,  
   sinewave    /* Timer callback.*/
};
    
    
/*****************************************************************
 * Call periodically from timer ISR to generate a sine wave.
 * This is to be done 16 X the frequency of the generated tone. 
 *****************************************************************/

static void sinewave(GPTDriver *gptp) {
  (void)gptp;
  analogWrite(sine[i++]);
  if (i >= STEPS) 
     i=0;
}


/**************************************************************************
 * Set the frequency of the tone to mark or space. 
 * argument to true to set it to MARK. Otherwise, SPACE. 
 **************************************************************************/

void tone_setHigh(bool hi) { 
   _toneHigh = hi; 
   if (_on) 
      gptChangeInterval(&AFSK_TONEGEN_GPT, 
         _toneHigh ? FREQ(AFSK_SPACE) : FREQ(AFSK_MARK) ); 
  } 

  
/**************************************************************************
 * Toggle between the two tone frequencies (mark or space).
 **************************************************************************/

void tone_toggle()
   { tone_setHigh(!_toneHigh); }
   
   
/**************************************************************************
 * Start generating a tone using the DAC.
 **************************************************************************/

void tone_start() {
   _on = true;
   dac_init();
   gptStart(&AFSK_TONEGEN_GPT, &gpt_cfg);
   gptStartContinuous(&AFSK_TONEGEN_GPT, 
      _toneHigh ? FREQ(AFSK_SPACE) : FREQ(AFSK_MARK));  
}


/**************************************************************************
 * Stop generating a tone. 
 **************************************************************************/

void tone_stop() {
   _on = false; 
   gptStopTimer(&AFSK_TONEGEN_GPT);
//   gptStop(&AFSK_TONEGEN_GPT);
}


