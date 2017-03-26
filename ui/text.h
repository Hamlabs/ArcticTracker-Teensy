 
 char* parseFreq(char* val, char* buf, bool tx);
 char* parseDigipathTokens(int argc, char* argv[], char* buf);
 char* parseDigipath(char* line, char* buf);
 char* parseSymbol(char* val, char* buf);
 uint8_t tokenize(char* buf, char* tokens[], uint8_t maxtokens, char *delim, bool merge);
 char* parseBoolSetting(uint16_t ee_addr, char* val, char* buf);
 char* printBoolSetting(uint16_t ee_addr, const void* default_val, char* buf);
 char* parseTurnLimit(char* val, char* buf);
 char* parseByteSetting(uint16_t ee_addr, char* val, uint8_t llimit, uint8_t ulimit, char* buf);
 char* parseWordSetting(uint16_t ee_addr, char* val, uint16_t llimit, uint16_t ulimit, char* buf);
 
#define PARSE_BOOL(x, val, buf) parseBoolSetting(x##_offset, val, buf)
#define PRINT_BOOL(x, buf) printBoolSetting(x##_offset, &x##_default, buf)
#define PARSE_BYTE(x, val, llimit, ulimit, buf) parseByteSetting(x##_offset, val, llimit, ulimit, buf)
#define PARSE_WORD(x, val, llimit, ulimit, buf) parseWordSetting(x##_offset, val, llimit, ulimit, buf)
