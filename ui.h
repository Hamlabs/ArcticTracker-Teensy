#if !defined __UI_H__
#define __UI_H__

void ui_init(void);
void rgb_led_on(bool, bool, bool);
void rgb_led_off(void);
void rgb_led_mix(uint8_t red, uint8_t green, uint8_t blue, uint8_t off);

void dcd_led_on(void);
void dcd_led_off(void);

void button_handler(EXTDriver *extp, expchannel_t channel);

#endif
