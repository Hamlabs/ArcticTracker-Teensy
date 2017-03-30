
#include <stdio.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "defines.h"
#include "util/eeprom.h"
#include "config.h"
#include "ui/commands.h"
#include "ui/ui.h"
#include "radio.h"
#include "hdlc.h"
#include "afsk.h"

#define FLAG_BUSY_LOCK  0x01
#define FLAG_COMP_EXP   0x02
#define FLAG_LO_POWER   0x04


static bool     _on = false;
static bool     _sq_on = false; 
static uint8_t  _widebw; 
static uint8_t  _flags;        
static uint32_t _txfreq;       // TX frequency in 100 Hz units
static uint32_t _rxfreq;       // RX frequency in 100 Hz units
static uint8_t  _squelch;      // Squelch level (0-8 where 0 is open)
static Stream*  _serial;

static int count = 0; 
  
static bool _handshake(void);
static bool _setGroupParm(void);
static void _initialize(void);

/*
extern SerialUSBDriver SDU1;
static Stream* out = (Stream*) &SDU1; 
*/
static const SerialConfig _serialConfig = {
   9600
};

MUTEX_DECL(radio_mutex);
#define MUTEX_LOCK chMtxLock(&radio_mutex)
#define MUTEX_UNLOCK chMtxUnlock(&radio_mutex)

MUTEX_DECL(radio2_mutex);
#define MUTEX2_LOCK chMtxLock(&radio2_mutex)
#define MUTEX2_UNLOCK chMtxUnlock(&radio2_mutex)

BSEMAPHORE_DECL(tx_off, true);
#define WAIT_TX_OFF chBSemWait(&tx_off)
#define SIGNAL_TX_OFF chBSemSignal(&tx_off)

CONDVAR_DECL(_radio_rdy);
#define WAIT_RADIO_READY chCondWait(&_radio_rdy)
#define SIGNAL_RADIO_READY chCondBroadcast(&_radio_rdy)
bool radio_rdy = false;

CONDVAR_DECL(_channel_rdy);
#define WAIT_CHANNEL_READY chCondWait(&_channel_rdy)
bool channel_rdy = true;



/******************************************************
 * Need radio - turn it on if not already on
 ******************************************************/
 
void radio_require(void)
{   
   MUTEX_LOCK;
   if (++count == 1) 
     radio_on(true);
   MUTEX_UNLOCK;
}



/*******************************************************
 * Radio not needed any more - turn it off if no others
 * need it
 *******************************************************/
 
void radio_release(void)
{
    MUTEX_LOCK;
    if (--count == 0) {
       /* 
        * Before turning off transceiver, wait until
        * Packet is sent and transmitter is turned off. 
        */
       sleep(60);
       hdlc_wait_idle();
       WAIT_TX_OFF;
       radio_on(false);
    }
    if (count < 0) count = 0;
    MUTEX_UNLOCK;
}


/***********************************************
 * Initialize
 ***********************************************/

void radio_init(SerialDriver* sd)
{  
   _serial = (Stream*) sd;
   setPinMode(TRX_SERIAL_RXD, PAL_MODE_ALTERNATIVE_3);
   setPinMode(TRX_SERIAL_TXD, PAL_MODE_ALTERNATIVE_3);
   setPinMode(TRX_PTT, PAL_MODE_OUTPUT_PUSHPULL);
   setPinMode(TRX_PTT_REV, PAL_MODE_OUTPUT_PUSHPULL);
   setPin(TRX_PTT);
   clearPin(TRX_PTT_REV);
   sdStart(sd, &_serialConfig);  
}
  
  
static void _initialize()
{  
   radio_PTT(false);
   sleep(600);
   _handshake();
   sleep (100);
  
   /* Get parameters from EEPROM */
   GET_PARAM(TRX_TX_FREQ, &_txfreq);
   GET_PARAM(TRX_RX_FREQ, &_rxfreq);
   _squelch = GET_BYTE_PARAM(TRX_SQUELCH);
   _flags = 0x00;
   _widebw = TRX_BANDWIDTH;
   _setGroupParm();  
   sleep(100);
//   radio_setMicLevel(8);
}
  
  
  
/***********************************************
 * Set TX and RX frequency (100 Hz units)
 ***********************************************/
  
bool radio_setFreq(uint32_t txfreq, uint32_t rxfreq)
{
    if (txfreq > 0)
       _txfreq = txfreq;
    if (rxfreq > 0)
       _rxfreq = rxfreq;
    return _setGroupParm();
}
 
 
/***********************************************
 * Set squelch level (1-8)
 ***********************************************/

