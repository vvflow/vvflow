#ifndef MERGEFAST_H
#define MERGEFAST_H
#include "libVVHD/core.h"

int InitMergeFast(Space *sS, double sSqEps);
int MergeFast();
int MergedFastV();

// hint: two vortexes will be merged if dx+dy < SqEps

#endif
