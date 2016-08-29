#if !defined __UI_H__
#define __UI_H__

void ui_init(void);
void rgb_led_on(bool, bool, bool);
void rgb_led_off(void);
void rgb_led_mix(uint8_t red, uint8_t green, uint8_t blue, uint8_t off);

void tx_led_on(void);
void tx_led_off(void);

void button_handler(EXTDriver *extp, expchannel_t channel);


void _beep(uint16_t freq, uint16_t time); 
void beeps(char* s);
void blipUp(void);
void blipDown(void);
void ring(void);

#define BEEP_FREQ 2000
#define BEEP_ALT_FREQ 1900

#define beep(t) _beep(BEEP_FREQ, (t))
#define lbeep(t) _beep(BEEP_ALT_FREQ, (t))


#endif
