#include <stdint.h>

#define KINETISK



void     eeprom_initialize  (void);
uint8_t  eeprom_read_byte   (const uint16_t *addr);
uint16_t eeprom_read_word   (const uint16_t *addr);
uint32_t eeprom_read_dword  (const uint16_t *addr);
void     eeprom_read_block  (void *buf, const void *addr, uint32_t len);
int      eeprom_is_ready    (void);
void     eeprom_write_byte  (uint16_t *addr, uint8_t value);
void     eeprom_write_word  (uint16_t *addr, uint16_t value);
void     eeprom_write_dword (uint16_t *addr, uint32_t value);
void     eeprom_write_block (const void *buf, void *addr, uint32_t len);

