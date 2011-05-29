#ifndef __CONVECTIVE_H__
#define __CONVECTIVE_H__
#include <math.h>
#include "core.h"

int InitConvective(Space *sS, double sEps);
int CalcConvective();
int CalcCirculation();
TVec SpeedSum(vector<TObj> *list, TVec p);

#endif
