#ifndef _DIFFUSIVEFAST_H_
#define _DIFFUSIVEFAST_H_
#include <math.h>
#include "libVVHD/core.h"

int InitDiffusiveFast(Space *sS, double sRe);
int CalcVortexDiffusiveFast();
int CalcHeatDiffusiveFast();

#endif