bool radio_setSquelch(uint8_t sq) 
{
    _squelch = sq;
    if (_squelch > 8)
       _squelch = 0; 
    return _setGroupParm();
}


/************************************************
 * Squelch handler. 
 ************************************************/

void squelch_handler(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;
  
  chSysLock();
  if (!_sq_on && radio_rdy && !pinIsHigh(TRX_SQ)) {
    _sq_on = true;
    pri_rgb_led_on(true, true, false);
    afsk_rx_enable();
    channel_rdy = false;
  }
  else if (_sq_on) {
    _sq_on = false; 
    pri_rgb_led_off();
    afsk_rx_disable();
    channel_rdy = true;
    chCondBroadcastI(&_channel_rdy);
    chSchRescheduleS();
  }
  chSysUnlock();
}


/************************************************
 * Wait to radio is ready 
 ************************************************/

void radio_wait_enabled() {
  MUTEX_LOCK;
  while(!radio_rdy)
     WAIT_RADIO_READY;
  MUTEX_UNLOCK;
}


/************************************************
 * Wait to channel is ready 
 ************************************************/
void wait_channel_ready()
{
  MUTEX2_LOCK;
  /* Wait to radio is on and squelch is closed */
  rgb_led_on(false, false, true);
  while (!channel_rdy)
     WAIT_CHANNEL_READY;
  rgb_led_off();
  MUTEX2_UNLOCK;
}



/************************************************
 * Power on
 ************************************************/

void radio_on(bool on)
{
   if (on == _on)
      return; 
   _on = on;
   if (on) {
      setPin(TRX_PD);
      _initialize();
      radio_rdy=true;
      SIGNAL_RADIO_READY;
   }
   else {
      clearPin(TRX_PD);
      radio_rdy=false;
   }
}


/************************************************
 * PTT 
 ************************************************/

void radio_PTT(bool on)
{
    if (!_on)
       return;
    if (on) {
       WAIT_TX_OFF;
       clearPin(TRX_PTT);
       setPin(TRX_PTT_REV);
       tx_led_on();
    }
    else {
       clearPin(TRX_PTT_REV);
       setPin(TRX_PTT);
       tx_led_off();
       SIGNAL_TX_OFF;
    }
}



/************************************************
 * Set receiver volume (1-8)
 ************************************************/

bool radio_setVolume(uint8_t vol)
{
   if (!_on)
      return true;
   char reply[16];
   if (vol > 8)
      vol = 8;
   chprintf(_serial, "AT+DMOSETVOLUME=%1d\r\n", vol);
   readline(_serial, reply, 16);
   return (reply[13] == '0');
}


/************************************************
 * Set mic sensitivity (1-8)
 ************************************************/

bool radio_setMicLevel(uint8_t level)
{
  if (!_on)
    return true;
  if (level > 8)
    level = 8;
  char reply[16];
  chprintf(_serial, "AT+DMOSETMIC=%1d,0\r\n", level);
  readline(_serial, reply, 16);
  return (reply[13] == '0');
}


/*************************************************
 * If on=true, TX power is set to 0.5W. 
 * else it is set to 1W
 *************************************************/

bool radio_setLowTxPower(bool on)
{
  if (on)
    _flags ^= FLAG_LO_POWER;
  else
    _flags |= FLAG_LO_POWER;
  return _setGroupParm();
}


bool radio_isLowTxPower() {
  return !(_flags & FLAG_LO_POWER);
}


/************************************************
 * Auto powersave on/off. 
 ************************************************/

bool radio_setPowerSave(bool on)
{
   if (!_on)
      return true;
   char reply[16];
   chprintf(_serial, "AT+DMOAUTOPOWCONTR=%1d\r\n", (on ? 1:0));
   readline(_serial, reply, 16);
   return (reply[13] == '0');
}



static bool _handshake()
{
   char reply[16];
   chprintf(_serial, "AT+DMOCONNECT\r\n");
   readline(_serial, reply, 16);
   return (reply[14] == '0');
}


/***************************************************
 * Set a group of parameters 
 ***************************************************/

static bool _setGroupParm()
{
   if (!_on)
      return true;
   char txbuf[16], rxbuf[16], reply[16];
   sprintf(txbuf, "%lu.%04lu", _txfreq/10000, _txfreq%10000);
   sprintf(rxbuf, "%lu.%04lu", _rxfreq/10000, _rxfreq%10000);

   chprintf(_serial, "AT+DMOSETGROUP=%1d,%s,%s,00,%1d,00,%1d\r\n",
            _widebw, txbuf, rxbuf, _squelch, _flags);
   readline(_serial, reply, 16);
   return (reply[15] == '0');
}
