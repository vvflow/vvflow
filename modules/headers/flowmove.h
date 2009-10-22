#ifndef __FLOWMOVE_H__
#define __FLOWMOVE_H__
#include <math.h>
#include "libVVHD/core.h"

int InitFlowMove(Space *sS, double sdt, double sRemoveEps/*, Statistic *sStatistic, double sBoundaryLayerHeight/*, int scountfi*/);
int MoveAndClean(bool remove = false);
int Move();
int Clean();
int VortexFlight();
int HeatFlight();
int CleanedV(); // Returns ammount of Vortex items removed after MoveAndClean() call
int CleanedH(); // Returns ammount of Heat   items removed after MoveAndClean() call

#endif
