 
 char* parseFreq(char* val, char* buf, bool tx);
 char* parseDigipathTokens(int argc, char* argv[], char* buf);
 char* parseDigipath(char* line, char* buf);
// int tokenize(char* line, char *argv[], int maxarg);
 uint8_t tokenize(char* buf, char* tokens[], uint8_t maxtokens, char *delim, bool merge);