 
 /*
  *  Adapted from Chibios lcd3310.c
  *  ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio
  * 
  *  Licensed under the Apache License, Version 2.0 (the "License");
  *  you may not use this file except in compliance with the License.
  *  You may obtain a copy of the License at
  * 
  *      http://www.apache.org/licenses/LICENSE-2.0
  * 
  *  Unless required by applicable law or agreed to in writing, software
  *  distributed under the License is distributed on an "AS IS" BASIS,
  *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  *  See the License for the specific language governing permissions and
  *  limitations under the License.
  */
 
#include "ch.h"
#include "hal.h"
#include "defines.h"
#include "lcd.h"

static SPIDriver *_spip;
 
 
 
 void lcd_init(SPIDriver *spip) {
   
   _spip = spip;
   /* Reset LCD */
   palClearPad(LCD_RES_PORT, LCD_RES_PIN);
   sleep(15);
   palSetPad(LCD_RES_PORT, LCD_RES_PIN);
   sleep(15);
   
   /* Send configuration commands to LCD */
   lcd_writeByte(0x21, LCD_SEND_CMD);  /* LCD extended commands */
   lcd_writeByte(0xC8, LCD_SEND_CMD);  /* Set LCD Vop (Contrast) */
   lcd_writeByte(0x05, LCD_SEND_CMD);  /* Set start line S6 to 1 TLS8204 */
   lcd_writeByte(0x40, LCD_SEND_CMD);  /* Set start line S[5:0] to 0x00 TLS8204 */
   lcd_writeByte(0x12, LCD_SEND_CMD);  /* LCD bias mode 1:68. */
   lcd_writeByte(0x20, LCD_SEND_CMD);  /* LCD standard Commands, horizontal addressing mode. */
   lcd_writeByte(0x08, LCD_SEND_CMD);  /* LCD blank */
   lcd_writeByte(0x0C, LCD_SEND_CMD);  /* LCD in normal mode. */
   
   lcd_clear(); /* Clear LCD */
   lcd_setPosXY(1, 1);
 }
 
 
 
 
 
 void lcd_writeByte(uint8_t data, uint8_t cd) {
   
   spiSelect(_spip);
   
   if(cd == LCD_SEND_DATA) {
     palSetPad(LCD_DC_PORT, LCD_DC_PIN);
   }
   else {
     palClearPad(LCD_DC_PORT, LCD_DC_PIN);
   }
   
   spiSend(_spip, 1, &data);   // change to normal spi send
   spiUnselect(_spip);
 }
 
 
 
 

 
 void lcd_clear() { // ok
   uint32_t i, j;
   
   for (i = 0; i < LCD_Y_RES/LCD_FONT_Y_SIZE; i++) {
     lcd_setPosXY(0, i);
     for (j = 0; j < LCD_X_RES; j++)
       lcd_writeByte(0x00, LCD_SEND_DATA);
   }
   
 }
 

 
 
 
 void lcd_setPosXY(uint8_t x, uint8_t y) {
   
   if (y > LCD_Y_RES/LCD_FONT_Y_SIZE) return;
   if (x > LCD_X_RES) return;
   
   lcd_writeByte(0x80 | x, LCD_SEND_CMD);   /* Set x position */
   lcd_writeByte(0x40 | y, LCD_SEND_CMD);   /* Set y position */  
 }
 
 

 
 
 
 void lcd_Contrast (uint8_t contrast) {
   
   lcd_writeByte(0x21, LCD_SEND_CMD);              /* LCD Extended Commands */
   lcd_writeByte(0x80 | contrast, LCD_SEND_CMD);    /* Set LCD Vop (Contrast) */
   lcd_writeByte(0x20, LCD_SEND_CMD);              /* LCD Standard Commands, horizontal addressing mode */
 }
 
 
 