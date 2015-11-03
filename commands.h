 #if !defined __COMMANDS_H__
 #define __COMMANDS_H__

#include "defines.h"
#include "shell.h"
 
 thread_t* myshell_start(void);
 bool readline(Stream * cbp, char* buf, const uint16_t max);

#endif