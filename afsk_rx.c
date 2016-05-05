/*
 * AFSK Demodulation. 
 * 
 * Based on code from BertOS AFSK decoder. 
 * Originally by Develer S.r.l. (http://www.develer.com/), GPLv2 licensed.
 * 
 */

#include "hal.h"
#include "fifo.h"       
#include <string.h>
#include "defines.h"
#include "afsk.h"
#include "ui.h"
#include "fifo.h"
#include "adc_input.h"



#define SAMPLERATE 9600                             // The rate at which we are sampling 
#define BITRATE    1200                             // The actual bitrate at baseband. This is the baudrate.
#define SAMPLESPERBIT (SAMPLERATE / BITRATE)        // How many DAC/ADC samples constitute one bit (8).

/* Important: Sample rate must be divisible by bitrate */

/* Phase sync constants */
#define PHASE_BITS   8                              // How much to increment phase counter each sample
#define PHASE_INC    1                              // Nudge by an eigth of a sample each adjustment
#define PHASE_MAX    (SAMPLESPERBIT * PHASE_BITS)   // Resolution of our phase counter = 64
#define PHASE_THRESHOLD  (PHASE_MAX / 2)            // Target transition point of our phase window


/* Detect transition */
#define BITS_DIFFER(bits1, bits2) (((bits1)^(bits2)) & 0x01)
#define TRANSITION_FOUND(bits) BITS_DIFFER((bits), (bits) >> 1)


/* Qeue of decoded bits. To be used by HDLC packet decoder */
static uint8_t _buf[AFSK_RX_QUEUE_SIZE];
static input_queue_t iq;


/*********************************************************
 * This is our primary modem struct. It defines
 * the values we need to demodulate data.
 *********************************************************/

typedef struct AfskRx
{
   int16_t iirX[2];         // Filter X cells
   int16_t iirY[2];         // Filter Y cells
  
   uint8_t sampled_bits;    // Bits sampled by the demodulator (at ADC speed)
   int8_t  curr_phase;      // Current phase of the demodulator
   uint8_t found_bits;      // Actual found bits at correct bitrate
  
   bool    cd;              // Carrier detect 
   uint8_t cd_state;
  
} AfskRx;

static AfskRx afsk;


static void add_bit(bool bit);

int8_t delay_buf[100];
fifo_t fifo; 



/*******************************************
  Modem Initialization                             
 *******************************************/

input_queue_t* afsk_rx_init() {
  /* Allocate memory for struct */
  memset(&afsk, 0, sizeof(afsk));
  
  fifo_init(&fifo, delay_buf, sizeof(delay_buf));
  
  /* Fill sample FIFO with 0 */
  for (int i = 0; i < SAMPLESPERBIT / 2; i++)
    fifo_push(&fifo, 0);
  
  iqObjectInit(&iq, _buf,  AFSK_RX_QUEUE_SIZE, NULL, NULL);
  return &iq;
}


/*********************************************
 * Turn receiving on and off
 *********************************************/

void afsk_rx_enable() 
   { adc_start_sampling(); }
   
void afsk_rx_disable() 
   { adc_stop_sampling(); }


/*********************************************
 * Handler for squelch on/off signal
 *********************************************/

void trx_sq_handler(EXTDriver *extp, expchannel_t channel) {
   (void) extp;
   (void) channel;
}  
  
  
#define ABS(x) ((x) < 0 ? -(x) : (x))

/***************************************************************
  This routine should be called 9600
  times each second to analyze samples taken from
  the physical medium. 
****************************************************************/

void afsk_process_sample(int8_t curr_sample) 
{ 
#define DCD_LEVEL 5

    /* Butterworth filter */
    afsk.iirX[0] = afsk.iirX[1];
    afsk.iirX[1] = (fifo_pop(&fifo) * curr_sample) >> 2;
    afsk.iirY[0] = afsk.iirY[1];
    afsk.iirY[1] = afsk.iirX[0] + afsk.iirX[1] + (afsk.iirY[0] >> 1) + (afsk.iirY[0] >> 3) + (afsk.iirY[0] >> 5);
    
    /* Save this sampled bit in a delay line */
    afsk.sampled_bits <<= 1;
    afsk.sampled_bits |= (afsk.iirY[1] > 0) ? 1 : 0;
    /* Store current ADC sample in the af->delay_fifo */
    fifo_push(&fifo, curr_sample);
    
    
    /* 
     * If there is a transition, adjust the phase of our sampler
     * to stay in sync with the transmitter. 
     */ 
    if (TRANSITION_FOUND(afsk.sampled_bits)) {
        if (afsk.curr_phase < PHASE_THRESHOLD) {
            afsk.curr_phase += PHASE_INC;
        } else {
            afsk.curr_phase -= PHASE_INC;
        }
    }

    afsk.curr_phase += PHASE_BITS;

    /* Check if we have reached the end of
     * our sampling window.
     */ 
    if (afsk.curr_phase >= PHASE_MAX) 
    { 
        afsk.curr_phase %= PHASE_MAX;

        /* Shift left to make room for the next bit */
        afsk.found_bits <<= 1;

        /*
         * Determine bit value by reading the last 3 sampled bits.
         * If the number of ones is two or greater, the bit value is a 1,
         * otherwise is a 0.
         * This algorithm presumes that there are 8 samples per bit.
         */
        uint8_t bits = afsk.sampled_bits & 0x07;
        if ( bits == 0x07     // 111, 3 bits set to 
              || bits == 0x06 // 110, 2 bits
              || bits == 0x05 // 101, 2 bits
              || bits == 0x03 // 011, 2 bits
           )
           afsk.found_bits |= 1;

        /* 
         * Now we can pass the actual bit to the HDLC parser.
         * We are using NRZI coding, so if 2 consecutive bits
         * have the same value, we have a 1, otherwise a 0.
         * We use the TRANSITION_FOUND function to determine this.
         */
        add_bit( !TRANSITION_FOUND(afsk.found_bits) );
    }
}



/*********************************************************
 * Send a single bit to the HDLC decoder
 *********************************************************/

static uint8_t bit_count = 0;

static void add_bit(bool bit)
{ 
  static uint8_t octet;
  octet = (octet >> 1) | (bit ? 0x80 : 0x00);
  bit_count++;
  
  if (bit_count == 8) 
  {        
    if  (!iqIsFullI(&iq)) 
       iqPutI(&iq, octet);
 
    bit_count = 0;
  }
}

