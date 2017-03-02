/*
 * Code for reading/writing parameters in EEPROM, their default values in
 * program memory. Based on Polaric Tracker code.
 */

#define __CONFIG_C__   /* IMPORTANT */

#include "ch.h"
#include "chsys.h"
#include "defines.h"
#include "config.h"
#include "string.h"
#include "hal.h"

#define BYTEPTR(x) (uint8_t*)(uint32_t)(x)
#define WORDPTR(x) (uint16_t*)(uint32_t)(x)
#define PTR(x) (void*)(uint32_t)(x)

void reset_param(uint16_t ee_addr)
{   
   while (!eeprom_is_ready())
      t_yield();
   eeprom_write_byte(WORDPTR(ee_addr), 0xff);
}


/************************************************************************
 * Write config parameter into EEPROM. 
 ************************************************************************/

void set_param(uint16_t ee_addr, void* ram_addr, const uint8_t size)
{
   register uint8_t byte, checksum = 0x0f;
   register void* addr = ram_addr;
   while (!eeprom_is_ready())
      t_yield();
   for (int i=0; i<size; i++)
   {
       byte = *((uint8_t*) addr++);
       checksum ^= byte;
   }
   eeprom_write_block(ram_addr, PTR(ee_addr), size);  
   eeprom_write_byte(WORDPTR(ee_addr+size), checksum);
}



/************************************************************************
 * Get config parameter from EEPROM. 
 * If checksum does not match, use default value from program memory 
 * instead and return 1. Value is copied into ram_addr.
 ************************************************************************/
   
int get_param(uint16_t ee_addr, void* ram_addr, const uint8_t size, const void* default_val)
{
   register uint8_t checksum = 0x0f, s_checksum;
   register void* dest = ram_addr;

   while (!eeprom_is_ready())
      t_yield();
   eeprom_read_block(ram_addr, PTR(ee_addr), size);
   for (int i=0;i<size; i++)
       checksum ^= *((uint8_t*) dest++);
   s_checksum = eeprom_read_byte(PTR(ee_addr+size));
   if (s_checksum != checksum) {
      memcpy(ram_addr, default_val, size);
      return 1;
   }
   else 
      return 0;
} 



/************************************************************************
 * Write single byte config parameter into EEPROM. 
 ************************************************************************/

void set_byte_param(uint16_t ee_addr, uint8_t byte)
{
    while (!eeprom_is_ready())
      t_yield();
    eeprom_write_byte(WORDPTR(ee_addr), byte);
    eeprom_write_byte(PTR(ee_addr+1), (0x0f ^ byte));
}


 
/************************************************************************
 * Get (and return) single byte config parameter from EEPROM. 
 * If checksum does not match, use default value from flash memory 
 * instead.
 ************************************************************************/
 
uint8_t get_byte_param(uint16_t ee_addr, const void* default_val)
{
    while (!eeprom_is_ready())
      t_yield();
    register uint8_t b1 = eeprom_read_byte(WORDPTR(ee_addr));
    register uint8_t b2 = eeprom_read_byte(WORDPTR(ee_addr+1));
    if ((0x0f ^ b1) == b2)
       return b1;
    else
       return *((uint8_t*) default_val);
}













