/*
 * Definition of parameters to be stored in EEPROM, their default 
 * values in program memory. Macros for defining and accessing these values. 
 * 
 * Based on Polaric Tracker code.
 */
 

#if !defined __CONFIG_H__
#define __CONFIG_H__

#include "util/eeprom.h"
#include <stdint.h>
#include "ax25.h"
#include "ui/wifi.h"

#if defined __CONFIG_C__

    
#define DEFINE_PARAM(name, offs, type) \
    const uint16_t name##_offset = offs; \
    typedef type name##_type  
    /* FIXME: We need to calculate offsets manually. Can this be computed? */
    
#define DEFAULT_PARAM(name) \
const name##_type name##_default
#else

#define DEFINE_PARAM(name, prev, type) \
    extern const uint16_t name##_offset; \
    typedef type name##_type ; \
    extern const type name##_default
#endif
    
#define COMMENT_LENGTH 40
#define OBJID_LENGTH 10
#define CRED_LENGTH 32
#define INET_NAME_LENGTH 40

typedef uint8_t Byte; 
typedef uint16_t Word; 
typedef uint32_t Dword;

typedef addr_t __digilist_t[7];  
typedef char comment[COMMENT_LENGTH];
typedef char credential[CRED_LENGTH];
typedef char obj_id_t[OBJID_LENGTH];
typedef char inet_name[INET_NAME_LENGTH]; 

typedef ap_config_t __aplist_t;        // 64 bytes
   

/***************************************************************
 * Define parameters:
 *         Name  address  Type        
 ***************************************************************/ 

DEFINE_PARAM ( VERSION_KEY,        0,   Word );
DEFINE_PARAM ( TRX_TX_FREQ,        3,   Dword );
DEFINE_PARAM ( TRX_RX_FREQ,        8,   Dword );
DEFINE_PARAM ( TRX_SQUELCH,        13,  Byte );
DEFINE_PARAM ( TRX_VOLUME,         15,  Byte );
DEFINE_PARAM ( TXDELAY,            17,  Byte );
DEFINE_PARAM ( TXTAIL,             19,  Byte );
DEFINE_PARAM ( MAXFRAME,           21,  Byte );
DEFINE_PARAM ( MYCALL,             23,  addr_t );
DEFINE_PARAM ( DEST,               33,  addr_t );
DEFINE_PARAM ( NDIGIS,             43,  Byte ); 
DEFINE_PARAM ( DIGIS,              45,  __digilist_t );
DEFINE_PARAM ( SYMBOL,             100, Byte );
DEFINE_PARAM ( SYMBOL_TAB,         102, Byte ); 
DEFINE_PARAM ( TRACKER_ON,         104, Byte );
DEFINE_PARAM ( TRACKER_TURN_LIMIT, 106, Word );
DEFINE_PARAM ( TRACKER_MAXPAUSE,   109, Byte );
DEFINE_PARAM ( TRACKER_MINDIST,    111, Byte ); 
DEFINE_PARAM ( TRACKER_MINPAUSE,   113, Byte ); 
DEFINE_PARAM ( STATUS_TIME,        115, Byte );
DEFINE_PARAM ( TIMESTAMP_ON,       117, Byte );
DEFINE_PARAM ( COMPRESS_ON,        119, Byte );
DEFINE_PARAM ( ALTITUDE_ON,        121, Byte );
DEFINE_PARAM ( OBJ_SYMBOL,         123, Byte );
DEFINE_PARAM ( OBJ_SYMBOL_TABLE,   125, Byte );
DEFINE_PARAM ( OBJ_ID,             127, obj_id_t );
DEFINE_PARAM ( REPORT_COMMENT,     138, comment ); 
DEFINE_PARAM ( REPORT_BEEP_ON,     179, Byte );
DEFINE_PARAM ( REPEAT_ON,          181, Byte );
DEFINE_PARAM ( EXTRATURN_ON,       183, Byte );
DEFINE_PARAM ( FAKE_REPORTS_ON,    185, Byte );
DEFINE_PARAM ( GPS_POWERSAVE_ON,   187, Byte );
DEFINE_PARAM ( TXMON_ON,           189, Byte );
DEFINE_PARAM ( DIGIP_WIDE1_ON,     191, Byte );
DEFINE_PARAM ( DIGIP_SAR_ON,       193, Byte );
DEFINE_PARAM ( DIGIPEATER_ON,      195, Byte );
DEFINE_PARAM ( IGATE_ON,           197, Byte );
DEFINE_PARAM ( IGATE_HOST,         199, inet_name); 
DEFINE_PARAM ( IGATE_PORT,         240, Word );
DEFINE_PARAM ( IGATE_USERNAME,     243, credential );              
DEFINE_PARAM ( IGATE_PASSCODE,     275, Word );
DEFINE_PARAM ( IGATE_FILTER,       278, credential );   
DEFINE_PARAM ( WIFI_ON,            311, Byte );
DEFINE_PARAM ( HTTP_ON,            313, Byte );
DEFINE_PARAM ( HTTP_USER,          315, credential );
DEFINE_PARAM ( HTTP_PASSWD,        348, credential );
DEFINE_PARAM ( SOFTAP_PASSWD,      381, credential );
DEFINE_PARAM ( WIFIAP,             414, __aplist_t );   /* 6 instances = 390 bytes */

