#ifndef UTILS_H_
#define UTILS_H_

#ifdef XML_ENABLE
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

int PrintBody(std::ostream& os, Space *S);
int PrintVorticity(std::ostream& os, Space *S, bool PrintSpeed = false);
int PrintHeat(std::ostream& os, Space *S, bool PrintSpeed = false);

#ifdef XML_ENABLE
char* BinToBase64(char *src, int srclen);
char* Base64ToBin(char *src, int srclen);
#endif

#endif
