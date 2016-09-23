
#include "defines.h"
#include "config.h"
#include "ax25.h"
#include "hdlc.h"
#include "chprintf.h"
   
static bool mon_on = false;
static bool mon_ax25 = true; 
static Stream *out;
FBQ mon;



void mon_init(Stream* outstr)
{
    out = outstr;
    FBQ_INIT(mon, HDLC_DECODER_QUEUE_SIZE);
}




/******************************************************************************
 *  Monitor thread 
 *   Just write out incoming frames. 
 ******************************************************************************/

static THD_FUNCTION(monitor, arg)
{
  (void) arg;
  chRegSetThreadName("Packet monitor");
  while (mon_on)
  {
    /* Wait for frame and then to AFSK decoder/encoder 
     * is not running. 
     */
    FBUF frame = fbq_get(&mon);
    if (!fbuf_empty(&frame)) {
      /* Display it */
       if (mon_ax25)
          ax25_display_frame(out, &frame);
       else 
          fbuf_print(out, &frame);
       chprintf(out, "\r\n");
    }
    
    /* And dispose the frame. Note that also an empty frame should be disposed! */
    fbuf_release(&frame);    
  }
  if (!mon_ax25)
    chprintf(out, "\r\n**** Connection closed ****\r\n");
}


static thread_t* mont=NULL;

void mon_activate(bool m)
{ 
   /* AX.25 or text mode */
   mon_ax25 = true; 
   
   /* Start if not on already */
   bool tstart = m && !mon_on;
   
   /* Stop if not stopped already */
   bool tstop = !m && mon_on;
   
   mon_on = m;
   
   if (tstart) {
      FBQ* mq = (mon_on? &mon : NULL);
      hdlc_subscribe_rx(mq, 0);
      if ( true || !mon_on || GET_BYTE_PARAM(TXMON_ON) )
         hdlc_monitor_tx(mq);
      mont = THREAD_DSTART(monitor, STACK_MONITOR, NORMALPRIO, NULL);  
   }
   if (tstop) {
      fbq_signal(&mon);
      if (mont!=NULL) 
           chThdWait(mont);
      mont=NULL;
      hdlc_monitor_tx(NULL);
      hdlc_subscribe_rx(NULL, 0);
   }
}


FBQ* mon_text_activate(bool m)
{ 
  /* AX.25 or text mode */
  mon_ax25 = false; 
  
  /* Start if not on already */
  bool tstart = m && !mon_on;
  
  /* Stop if not stopped already */
  bool tstop = !m && mon_on;
  
  mon_on = m;
  
  if (tstart) {
    FBQ* mq = (mon_on? &mon : NULL);
    mont = THREAD_DSTART(monitor, STACK_MONITOR, NORMALPRIO, NULL);  
    return mq;
  }
  if (tstop) {
    fbq_signal(&mon);
    if (mont!=NULL) 
      chThdWait(mont);
    mont=NULL;
  }
  return NULL;
}


