#ifndef __CONVECTIVEFAST_H__
#define __CONVECTIVEFAST_H__
#include <math.h>
#include "core.h"
#include "matrix.h"

class convectivefast
{
	public:
		convectivefast(Space *sS);
		void CalcConvectiveFast();
		void CalcBoundaryConvective();
		TVec SpeedSumFast(TVec p);

	public:
		void CalcCirculationFast();
		Matrix* getMatrix() {return matrix;}
		//void fillSlae();
		//void solveSlae();

	private:
		bool canUseInverse();
		void FillMatrix(bool rightColOnly = false);

	private:
		TVec BioSavar(const TObj &obj, const TVec &p);
		TVec SpeedSum(const TSortedNode &Node, const TVec &p);

		TVec BoundaryConvective(const TBody &b, const TVec &p);
		TVec BoundaryConvectiveSlip(const TBody &b, const TVec &p);

	private:
		Space *S;

		int MatrixSize;
		Matrix *matrix;

		double _2PI_Xi_g(TVec p, const TAtt &seg, double rd); // in doc 2\pi\Xi_\gamma (1.7)
		double _2PI_Xi_q(TVec &p, const TAtt &seg, double rd); // in doc 2\pi\Xi_q (1.8)
		void _2PI_A123(const TAtt &seg, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3);
		double ConvectiveInfluence(TVec p, const TAtt &seg, double rd);
		double NodeInfluence(const TSortedNode &Node, const TAtt &seg);
		double AttachInfluence(const TAtt &seg, double rd);
		TVec SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2);

	private:
		void fillSlipEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillZeroEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillSteadyEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillInfSteadyEquationForSegment(TAtt* seg, bool rightColOnly);
		void fillForceXEquation(TBody* ibody, bool rightColOnly);
		void fillForceYEquation(TBody* ibody, bool rightColOnly);
		void fillForceOEquation(TBody* ibody, bool rightColOnly);
		void fillSpeedXEquation(TBody* ibody, bool rightColOnly);
		void fillSpeedYEquation(TBody* ibody, bool rightColOnly);
		void fillSpeedOEquation(TBody* ibody, bool rightColOnly);
};



#endif
