#ifndef EPSFAST_H
#define EPSFAST_H
#include "core.h"

void InitEpsilonFast(Space *sS, double sSqEps);
void CalcEpsilonFast(bool merge);
int Merged_count();

#endif
