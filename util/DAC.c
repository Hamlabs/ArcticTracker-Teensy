#include <inttypes.h> 
#include "util/DAC.h"
 
 /* "Borrowed" from Teensyduino kinetis.h */
 
#define SIM_SCGC2              (*(volatile uint32_t *)0x4004802C) // System Clock Gating Control Register 2
#define SIM_SCGC2_DAC0         ((uint32_t)0x00001000)             // DAC0 Clock Gate Control
#define DAC0_C0                (*(volatile uint8_t  *)0x400CC021) // DAC Control Register
#define DAC_C0_DACEN           0x80                               // DAC Enable
#define DAC_C0_DACRFS          0x40                               // DAC Reference Select
#define DAC0_DAT0L             (*(volatile uint8_t  *)0x400CC000) // DAC Data Low Register
#define DAC0_DAT0H             (*(volatile uint8_t  *)0x400CC001) // DAC Data High Register

 
 
static uint8_t analog_reference_internal = 0;
 
 
void dac_init()
{
  SIM_SCGC2 |= SIM_SCGC2_DAC0;
  if (analog_reference_internal) {
    DAC0_C0 = DAC_C0_DACEN;  // 1.2V ref is DACREF_1
  } else {
    DAC0_C0 = DAC_C0_DACEN | DAC_C0_DACRFS; // 3.3V VDDA is DACREF_2
  }
}

 
 /* "Borrowed" from Teensyduino analog.c */
 void analogWrite(int val)
 {
   if (val < 0) val = 0;
   else if (val > 4095) val = 4095;
   (DAC0_DAT0L) = (uint8_t) val;
   (DAC0_DAT0H) = (uint8_t) (val >> 8);
 }
 