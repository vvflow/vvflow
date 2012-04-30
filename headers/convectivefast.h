#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "core.h"

class convectivefast
{
	public:
		convectivefast(Space *sS, double sRadiusOfDiscretnessSq);
		void CalcConvectiveFast();
		void CalcBoundaryConvective();
		TVec SpeedSumFast(TVec p);

	public:
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

	private:
		TVec BioSavar(const TObj &obj, const TVec &p);
		TVec SpeedSum(const TNode &Node, const TVec &p);

		TVec BoundaryConvective(const TBody &b, const TVec &p);

	private:
		Space *S;
		double Rd2;
		double Rd;

		int MatrixSize;
		double *BodyMatrix;
		double *InverseMatrix;
		double *RightCol;
		double *Solution;

		int *ipvt; //technical variable for lapack
		void inverse(double* A, int N, double *workspace); //workspace is being spoiled;
		void transpose(double* A, int N);
		void SolveMatrix_inv();

		bool BodyMatrixOK;
		bool InverseMatrixOK;

		double ConvectiveInfluence(TVec p, const TAtt &seg, double rd);
		double NodeInfluence(const TNode &Node, const TAtt &seg, double rd);
		double AttachInfluence(const TAtt &seg, double rd);

		size_t LoadMatrix(double *matrix, const char* filename);
		void SaveMatrix(double *matrix, const char* filename);
		size_t LoadMatrix_bin(double *matrix, const char* filename);
		void SaveMatrix_bin(double *matrix, const char* filename); 
};



#endif
