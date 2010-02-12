#ifndef _DIFFMERGE_H_
#define _DIFFMERGE_H_
#include <math.h>
#include "libVVHD/core.h"

int InitDiffMerge(Space *sS, double sRe, double sMergeSqEps);
int CalcVortexDiffMerge();
int CalcHeatDiffMerge();
int DiffMergedV();

#endif

