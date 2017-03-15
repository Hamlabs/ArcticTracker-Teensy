 #if !defined __ADC_H__
 #define __ADC_H__
 
 void adc_init(void);
 uint8_t adc_dcoffset(void);
 int32_t adc_read_temp(void);
 int8_t adc_read_input(void);
 void adc_start_sampling(void);
 void adc_stop_sampling(void);
 
#endif
