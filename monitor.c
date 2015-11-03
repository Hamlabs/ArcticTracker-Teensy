
#include "defines.h"
#include "config.h"
#include "ax25.h"
#include "hdlc.h"
#include "chprintf.h"
   
static bool mon_on = false;
static Stream *out;
FBQ mon;

THREAD_STACK(monitor, 500);



void mon_init(Stream* outstr)
{
    out = outstr;
    FBQ_INIT(mon, HDLC_DECODER_QUEUE_SIZE);
}





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
       ax25_display_frame(out, &frame);
       chprintf(out, "\r\n");
    }
    
    /* And dispose the frame. Note that also an empty frame should be disposed! */
    fbuf_release(&frame);    
  }
}




void mon_activate(bool m)
{ 
   /* Start if not on already */
   bool tstart = m && !mon_on;
   
   /* Stop if not stopped already */
   bool tstop = !m && mon_on;
   
   mon_on = m;
   FBQ* mq = (mon_on? &mon : NULL);
   hdlc_subscribe_rx(mq, 0);
   if ( true || !mon_on /* || GET_BYTE_PARAM(TXMON_ON) */ )
      hdlc_monitor_tx(mq);
   
   if (tstart) 
      THREAD_START(monitor, NORMALPRIO, NULL);  
   if (tstop) {
      hdlc_monitor_tx(NULL);
      hdlc_subscribe_rx(NULL, 0);
      fbq_signal(&mon);
   }
}



