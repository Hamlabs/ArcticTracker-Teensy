#if !defined __WIFI_H__
#define __WIFI_H__
  
#include "hal.h"

typedef struct {
  char ssid[32];
  char passwd[32];
} ap_config_t; 

typedef struct {
  char username[32];
  char passwd[32];
} https_auth_t;


void  wifi_external(void);
void  wifi_internal(void);
void  wifi_enable(void);
void  wifi_disable(void);
void  wifi_shell(Stream* chp);
void  wifi_init(SerialDriver* sd);
char* wifi_doCommand(char*, char*); 
char* wifi_status(char* buf);

#endif