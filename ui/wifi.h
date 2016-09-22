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


#define ERR_CONNECT_FAIL 1
#define ERR_UNKNOWN_HOST 2
#define ERR_DISCONNECTED 3


void  wifi_external(void);
void  wifi_internal(void);
void  wifi_enable(void);
void  wifi_disable(void);
bool  wifi_is_enabled(void);
void  wifi_restart(void);
void  wifi_shell(Stream* chp);
void  wifi_init(SerialDriver* sd);
char* wifi_doCommand(char*, char*); 
char* wifi_status(char* buf);

int  inet_open(char* host, int port);
void inet_close(void);
bool inet_is_connected(void);
int  inet_read(char* buf);
void inet_write(char* text);
void inet_mon_on(bool on);
#endif