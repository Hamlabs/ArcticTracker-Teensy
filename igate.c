
/*
 * interface: 
 *  igate_init()
 *  igate_on()
 *  igate_off()
 *  igate_status()
 * 
 * Packet queue from receiver and from tracker 
 * 
 * Do we want RF transmission? 
 * If so, packet queue to transmitter
 *
 * Config: 
 *   IGATE_ON
 *   IGATE_HOST
 *   IGATE_PORT
 *   IGATE_PASSCODE
 *   IGATE_FILTER 
 *   IGATE_DIGIPATH
 * 
 * 
 * Add to config (later?):
 *   IGATE_RF_ON  
 *   IGATE_OBJ_RADIUS 
 */ 

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "config.h"
#include "fbuf.h"
#include "ui/ui.h"
#include "ui/wifi.h"
#include "hdlc.h"
#include "afsk.h"
#include "radio.h"
#include "heardlist.h"
#include "igate.h"



static void rf2inet(FBUF *);
static void inet2rf(FBUF *);

static bool _igate_on = false;
static bool _igate_run = false; 
static uint32_t _icount = 0;
static uint32_t _rcvd = 0;
static uint32_t _tracker_icount = 0;


static FBQ rxqueue;           /* Frames from radio or tracker */

extern fbq_t* outframes;      /* Frames to be transmitted on radio */
extern fbq_t* mon_q;          /* Do we need to monitor igate? */

static char buf[128];
static thread_t* igt=NULL;
static thread_t* igtm=NULL;



bool igate_is_on() 
  { return _igate_on; }


uint32_t igate_icount()
  { return _icount; }
  
uint32_t igate_rxcount()
  { return _rcvd; }
  
uint32_t igate_tr_count()
  { return _tracker_icount; }
  
  
/********************************************
 * Radio thread
 * Listen for incoming packets from radio 
 * (or outgoing packets from tracker)
 ********************************************/

static THD_FUNCTION(igate_radio, arg)
{
  (void) arg;
  chRegSetThreadName("Igate Radio");
  sleep(100); 
  while(_igate_run) {
    FBUF frame = fbq_get(&rxqueue);
    if (fbuf_length(&frame) > 2) {
      beeps("- ");
      _rcvd++;
      rf2inet(&frame);
    }   
    fbuf_release(&frame);
    sleep(100);
  }
}


/*******************************************
 * Igate main thread. 
 *  connect to aprs-is server
 *  listen for incoming data from server.
 *******************************************/

static THD_FUNCTION(igate_main, arg)
{
  (void) arg;
  chRegSetThreadName("Igate Main");
  sleep(1000);
  
  while (_igate_on) {
     /* connect-to-aprs-is */
     char host[INET_NAME_LENGTH]; 
     uint16_t port;
     GET_PARAM(IGATE_HOST, host);
     GET_PARAM(IGATE_PORT, &port);
     int res = -1, tries=0;
    
     while (!wifi_is_connected() 
            || (res=inet_open(host, port) != 0 && tries++ < 3))
        sleep(10000);
  
     if (_igate_on && res == 0) {
       /* Connected ok. Await welcome text */
       inet_ignoreInput();
       beeps("--.  "); blipUp();
       _igate_run = true;
    
       // Login using username/passcode and (option) sende filter-string
       char uname[CRED_LENGTH];
       char filter[CRED_LENGTH];
       uint16_t pass;
       GET_PARAM(IGATE_USERNAME, uname); 
       GET_PARAM(IGATE_PASSCODE, &pass);
       GET_PARAM(IGATE_FILTER, filter);         
       igate_login(uname, pass, filter);
    
       /* Start child thread to listen for frames from radio or tracker */
       igt = THREAD_DSTART(igate_radio, STACK_IGATE_RADIO, NORMALPRIO, NULL);
       hdlc_subscribe_rx(&rxqueue, 2);
       
       /* Listen for data from APRS/IS server */
       while (inet_is_connected() && _igate_on) {
          rgb_led_mix(2, 0, 10, 10);
          FBUF frame = inet_readFB();
          if (!fbuf_empty(&frame) && fbuf_getChar(&frame) != '#')
             inet2rf(&frame);
          fbuf_release(&frame);
       }
       rgb_led_off();
    
       /* Unsubscribe and terminate child thread */
       _igate_run = false; 
       fbq_signal(&rxqueue);
       sleep(10);
       hdlc_subscribe_rx(NULL, 2);
       
       if (igt!=NULL) 
          chThdWait(igt);
       igt=NULL;
       
       /* Connection failure. Wait for 2 minutes */
       if (_igate_on) {
          beeps(" --. ..-.");
          for (int i=0; i<60 && _igate_on; i++)
            sleep(2000);
       }  
     }
  }
  beeps("--.  "); blipDown();
}


