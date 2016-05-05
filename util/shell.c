/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
    
    Slightly hacked by Øyvind Hanssen, LA7ECA, ohanssen@acm.org
*/

/**
 * @file    shell.c
 * @brief   Simple CLI shell code.
 *
 * @addtogroup SHELL
 * @{
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

/**
 * @brief   Shell termination event source.
 */
event_source_t shell_terminated;

static char *_strtok(char *str, const char *delim, char **saveptr) {
  char *token;
  if (str)
    *saveptr = str;
  token = *saveptr;

  if (!token)
    return NULL;

  token += strspn(token, delim);
  *saveptr = strpbrk(token, delim);
  if (*saveptr)
    *(*saveptr)++ = '\0';

  return *token ? token : NULL;
}


static void usage(BaseSequentialStream *chp, char *p) {

  chprintf(chp, "Usage: %s\r\n", p);
}


static void list_commands(BaseSequentialStream *chp, const ShellCommand *scp) {

  while (scp->sc_name != NULL) {
    chprintf(chp, "%s ", scp->sc_name);
    scp++;
  }
}


static void cmd_info(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    usage(chp, "info");
    return;
  }

  chprintf(chp, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
#ifdef PORT_COMPILER_NAME
  chprintf(chp, "Compiler:     %s\r\n", PORT_COMPILER_NAME);
#endif
  chprintf(chp, "Architecture: %s\r\n", PORT_ARCHITECTURE_NAME);
#ifdef PORT_CORE_VARIANT_NAME
  chprintf(chp, "Core Variant: %s\r\n", PORT_CORE_VARIANT_NAME);
#endif
#ifdef PORT_INFO
  chprintf(chp, "Port Info:    %s\r\n", PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  chprintf(chp, "Platform:     %s\r\n", PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  chprintf(chp, "Board:        %s\r\n", BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
  chprintf(chp, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif
}



static void cmd_systime(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    usage(chp, "systime");
    return;
  }
  chprintf(chp, "%lu\r\n", (unsigned long)chVTGetSystemTime());
}



/**
 * @brief   Array of the default commands.
 */
static ShellCommand local_commands[] = {
  {"info", "System information", 4, cmd_info},
  {"systime", "Get system time", 7, cmd_systime},
  {NULL, NULL, 0, NULL}
};



static const ShellCommand* findcommand( const ShellCommand *scp, char* name)
{
  unsigned len = strlen(name);
  while (scp->sc_name != NULL) 
  {
    if ( len >= scp->sc_minchars && 
         len <= strlen(scp->sc_name) && 
         strncasecmp(scp->sc_name, name, scp->sc_minchars) == 0
       )
         return scp;
    scp++;
  }
  return NULL;
}



static void cmdhelp( const ShellCommand *scp, BaseSequentialStream *chp, char *name) 
{
   if (strcasecmp(name, "exit")==0)
      chprintf(chp, "exit: Leave the shell");
   else if (strcasecmp(name, "help")==0)
      ;
   else {
      scp = findcommand(scp, name);
      if (scp == NULL) 
         scp = findcommand(local_commands, name);

      if (scp != NULL)
         chprintf(chp, "%s: %s\r\n\r\n", name, scp->sc_descr);
      else
         chprintf(chp, "%s: Unknown command\r\n\r\n", name);
   }
}



static bool cmdexec(const ShellCommand *scp, BaseSequentialStream *chp,
                      char *name, int argc, char *argv[]) 
{    
  scp = findcommand(scp, name);
  if (scp==NULL) 
    scp = findcommand(local_commands, name);
  
  if (scp != NULL) 
  {
     scp->sc_function(chp, argc, argv);
     chprintf(chp, "\r\n");
     return false;
  }
  else
    chprintf(chp, "%s: Unknown command\r\n\r\n", name);
  return true;
}




/**
 * @brief   Shell thread function.
 *
 * @param[in] p         pointer to a @p BaseSequentialStream object
 */
static THD_FUNCTION(shell_thread, p) {
  int n;
  BaseSequentialStream *chp = ((ShellConfig *)p)->sc_channel;
  const ShellCommand *scp = ((ShellConfig *)p)->sc_commands;
  char *lp, *cmd, *tokp, line[SHELL_MAX_LINE_LENGTH];
  char *args[SHELL_MAX_ARGUMENTS + 1];

  chRegSetThreadName("shell");
  chprintf(chp, "\r\nChibiOS/RT Shell\r\n");
  while (true) {
    chprintf(chp, "ch> ");
    if (shellGetLine(chp, line, sizeof(line))) {
      chprintf(chp, "\r\nlogout");
      break;
    }
    lp = _strtok(line, " \t", &tokp);
    cmd = lp;
    n = 0;
    while ((lp = _strtok(NULL, " \t", &tokp)) != NULL) {
      if (n >= SHELL_MAX_ARGUMENTS) {
        chprintf(chp, "too many arguments\r\n");
        cmd = NULL;
        break;
      }
      args[n++] = lp;
    }
    args[n] = NULL;
    if (cmd != NULL) {
      
      if (strcasecmp(cmd, "exit") == 0) {
        if (n > 0) {
          usage(chp, "exit");
          continue;
        }
        break;
      }
      
      else if ( strcmp(cmd, "?") == 0 || 
                strcasecmp(cmd, "help") == 0 ) {
        if (n > 1) {
          usage(chp, "help | help <command>");
          continue;
        }
        else if (n == 1) {
           cmdhelp(scp, chp, args[0]);
           continue;
        }
        chprintf(chp, "Commands: help exit ");
        list_commands(chp, local_commands);
        if (scp != NULL)
          list_commands(chp, scp);
        chprintf(chp, "\r\n\r\n");
      }
      
      else cmdexec(scp, chp, cmd, n, args);
    }
  }
  shellExit(MSG_OK);
}



/**
 * @brief   Shell manager initialization.
 *
 * @api
 */
void shellInit(void) {

  chEvtObjectInit(&shell_terminated);
}




/**
 * @brief   Terminates the shell.
 * @note    Must be invoked from the command handlers.
 * @note    Does not return.
 *
 * @param[in] msg       shell exit code
 *
 * @api
 */
void shellExit(msg_t msg) {

  /* Atomically broadcasting the event source and terminating the thread,
     there is not a chSysUnlock() because the thread terminates upon return.*/
  chSysLock();
  chEvtBroadcastI(&shell_terminated);
  chThdExitS(msg);
}

/**
 * @brief   Spawns a new shell.
 * @pre     @p CH_CFG_USE_HEAP and @p CH_CFG_USE_DYNAMIC must be enabled.
 *
 * @param[in] scp       pointer to a @p ShellConfig object
 * @param[in] size      size of the shell working area to be allocated
 * @param[in] prio      priority level for the new shell
 * @return              A pointer to the shell thread.
 * @retval NULL         thread creation failed because memory allocation.
 *
 * @api
 */
#if CH_CFG_USE_HEAP && CH_CFG_USE_DYNAMIC
thread_t *shellCreate(const ShellConfig *scp, size_t size, tprio_t prio) {

  return chThdCreateFromHeap(NULL, size, "shell", prio, shell_thread, (void *)scp);
}
#endif

/**
 * @brief   Create statically allocated shell thread.
 *
 * @param[in] scp       pointer to a @p ShellConfig object
 * @param[in] wsp       pointer to a working area dedicated to the shell thread stack
 * @param[in] size      size of the shell working area
 * @param[in] prio      priority level for the new shell
 * @return              A pointer to the shell thread.
 *
 * @api
 */
thread_t *shellCreateStatic(const ShellConfig *scp, void *wsp,
                            size_t size, tprio_t prio) {

  return chThdCreateStatic(wsp, size, prio, shell_thread, (void *)scp);
}

/**
 * @brief   Reads a whole line from the input channel.
 *
 * @param[in] chp       pointer to a @p BaseSequentialStream object
 * @param[in] line      pointer to the line buffer
 * @param[in] size      buffer maximum length
 * @return              The operation status.
 * @retval true         the channel was reset or CTRL-D pressed.
 * @retval false        operation successful.
 *
 * @api
 */
bool shellGetLine(BaseSequentialStream *chp, char *line, unsigned size) {
  char *p = line;

  while (true) {
    char c;

    if (streamRead(chp, (uint8_t *)&c, 1) == 0)
      return true;
    if (c == 4) {
      chprintf(chp, "^D");
      return true;
    }
    if ((c == 8) || (c == 127)) {
      if (p != line) {
        streamPut(chp, c);
        streamPut(chp, 0x20);
        streamPut(chp, c);
        p--;
      }
      continue;
    }
    if (c == '\r') {
      chprintf(chp, "\r\n");
      *p = 0;
      return false;
    }
    if (c < 0x20)
      continue;
    if (p < line + size - 1) {
      streamPut(chp, c);
      *p++ = (char)c;
    }
  }
}

/** @} */
