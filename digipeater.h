 #if !defined __DIGIPEATER_H__
 #define __DIGIPEATER_H__
 
 #include <inttypes.h>
 
 #define HEARDLIST_SIZE 32
 #define HEARDLIST_MAX_AGE 6
 
 void digipeater_init(void);
 void digipeater_on(bool m);
 void digipeater_activate(bool m);
 
 bool hlist_exists(uint16_t x);
 void hlist_add(uint16_t x);
 void hlist_tick(void);
 
 #endif /* __DIGIPEATER_H__ */