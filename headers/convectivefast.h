#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "core.h"

void InitConvectiveFast(Space *sS, double sRd2);
void CalcConvectiveFast();
void CalcBoundaryConvective();
TVec SpeedSumFast(TVec p);

void CalcCirculationFast();
void FillMatrix();
void FillRightCol();
void SolveMatrix();

double* MatrixLink();

int LoadBodyMatrix(const char* filename);
int LoadInverseMatrix(const char* filename);
void SaveBodyMatrix(const char* filename);
void SaveInverseMatrix(const char* filename);

int LoadBodyMatrix_bin(const char* filename);
int LoadInverseMatrix_bin(const char* filename);
void SaveBodyMatrix_bin(const char* filename);
void SaveInverseMatrix_bin(const char* filename);

#endif
