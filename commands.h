#include "defines.h"
#include "shell.h"
 
 thread_t* myshell_start(void);
 void readline(Stream * cbp, char* buf, const uint16_t max);
 
 extern const ShellCommand shell_commands[];
 