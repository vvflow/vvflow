#ifndef _DIFFUSIVEFAST_H_
#define _DIFFUSIVEFAST_H_
#include "core.h"
#include "body.h"

class diffusivefast
{
	public:
		diffusivefast(Space *sS);
		void CalcVortexDiffusiveFast();
		void CalcHeatDiffusiveFast();

	private:
		Space *S;
		double Re;
		double Pr;
		enum ParticleType {Vortex, Heat};
		void VortexInfluence(const TObj &v, const TObj &vj, TVec *i2, double *i1);
		void SegmentInfluence(const TObj &v, TAtt *pk, TVec *i3, double *i0, bool calc_friction);
};

#endif
