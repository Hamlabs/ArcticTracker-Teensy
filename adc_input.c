
/*
 * ADC sampling of receiver audio signal
 */


#include "hal.h"
#include "defines.h"
#include "chprintf.h"


#define RADIO_ADC_SAMPLE_FREQ    9600 
#define RADIO_ADC_NUM_CHANNELS   1
#define RADIO_ADC_CHANNELS       ADC_AD10
#define RADIO_ADC_BUFSIZE        1
#define RADIO_ADC_GPT            AFSK_RX_GPT
 
 
 /* Buffer */
 static adcsample_t samples[RADIO_ADC_NUM_CHANNELS * RADIO_ADC_BUFSIZE];
 
 static void adc_sample(void); 
 static void adc_sample_cb(void);
 
 
 
 /*************************************************************
  * ADC conversion group.
  * Mode: Linear buffer, 1 sample of 1 channel, SW triggered.
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
   
   /* CFG1 Register - ADCCLK = SYSCLK / 16, 16 bits per sample */
   ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
   ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
   ADCx_CFG1_MODE(ADCx_CFG1_MODE_16_BITS),
   
   /* SC3 Register - Average 4 readings per sample */
   ADCx_SC3_AVGE |
   ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_4_SAMPLES)
 };
 
 
/* ADC config */ 
 static const ADCConfig adc_cfg = {
   /* Perform initial calibration */
   true
 };
 
/* GPT timer config */ 
 static const GPTConfig radio_gpt_cfg = {
   RADIO_ADC_SAMPLE_FREQ,  
   adc_sample    /* Timer callback.*/
 };
 

/***************************************************
 * Start sampling. 
 * We use a GPT running in continuous mode to 
 * trigger sampling
 ***************************************************/

void adc_start_sampling() {
   adcStart(&ADCD1, &adc_cfg);
   gptStart(&RADIO_ADC_GPT, &radio_gpt_cfg);
   gptStartContinuous(&RADIO_ADC_GPT, 960);  
}


/***************************************************
 * Stop sampling
 ***************************************************/

void adc_stop_sampling() {
   gptStopContinuous(&RADIO_ADC_GPT);
}


/***************************************************
 * This is for testing. Just put samples into
 * a queue to be read by consumer thread
 ***************************************************/

static uint16_t _buf[4];
INPUTQUEUE_DECL(testq, _buf, 4, NULL, NULL); 
 

static void adc_sample() {
    adcStartConversionI(&ADCD1, &adc_grpcfg, samples, RADIO_ADC_BUFSIZE);    
}

static void adc_sample_cb() {
    if (!chIQIsFullI(&testq)) 
       chIQPutI(&testq, samples[0]);
}

uint8_t adc_getSample() {
    return chIQGet(&testq);
}

