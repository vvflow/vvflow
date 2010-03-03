#ifndef UTILS_H_
#define UTILS_H_

#include <libxml/parser.h>
#include <libxml/tree.h>

int PrintBody(std::ostream& os, Space *S);
int PrintVorticity(std::ostream& os, Space *S, bool PrintSpeed = false);
int PrintHeat(std::ostream& os, Space *S, bool PrintSpeed = false);

char* BinToBase64(char *src, int srclen);
char* Base64ToBin(char *src, int srclen);

#endif
