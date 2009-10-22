#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "libVVHD/core.h"

int InitConvectiveFast(Space *sS, double sEps);
int CalcConvectiveFast();
int CalcCirculationFast();
//int SpeedSum(TList *List, double px, double py, double &resx, double &resy);

#endif