/**********************
 *  igate init
 **********************/

void igate_init() {
  FBQ_INIT(rxqueue, HDLC_DECODER_QUEUE_SIZE);
  if (GET_BYTE_PARAM(IGATE_ON))
    igate_activate(true);
}



/***************************************************************
 * Turn igate on if argument is true, turn it off
 * if false. 
 ***************************************************************/

void igate_on(bool m) {
   SET_BYTE_PARAM(IGATE_ON, (m? 1:0) );
   igate_activate(m);
}



/***************************************************************
 * Activate the igate if argument is true
 * Deactivate if false
 ***************************************************************/

void igate_activate(bool m) 
{
   bool tstart = m && !_igate_on;
   bool tstop = !m && _igate_on;
  
   _igate_on = m;
   FBQ* mq = (_igate_on? &rxqueue : NULL);
   
   if (tstart) {
      /* Subscribe to RX (and tracker) packets and start treads */
      hdlc_subscribe_rx(mq, 2);
      tracker_setGate(mq);
      igtm = THREAD_DSTART(igate_main, STACK_IGATE, NORMALPRIO, NULL);  
      hlist_start();
    
      /* Turn on radio and decoder */
      /* FIXME: Need to turn on internet as well */
      radio_require();
   } 
   if (tstop) {
      /* Turn off radio and decoder */
      radio_release();
    
      /* Close internet connection */
      inet_close(); 
      sleep(100);
      
      inet_signalReader();
      if (igtm!=NULL)
        chThdWait(igtm);
      igtm=NULL;
      hdlc_subscribe_rx(NULL, 2);
      tracker_setGate(NULL);
      _icount = _rcvd = _tracker_icount = 0;
   }
}




/***************************************************************************
 * Gate frame to internet
 ***************************************************************************/

static void rf2inet(FBUF *frame) 
{
  FBUF newHdr;
  addr_t from, to, mycall; 
  addr_t digis[7];
  uint8_t ctrl, pid;
  GET_PARAM(MYCALL, &mycall);
  fbuf_reset(frame);
  uint8_t ndigis =  ax25_decode_header(frame, &from, &to, digis, &ctrl, &pid);
  char type = fbuf_getChar(frame);
  bool own = addrCmp(&mycall, &from); 
  
  if (hlist_duplicate(&from, &to, frame, ndigis))
    return;
  
  static const char* nogate[8] = {"TCP", "NOGATE", "RFONLY", NULL};
  if ( type == '?' /* QUERY */ ||
     ( !own && ax25_search_digis( digis, ndigis, (char**) nogate)))
    return;

  /* Write header in plain text -> newHdr */
  fbuf_new(&newHdr);
  fbuf_putstr(&newHdr, addr2str(buf,&from)); 
  fbuf_putstr(&newHdr, ">");
  fbuf_putstr(&newHdr, addr2str(buf,&to));
  fbuf_putstr(&newHdr, ",");
  fbuf_putstr(&newHdr, digis2str(buf, ndigis, digis));
  if (own && strncmp(buf, "TCPIP", 5) == 0)
     fbuf_putstr(&newHdr, "*");
  else {
     fbuf_putstr(&newHdr, ",qAR,");
     fbuf_putstr(&newHdr, addr2str(buf, &mycall));  
  }
  fbuf_putstr(&newHdr, ":");
  
  /* Replace header in original packet with new header. 
   * Do this non-destructively: Just add rest of existing packet to new header 
   */
  fbuf_connect(&newHdr, frame, AX25_HDR_LEN(ndigis) );
  
  /* Send to internet server */
  inet_writeFB(&newHdr);
  fbuf_release(&newHdr);
  if (own) _tracker_icount++; else _icount++;
}




/***************************************************************************
 * Gate frame to radio
 ***************************************************************************/

static void inet2rf(FBUF *frame) {
  (void) frame;
  /* TBD */
}



/***********************************************
 * Log in to APRS/IS server. 
 * Assume that connection is established. 
 ***********************************************/

void igate_login(char* user, uint16_t pass, char* filter) 
{
   sprintf(buf, "user %s pass %d vers Arctic-Tracker 0.1\r\n", user, pass);
   inet_write(buf);
   inet_ignoreInput();
  
   if (strlen(filter) > 1) {
      sprintf(buf, "filter %s\r\n", filter);
      inet_write(buf);
   }
}

