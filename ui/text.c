 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "defines.h"
#include "config.h"
#include "ui/text.h"
#include "radio.h"



/*********************************************************************
 * Parse and set frequency
 *   val - text representation of frequency
 *   buf - buffer for result message
 *   tx  - true if TX frequency, false if RX
 *
 * Returns buf
 *********************************************************************/

char* parseFreq(char* val, char* buf, bool tx)
{
   uint32_t f = 0;
   if (sscanf(val, "%ld", &f) == 1) {
     if (f < TRX_MIN_FREQUENCY)
       sprintf(buf, "ERROR. Frequency is below lower limit");
     else if (f > TRX_MAX_FREQUENCY)
       sprintf(buf, "ERROR. Frequency is above upper limit");
     else { 
       sprintf(buf, "OK");
       if (tx) {
         SET_PARAM(TRX_TX_FREQ, &f);
         radio_setFreq(f, 0);
       }
       else {   
         SET_PARAM(TRX_RX_FREQ, &f);
         radio_setFreq(0, f);
       }
     }
   }
   else
     sprintf(buf, "ERROR, Couldn't parse input. Wrong format?");
   
   return buf;
}



/********************************************************************
 * Parse and set symbol
 ********************************************************************/

char* parseSymbol(char* val, char* buf)
{
   if (strlen(val) != 2) 
      sprintf(buf, "ERROR. Symbol should be two characters\r");
   else {
      SET_BYTE_PARAM(SYMBOL_TAB, val[0]);
      SET_BYTE_PARAM(SYMBOL, val[1]);
      sprintf(buf, "OK");
   }
   return buf;
}


 
/*********************************************************************
 * Parse and set digipeater path
 *  line - input text. Digipeaters separated by comma or whitespace
 *  buf - buffer for result message
 * 
 * Returns buf
 *********************************************************************/

char* parseDigipath(char* line, char* buf)
{
  char* argv[7];
  int argc;
  argc = tokenize(line, argv, 7, " \t,", false);
  return parseDigipathTokens(argc, argv, buf);
}



/*********************************************************************
 * Parse and set digipeater path from token list (argc, argv)
 * 
 *********************************************************************/

char* parseDigipathTokens(int argc, char* argv[], char* buf)
{
   __digilist_t digis;
   uint8_t ndigis;
   if (argc==1 && strncasecmp("off", argv[0], 3)==0)
      ndigis = 0;
   else {
      ndigis = args2digis(digis, argc, argv);
      SET_PARAM(DIGIS, digis);     
   }
   SET_BYTE_PARAM(NDIGIS, ndigis);
   sprintf(buf, "Ok\r\n");
   return buf;
}






/****************************************************************************
 * split input string into tokens - returns number of tokens found
 *
 * ARGUMENTS: 
 *   buf       - text buffer to tokenize
 *   tokens    - array in which to store pointers to tokens
 *   maxtokens - maximum number of tokens to scan for
 *   delim     - characters which can be used as delimiters between tokens
 *   merge     - if true, merge empty tokens
 *
 * Text which is enclosed by " " is regarded as a single token. 
 ****************************************************************************/

uint8_t tokenize(char* buf, char* tokens[], uint8_t maxtokens, char *delim, bool merge)
{ 
  register uint8_t ntokens = 0;
  while (ntokens<maxtokens)
  {
    if (*buf == '\"') {
      /* Special case: token is enclosed in " */
      tokens[ntokens++] = ++buf;
      char *endt = strchrnul(buf, '\"'); 
      *endt = '\0';
      buf = endt + 1;
    }
    else {    
      tokens[ntokens] = strsep(&buf, delim);
      if ( buf == NULL)
        break;
      if (!merge || *tokens[ntokens] != '\0') 
        ntokens++;
    }
  }
  return (merge && *tokens[ntokens] == '\0' ? ntokens : ntokens+1);
}
