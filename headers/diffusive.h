#ifndef _DIFFUSIVE_H_
#define _DIFFUSIVE_H_
#include <math.h>
#include "core.h"

int InitDiffusive(Space *sS, double sRe);
int CalcVortexDiffusive();
int CalcHeatDiffusive();

#endif

