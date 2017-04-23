
/*
 * ADC sampling of receiver audio signal
 */


#include "hal.h"
#include "defines.h"
#include "chprintf.h"
#include "adc_input.h"


#define RADIO_ADC_SAMPLE_FREQ    9600 
#define RADIO_ADC_NUM_CHANNELS   1
#define RADIO_ADC_CHANNELS       ADC_TEENSY_PIN11
#define RADIO_ADC_BUFSIZE        1
#define RADIO_ADC_GPT            AFSK_RX_GPT
 
#define BATT_CHANNELS            ADC_TEENSY_PIN10 
#define BATT_NUM_CHANNELS        2
#define BATT_NUM_BUFSIZE         1


static uint8_t dcoffset = 0; 

 /* Buffer */
 static adcsample_t samples[RADIO_ADC_NUM_CHANNELS * RADIO_ADC_BUFSIZE];
 static adcsample_t samples2[BATT_NUM_CHANNELS * BATT_NUM_BUFSIZE]; 
 
 extern void afsk_process_sample(int8_t curr_sample);
 static void adc_sample(GPTDriver *gptp);
 static void adc_sample_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n);
 
 
 /* ADC config */ 
 static const ADCConfig adc_cfg = {
   /* Perform initial calibration */
   true
 };
 
 
 
 
 
 /*************************************************************
  * Config of ADC for sampling temp sensor, etc.. 
  *************************************************************/
 
 static const ADCConversionGroup adc_grpcfg2 = {
   false,
   BATT_NUM_CHANNELS,
   NULL, NULL,
   BATT_CHANNELS | ADC_BANDGAP,
   /* CFG1 Regiser - ADCCLK = SYSCLK / 16, 16 bits per sample */
   ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
   ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
   ADCx_CFG1_MODE(ADCx_CFG1_MODE_16_BITS),
   /* SC3 Register - Average 32 readings per sample */
   ADCx_SC3_AVGE |
   ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES)
 };
 
 
 
 /*************************************************************
  * Config of ADC for sampling radio signal
  * Set up a ADC group with 8 bit sampling of 1 channel. 
  * 
  * Also set up a HW timer to do periodic sampling with a 
  * specific frequency. 
  *************************************************************/
 
 static const ADCConversionGroup adc_grpcfg = {
  /* Enable circular buffer */  
   false, 
  
   /* Number of channels */
   RADIO_ADC_NUM_CHANNELS,        
   
   /* Callback, error-callback */
   adc_sample_cb, NULL,
   
   /* Bitmask of channels */
   RADIO_ADC_CHANNELS,
   
   /* CFG1 Register - ADCCLK = SYSCLK */
   ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
   ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
   ADCx_CFG1_MODE( ADCx_CFG1_MODE_8_OR_9_BITS ),
   
   /* SC3 Register - Average 4 readings per sample */
   ADCx_SC3_AVGE |
    ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_4_SAMPLES)
 };
 
 
/* GPT timer config */ 
 static const GPTConfig radio_gpt_cfg = {
   RADIO_ADC_SAMPLE_FREQ,  
   adc_sample    /* Timer callback.*/
 };
 

 
 
/***************************************************
 * ADC init
 ***************************************************/ 
 
void adc_init()
{
   adcStart(&ADCD1, &adc_cfg);
   gptStart(&RADIO_ADC_GPT, &radio_gpt_cfg);
   adc_calibrate();
}

void adc_calibrate() {
   if (adcConvert(&ADCD1, &adc_grpcfg, samples, 1) == MSG_OK)
      dcoffset = (uint8_t) samples[0];
}

uint8_t adc_dcoffset() {
    return dcoffset;
}




/***************************************************
 * Read temperature
 * Taken from ChibiOS testhal example
 ***************************************************/

uint16_t adc_read_batt()
{
  if (adcConvert(&ADCD1, &adc_grpcfg2, samples2, 1) != MSG_OK)
    return 0; 
 
  uint16_t batt = (uint16_t) samples2[0];
  uint16_t bandgap = (uint16_t) samples2[1];
  uint32_t  vbatt = ((uint32_t) batt * 985) / (uint32_t) bandgap;
    
  return (uint16_t) vbatt * 3;
}


int8_t adc_read_input()
{
  if (adcConvert(&ADCD1, &adc_grpcfg, samples, 1) != MSG_OK)
    return 0;
  return (int8_t) (samples[0] - dcoffset);
}


/***************************************************
 * Start sampling. 
 * We use a GPT running in continuous mode to 
 * trigger sampling
 ***************************************************/

void adc_start_sampling() {
   gptStartContinuous(&RADIO_ADC_GPT, 1);  
}


/***************************************************
 * Stop sampling
 ***************************************************/

void adc_stop_sampling() {
   gptStopTimer(&RADIO_ADC_GPT);
}


/***************************************************
 * Callbacks
 ***************************************************/

static void adc_sample(GPTDriver *gptp) {
    (void)gptp;
    adcStartConversionI(&ADCD1, &adc_grpcfg, samples, RADIO_ADC_BUFSIZE);    
}

static void adc_sample_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
    (void)adcp;
    (void)n;
    
    afsk_process_sample((int8_t) (buffer[0] - dcoffset));
}

