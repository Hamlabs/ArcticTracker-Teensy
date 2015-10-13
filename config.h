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
    
    
typedef uint8_t Byte; 
typedef uint16_t Word; 
typedef uint32_t Dword;

typedef addr_t __digilist_t[7];  

/***************************************************************
 * Define parameters:
 *         Name  address  Type      
 ***************************************************************/ 

DEFINE_PARAM ( VERSION_KEY,    0,     Word );
DEFINE_PARAM ( TRX_TX_FREQ,    3,     Dword );
DEFINE_PARAM ( TRX_RX_FREQ,    8,     Dword );
DEFINE_PARAM ( TRX_SQUELCH,    13,    Byte );
DEFINE_PARAM ( TRX_VOLUME,     15,    Byte );
DEFINE_PARAM ( TXDELAY,        17,    Byte );
DEFINE_PARAM ( TXTAIL,         19,    Byte );
DEFINE_PARAM ( MAXFRAME,       21,    Byte );
DEFINE_PARAM ( MYCALL,         23,    addr_t );
DEFINE_PARAM ( DEST,           33,    addr_t );
DEFINE_PARAM ( NDIGIS,         43,    Byte ); 
DEFINE_PARAM ( DIGIS,          45,   __digilist_t );


#if defined __CONFIG_C__

/***************************************************************
 * Default values for parameters to be stored in program
 * memory. This MUST be done for each parameter defined above
 * or the linker will complain.
 ***************************************************************/

DEFAULT_PARAM( VERSION_KEY )  = 0;
DEFAULT_PARAM( TRX_TX_FREQ )  = 1448000;
DEFAULT_PARAM( TRX_RX_FREQ )  = 1448000;
DEFAULT_PARAM( TRX_SQUELCH )  = 4;
DEFAULT_PARAM( TRX_VOLUME )   = 4;
DEFAULT_PARAM( TXDELAY )      = 100;
DEFAULT_PARAM( TXTAIL )       = 10;
DEFAULT_PARAM( MAXFRAME )     = 2;
DEFAULT_PARAM( MYCALL )       = {"LA7ECA",0,0};
DEFAULT_PARAM( DEST )         = {"APAT01",0,0};
DEFAULT_PARAM( NDIGIS )       = 2;
DEFAULT_PARAM( DIGIS )        = {{"WIDE1",1,0}, {"WIDE2",2,0}};

#endif

#define RESET_PARAM(x)         reset_param(x##_offset, sizeof(x##_type))
#define GET_PARAM(x, val)      get_param(x##_offset, (val), sizeof(x##_type), &x##_default)
#define SET_PARAM(x, val)      set_param(x##_offset, (val), sizeof(x##_type))
#define GET_BYTE_PARAM(x)      get_byte_param(x##_offset, &x##_default)
#define SET_BYTE_PARAM(x, val) set_byte_param(x##_offset, ((uint8_t) val))

void    reset_param(uint16_t);
void    set_param(uint16_t, void*, const uint8_t);
int     get_param(uint16_t, void*, const uint8_t, const void*);
void    set_byte_param(uint16_t, uint8_t);
uint8_t get_byte_param(uint16_t, const void*);


#endif



    