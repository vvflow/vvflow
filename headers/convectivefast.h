#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "core.h"
#include "matrix.h"

class convectivefast
{
	public:
		convectivefast(Space *sS, double sRadiusOfDiscretness);
		void CalcConvectiveFast();
		void CalcBoundaryConvective();
		TVec SpeedSumFast(TVec p);

	public:
		void CalcCirculationFast(bool use_inverse);
		Matrix* getMatrix() {return matrix;}
		//void fillSlae();
		//void solveSlae();

	private:
		void FillMatrix(bool rightColOnly = false);

	private:
		TVec BioSavar(const TObj &obj, const TVec &p);
		TVec SpeedSum(const TNode &Node, const TVec &p);

		TVec BoundaryConvective(const TBody &b, const TVec &p);

	private:
		Space *S;
		double Rd2;
		double Rd;

		int MatrixSize;
		Matrix *matrix;

		double _2PI_Xi_g(TVec p, const TAtt &seg, double rd); // in doc 2\pi\Xi_\gamma (1.7)
		double _2PI_Xi_q(TVec &p, const TAtt &seg, double rd); // in doc 2\pi\Xi_q (1.8)
		void _2PI_A123(const TAtt &seg, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3);
		double ConvectiveInfluence(TVec p, const TAtt &seg, double rd);
		double NodeInfluence(const TNode &Node, const TAtt &seg, double rd);
		double AttachInfluence(const TAtt &seg, double rd);
		TVec SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2);

	private:
		void fillSlipEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillZeroEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillSteadyEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillInfSteadyEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillForceXEquation(TBody* ibody, bool rightColOnly);
		void fillForceYEquation(TBody* ibody, bool rightColOnly);
		void fillMomentEquation(TBody* ibody, bool rightColOnly);
};



#endif
