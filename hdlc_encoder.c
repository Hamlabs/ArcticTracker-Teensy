/*
 * Adapted from Polaric Tracker code. 
 * By LA7ECA, ohanssen@acm.org and LA3T
 */

#include "ch.h"
#include "config.h"
#include "util/crc16.h"
#include "defines.h"
#include "chprintf.h"
#include <stdlib.h>
#include "hdlc.h"
#include "hal.h"

output_queue_t *outqueue; 
FBQ encoder_queue; 
FBQ *mqueue = NULL;
static FBUF buffer; 

#define BUFFER_EMPTY (fbuf_eof(&buffer))            


/* FIXME: Those should be parameters stored in EEPROM ! */
#define PERSISTENCE 80 /* p = x/255 */
#define SLOTTIME    10 /* Milliseconds/10 */

THREAD_STACK(hdlc_txencoder, STACK_HDLCENCODER);




static bool hdlc_idle = true;

// static msg_t hdlc_txencoder(void*);
static void hdlc_encode_frames(void);
static void hdlc_encode_byte(uint8_t txbyte, bool flag);
static void wait_channel_ready(void);
// static msg_t hdlc_testsignal(void *);


extern Stream* shell; 

void hdlc_monitor_tx(FBQ* m)
   { mqueue = m; }

fbq_t* hdlc_get_encoder_queue()
   { return &encoder_queue; }



bool hdlc_enc_packets_waiting()
   { return !fbq_eof(&encoder_queue) || !BUFFER_EMPTY; }



/*******************************************************
 * Pseudo random function.
 *******************************************************/

static uint64_t seed = 123456789;
uint8_t rand_u8()
{
   seed = (1103515245 * seed + 12345) % 2147483647;
   return seed % 256;
}


/*******************************************************
 * Code for generating a test signal
 *******************************************************/

static bool test_active;
static uint8_t testbyte;

static THD_FUNCTION(hdlc_testsignal, arg)
{  
   (void)arg;
   chRegSetThreadName("HDLC TX Test Signal"); 
   hdlc_idle = false;
  
   while(test_active) 
     oqPut(outqueue, testbyte);
   
   hdlc_idle = true; 
   return;
}

static thread_t* testt=NULL; 
void hdlc_test_on(uint8_t b)
{ 
   testbyte = b;
   test_active = true;
   testt = THREAD_DSTART(hdlc_testsignal, "hdlc testsignal", 256, NORMALPRIO, NULL);
}

void hdlc_test_off()
  { test_active=false; 
    if (testt!=NULL) chThdWait(testt);
    testt=NULL;
  }



/*******************************************************************************
 * TX encoder thread
 *
 * This function gets a frame from buffer-queue, and starts the transmitter
 * as soon as the channel is free.   
 *******************************************************************************/
extern SerialUSBDriver SDU1;
__attribute__((noreturn))
static THD_FUNCTION(hdlc_txencoder, arg)
{ 
  (void)arg;
  
  chRegSetThreadName("HDLC TX Encoder");
  while (true)  
  {
     /* Get frame from buffer-queue when available. 
      * This is a blocking call.
      */  
     buffer = fbq_get(&encoder_queue); 

     /* Wait until channel is free 
      * P-persistence algorithm 
      */
//   radio_wait_enabled();  
     hdlc_idle = false;
     for (;;) {
        wait_channel_ready(); 
        uint8_t r  = rand_u8();    
        if (r > PERSISTENCE)
           sleep(SLOTTIME*10); 
        else 
           break;
      } 
      hdlc_encode_frames();
      hdlc_idle = true; 
 //   notifyAll(&hdlc_idle_sig);  WHAT IS THIS??
      sleep(500);
  }
}



static void wait_channel_ready()
{
    /* Wait to squelch is closed */
    /* TODO: Implement this in sr_frs.c */
}


/*************************************************************
 * Initialize hdlc encoder
 *************************************************************/

FBQ* hdlc_init_encoder(output_queue_t *oq) 
{
  outqueue = oq;
  FBQ_INIT(encoder_queue, HDLC_ENCODER_QUEUE_SIZE);
  THREAD_START(hdlc_txencoder, NORMALPRIO, NULL);
  return &encoder_queue; 
}




/*******************************************************************************
 * HDLC encode and transmit one or more frames (one single transmission)
 * It is responsible for computing checksum, bit stuffing and for adding 
 * flags at start and end of frames.
 *******************************************************************************/

static void hdlc_encode_frames()
{
   uint16_t crc = 0xffff;
   uint8_t txbyte, i; 
   uint8_t txdelay = GET_BYTE_PARAM(TXDELAY);
   uint8_t txtail  = GET_BYTE_PARAM(TXTAIL);
   uint8_t maxfr   = GET_BYTE_PARAM(MAXFRAME);
  
   /* Preamble of TXDELAY flags */
   for (i=0; i<txdelay; i++)
      hdlc_encode_byte(HDLC_FLAG, true);

   for (i=0;i<maxfr;i++) 
   { 
      fbuf_reset(&buffer);
      crc = 0xffff;

      while(!BUFFER_EMPTY)
      {
         txbyte = fbuf_getChar(&buffer);
         crc = _crc_ccitt_update (crc, txbyte);
         hdlc_encode_byte(txbyte, false);
      }
      if (mqueue != NULL) {
         /* 
          * Put packet on monitor queue, if active
          */
          fbq_put(mqueue, buffer);
      }
      else 
          fbuf_release(&buffer);   
    
      hdlc_encode_byte(crc^0xFF, false);       // Send FCS, LSB first
      hdlc_encode_byte((crc>>8)^0xFF, false);  // MSB
    
      if (!fbq_eof(&encoder_queue) && i < maxfr) {
         hdlc_encode_byte(HDLC_FLAG, true);
         buffer = fbq_get(&encoder_queue); 
      }
      else
         break;
   }

   /* Postamble of TXTAIL flags */  
   for (i=0; i<txtail; i++)
      hdlc_encode_byte(HDLC_FLAG, true);
}




/*******************************************************************************
 * HDLC encode and transmit a single byte. Includes bit stuffing if not flag
 *******************************************************************************/
 
 static void hdlc_encode_byte(uint8_t txbyte, bool flag)
 {    
    static uint8_t outbits = 0;
    static uint8_t outbyte;
   
    for (uint8_t i=1; i<8+1; i++)
    { 
       if (!flag && (outbyte & 0x7c) == 0x7c) 
          i--;
     
       else {
          outbyte |= ((txbyte & 0x01) << 7);
          txbyte >>= 1;  
       }
     
       if (++outbits == 8) {
          oqPut(outqueue, outbyte);
          outbits = 0;
       }
       outbyte >>= 1;      
    }   
 }
 