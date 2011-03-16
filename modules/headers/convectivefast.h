#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "libVVHD/core.h"

int InitConvectiveFast(Space *sS, double sEps);
int CalcConvectiveFast();

int CalcCirculationFast();
int FillMatrix();
int FillRightCol();
int SolveMatrix();

//int SpeedSum(double px, double py, double &resx, double &resy);
int LoadBodyMatrix(const char* filename);
int LoadInverseMatrix(const char* filename);

#endif
