#ifndef UTILS_H_
#define UTILS_H_

int PrintBody(std::ostream& os, Space *S);
int PrintVorticity(std::ostream& os, Space *S, bool PrintSpeed = false);
int PrintHeat(std::ostream& os, Space *S, bool PrintSpeed = false);


#endif
