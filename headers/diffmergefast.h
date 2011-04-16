#ifndef _DIFFMERGEFAST_H_
#define _DIFFMERGEFAST_H_
#include <math.h>
#include "core.h"

int InitDiffMergeFast(Space *sS, double sRe, double sMergeSqEps);
int CalcVortexDiffMergeFast();
int CalcHeatDiffMergeFast();
int DiffMergedFastV();

#endif
