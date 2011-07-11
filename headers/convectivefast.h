#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "core.h"

void InitConvectiveFast(Space *sS, double sRd2);
void CalcConvectiveFast();
void CalcBoundaryConvective();
TVec SpeedSumFast(TVec p);

void CalcCirculationFast(bool use_inverse);
void FillMatrix();
void FillInverseMatrix();
void FillRightCol();
void SolveMatrix();
void SpoilBodyMatrix();
void SpoilInverseMatrix();

double* MatrixLink();
double* InvMatrixLink();

bool LoadBodyMatrix(const char* filename);
bool LoadInverseMatrix(const char* filename);
void SaveBodyMatrix(const char* filename);
void SaveInverseMatrix(const char* filename);

bool LoadBodyMatrix_bin(const char* filename);
bool LoadInverseMatrix_bin(const char* filename);
void SaveBodyMatrix_bin(const char* filename);
void SaveInverseMatrix_bin(const char* filename);

#endif
