/*
    ChibiOS/RT - Copyright (C) 2006-2014 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#define K20x_MCUCONF

/*
 * HAL driver system settings.
 */


/*
 * SERIAL driver system settings.
 */
#define KINETIS_SERIAL_USE_UART0              TRUE
#define KINETIS_SERIAL_USE_UART1              TRUE
#define KINETIS_SERIAL_USE_UART2              TRUE

/*
 * USB driver settings
 */
#define KINETIS_USB_USE_USB0                  TRUE

/*
 * ADC
 */
#define KINETIS_ADC_USE_ADC1                  TRUE
#define KINETIS_ADC_USE_ADC0                  TRUE



// Must define
#define KINETIS_HAS_SERIAL1                   TRUE
//#define KINETIS_SERIAL1_IRQ_VECTOR            VectorFC
#define KINETIS_HAS_SERIAL2                   TRUE
//#define KINETIS_SERIAL2_IRQ_VECTOR            Vector104

#define KINETIS_GPT_USE_PIT0 TRUE 
#define KINETIS_GPT_USE_PIT1 TRUE 
#define KINETIS_GPT_USE_PIT2 TRUE 
#define KINETIS_GPT_USE_PIT3 TRUE 
#define KINETIS_GPT_PIT1_IRQ_PRIORITY 15
#define KINETIS_GPT_PIT2_IRQ_PRIORITY 15
#define KINETIS_GPT_PIT3_IRQ_PRIORITY 15




/*
 * EXT driver system settings.
 */
#define KINETIS_EXTI_NUM_CHANNELS         2
#define KINETIS_EXT_PORTA_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTB_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTC_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTD_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTE_IRQ_PRIORITY          12

/* K20 64pin  */
#define KINETIS_EXT_PORTA_WIDTH                 20
#define KINETIS_EXT_PORTB_WIDTH                 20
#define KINETIS_EXT_PORTC_WIDTH                 12
#define KINETIS_EXT_PORTD_WIDTH                 8
#define KINETIS_EXT_PORTE_WIDTH                 2
