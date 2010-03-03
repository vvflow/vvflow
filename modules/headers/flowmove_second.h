#ifndef __FLOWMOVE_SECOND_H__
#define __FLOWMOVE_SECOND_H__
#include <math.h>
#include "libVVHD/core.h"

int InitFlowMove_Second(Space *sS, double sdt, double sRemoveEps);
int Move1();
int Move2();
int Clean();
int VortexShed();
int HeatShed();
int CleanedV_inbody(); // Returns ammount of Vortex items removed after MoveAndClean() call
int CleanedV_toosmall();
int CleanedH(); // Returns ammount of Heat   items removed after MoveAndClean() call

#endif