#if defined __CONFIG_C__

/***************************************************************
 * Default values for parameters to be stored in program
 * memory. This MUST be done for each parameter defined above
 * or the linker will complain.
 ***************************************************************/

DEFAULT_PARAM( VERSION_KEY )         = 0;
DEFAULT_PARAM( TRX_TX_FREQ )         = 1448000;
DEFAULT_PARAM( TRX_RX_FREQ )         = 1448000;
DEFAULT_PARAM( TRX_SQUELCH )         = 4;
DEFAULT_PARAM( TRX_VOLUME )          = 4;
DEFAULT_PARAM( TXDELAY )             = 20;
DEFAULT_PARAM( TXTAIL )              = 10;
DEFAULT_PARAM( MAXFRAME )            = 2;
DEFAULT_PARAM( MYCALL )              = {"NOCALL",0,0};
DEFAULT_PARAM( DEST )                = {"APAT01",0,0};
DEFAULT_PARAM( NDIGIS )              = 2;
DEFAULT_PARAM( DIGIS )               = {{"WIDE1",1,0}, {"WIDE2",2,0}};
DEFAULT_PARAM( SYMBOL )              = '[';
DEFAULT_PARAM( SYMBOL_TAB )          = '/';
DEFAULT_PARAM( TRACKER_ON )          = 0;
DEFAULT_PARAM( TRACKER_TURN_LIMIT )  = 35;
DEFAULT_PARAM( TRACKER_MAXPAUSE )    = 18;
DEFAULT_PARAM( TRACKER_MINDIST )     = 100;
DEFAULT_PARAM( TRACKER_MINPAUSE )    = 3;
DEFAULT_PARAM( STATUS_TIME )         = 30;
DEFAULT_PARAM( TIMESTAMP_ON)         = 1;
DEFAULT_PARAM( COMPRESS_ON)          = 0;
DEFAULT_PARAM( ALTITUDE_ON)          = 0;
DEFAULT_PARAM( OBJ_SYMBOL)           = 'c';
DEFAULT_PARAM( OBJ_SYMBOL_TABLE)     = '/';
DEFAULT_PARAM( OBJ_ID)               = "MARK-";
DEFAULT_PARAM( REPORT_COMMENT )      = "Arctic Tracker";
DEFAULT_PARAM( REPORT_BEEP_ON )      = 0;
DEFAULT_PARAM( REPEAT_ON )           = 0;
DEFAULT_PARAM( EXTRATURN_ON )        = 0;
DEFAULT_PARAM( FAKE_REPORTS_ON )     = 0;
DEFAULT_PARAM( GPS_POWERSAVE_ON )    = 0;
DEFAULT_PARAM( TXMON_ON )            = 0;
DEFAULT_PARAM( DIGIP_WIDE1_ON )      = 0; 
DEFAULT_PARAM( DIGIP_SAR_ON )        = 0;
DEFAULT_PARAM( DIGIPEATER_ON )       = 0;
DEFAULT_PARAM( IGATE_ON )            = 0;
DEFAULT_PARAM( IGATE_HOST )          = "aprs.no";  
DEFAULT_PARAM( IGATE_PORT )          = 14580;
DEFAULT_PARAM( IGATE_USERNAME )      = "nocall";
DEFAULT_PARAM( IGATE_PASSCODE )      = 0; 
DEFAULT_PARAM( IGATE_FILTER )        = "";
DEFAULT_PARAM( WIFI_ON )             = 1;
DEFAULT_PARAM( HTTP_ON )             = 1;
DEFAULT_PARAM( HTTP_USER )           = "user";
DEFAULT_PARAM( HTTP_PASSWD )         = "password";
DEFAULT_PARAM( SOFTAP_PASSWD )       = "password"; 
DEFAULT_PARAM( WIFIAP )              = {"", ""}; 

#endif

#define RESET_PARAM(x)         reset_param(x##_offset, sizeof(x##_type))
#define GET_PARAM(x, val)      get_param(x##_offset, (val), sizeof(x##_type), &x##_default)
#define SET_PARAM(x, val)      set_param(x##_offset, (val), sizeof(x##_type))
#define GET_BYTE_PARAM(x)      get_byte_param(x##_offset, &x##_default)
#define SET_BYTE_PARAM(x, val) set_byte_param(x##_offset, ((uint8_t) val))

#define GET_PARAM_I(x, i, val) get_param(x##_offset + ((i)*(sizeof(x##_type)+1)), (val), sizeof(x##_type), &x##_default)
#define SET_PARAM_I(x, i, val) set_param(x##_offset + ((i)*(sizeof(x##_type)+1)), (val), sizeof(x##_type))


void    reset_param(uint16_t);
void    set_param(uint16_t, void*, const uint8_t);
int     get_param(uint16_t, void*, const uint8_t, const void*);
void    set_byte_param(uint16_t, uint8_t);
uint8_t get_byte_param(uint16_t, const void*);


#endif



    