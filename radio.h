 
 #include <stdint.h>
 
 
 void radio_init(SerialDriver* sd);
 bool radio_setFreq(uint32_t txfreq, uint32_t rxfreq);
 bool radio_setSquelch(uint8_t sq);
 void radio_on(bool on);
 void radio_PTT(bool on);
 bool radio_setVolume(uint8_t vol);
 bool radio_setMicLevel(uint8_t level);
 bool radio_powerSave(bool on);
 
