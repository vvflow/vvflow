#ifndef __FLOWMOVE_H__
#define __FLOWMOVE_H__
#include <math.h>
#include "core.h"

void InitFlowMove(Space *sS, double sdt, double sRemoveEps);
void MoveAndClean(bool remove = false);
void VortexShed();
void HeatShed();
int CleanedV(); // Returns ammount of Vortex items removed after MoveAndClean() call
int CleanedH(); // Returns ammount of Heat   items removed after MoveAndClean() call

#endif
