
/*
 * ADC sampling of receiver audio signal
 */


#include "hal.h"
#include "defines.h"
#include "chprintf.h"
#include "adc_input.h"


#define RADIO_ADC_SAMPLE_FREQ    9600 
#define RADIO_ADC_NUM_CHANNELS   1
#define RADIO_ADC_CHANNELS       ADC_TEENSY_PIN10
#define RADIO_ADC_BUFSIZE        1
#define RADIO_ADC_GPT            AFSK_RX_GPT
 
#define TEMP_NUM_CHANNELS        2
#define TEMP_NUM_BUFSIZE         1
#define ADC_TEMP_ERROR           300000

static uint8_t dcoffset = 0; 

 /* Buffer */
 static adcsample_t samples[RADIO_ADC_NUM_CHANNELS * RADIO_ADC_BUFSIZE];
 static adcsample_t samples2[TEMP_NUM_CHANNELS * TEMP_NUM_BUFSIZE]; 
 
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
   TEMP_NUM_CHANNELS,
   NULL, NULL,
   ADC_TEMP_SENSOR | ADC_BANDGAP,
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
   0 //ADCx_SC3_AVGE |
   // ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_4_SAMPLES)
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
   if (adcConvert(&ADCD1, &adc_grpcfg, samples, 1) == MSG_OK)
     dcoffset = (uint8_t) samples[0];
}



/***************************************************
 * Read temperature
 * Taken from ChibiOS testhal example
 ***************************************************/

int32_t adc_read_temp()
{
  if (adcConvert(&ADCD1, &adc_grpcfg2, samples2, 2) != MSG_OK)
    return ADC_TEMP_ERROR; 
 
  /*
   * The bandgap value represents the ADC reading for 1.0V
   */
  uint16_t sensor = samples2[0];
  uint16_t bandgap = samples2[1];
  
  /*
   * The v25 value is the voltage reading at 25C, it comes from the ADC
   * electricals table in the processor manual. V25 is in millivolts.
   */
  int32_t v25 = 719;
  
  /*
   * The m value is slope of the temperature sensor values, again from
   * the ADC electricals table in the processor manual.
   * M in microvolts per degree.
   */
  int32_t m = 1715;
  
  /*
   * Divide the temperature sensor reading by the bandgap to get
   * the voltage for the ambient temperature in millivolts.
   */
  int32_t vamb = ((int32_t) sensor * 1000) / (int32_t) bandgap;
  
  /*
   * This formula comes from the reference manual.
   * Temperature is in millidegrees C.
   */
  int32_t delta = (((vamb - v25) * 1000000) / m);
  int32_t temp = 25000 - delta;
  return temp;
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

