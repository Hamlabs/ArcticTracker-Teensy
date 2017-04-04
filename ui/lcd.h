/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/



#ifndef LCD_H
#define LCD_H



#define LCD_X_RES                  84
#define LCD_Y_RES                  48

#define LCD_FONT_X_SIZE             5
#define LCD_FONT_Y_SIZE             8

#define LCD_SEND_CMD                0
#define LCD_SEND_DATA               1



void lcd_init(SPIDriver *spip);
void lcd_backlight(void);
void lcd_writeByte(uint8_t data, uint8_t cd);
void lcd_contrast(uint8_t contrast);
void lcd_clear(void);
void lcd_setPosXY(uint8_t x, uint8_t y);



#endif /* LCD_H */


