#include <math.h>
#include <cstdlib>
#include "diffusivefast.h"
#define expdef(x) exp(x)

const double ExpArgRestriction = -8.;

#include "iostream"
using namespace std;

diffusivefast::diffusivefast(Space *sS)
{
	S = sS;
	Re = S->Re;
	Pr = S->Pr;
}

void diffusivefast::CalcVortexDiffusiveFast()
{
	if ( !S->VortexList ) return;

	auto bnodes = S->Tree->getBottomNodes();
	if ( !bnodes ) return;

	#pragma omp parallel for schedule(dynamic, 10)
	const_for(bnodes, llbnode)
	{
		#define bnode (**llbnode)
		for (TObj *lobj = bnode.vRange.first; lobj < bnode.vRange.last; lobj++)
		{
			if (!lobj->g) { continue; }

			TVec S2(0,0), S3(0,0);
			double S1 = 0, S0 = 0;

			auto nnodes = bnode.NearNodes;
			const_for(nnodes, llnnode)
			{
				#define nnode (**llnnode)
				for (TObj *ljobj = nnode.vRange.first; ljobj < nnode.vRange.last; ljobj++)
				{
					if (!ljobj->g) { continue; }
					VortexInfluence(*lobj, *ljobj, &S2, &S1);
				}

				if ( nnode.BodyLList )
				const_for(nnode.BodyLList, lljatt)
				{
					if (!*lljatt) { fprintf(stderr, "diffusivefast.cpp:49 lljatt = NULL. Is it possible?\n"); continue; }
					SegmentInfluence(*lobj, (TAtt*)(*lljatt), &S3, &S0, true);
				}
				#undef nnode
			}

			// trick or treat? ok, trick.
			if ( (sign(S1)!=lobj->sign()) || (fabs(S1)<fabs(0.1*lobj->g)) ) { S1 = 0.1*lobj->g; }
			if ( S0 > C_PI ) S0 = C_PI;
			if ( S0 < 0.   ) S0 = 0.;

			lobj->v += lobj->_1_eps/(Re*S1) * S2;
			lobj->v += (sqr(lobj->_1_eps)/(Re*(C_2PI-S0))) * S3;
		}
		#undef bnode
	}
}

void diffusivefast::CalcHeatDiffusiveFast()
{
	if ( !S->HeatList ) return;

	auto bnodes = S->Tree->getBottomNodes();
	if ( !bnodes ) return;

	#pragma omp parallel for schedule(dynamic, 10)
	const_for(bnodes, llbnode)
	{
		#define bnode (**llbnode)

		for (TObj *lobj = bnode.hRange.first; lobj < bnode.hRange.last; lobj++)
		{
			if (!lobj->g) { continue; }

			TVec S2(0,0), S3(0,0);
			double S1 = 0, S0 = 0;

			auto nnodes = bnode.NearNodes;
			const_for(nnodes, llnnode)
			{
				#define nnode (**llnnode)
				for (TObj *ljobj = nnode.hRange.first; ljobj < nnode.hRange.last; ljobj++)
				{
					if (!ljobj->g) { continue; }
					VortexInfluence(*lobj, *ljobj, &S2, &S1);
				}

				if ( nnode.BodyLList )
				const_for(nnode.BodyLList, lljatt)
				{
					if (!*lljatt) { fprintf(stderr, "diffusivefast.cpp:97 lljatt = NULL. Is it possible?\n"); continue; }
					SegmentInfluence(*lobj, (TAtt*)(*lljatt), &S3, &S0, false);
				}
				#undef nnode
			}

			if ( (sign(S1)!=lobj->sign()) || (fabs(S1)<fabs(0.1*lobj->g)) ) { S1 = 0.1*lobj->g; }

			lobj->v += (lobj->_1_eps*S2+S3)/(S1+S0/sqr(lobj->_1_eps))/(Pr*Re);
//			obj.v += obj._1_eps/(Re*S1) * S2;
//			obj.v += (sqr(obj._1_eps)/(Re*(C_2PI-S0))) * S3;
		}
		#undef bnode
	}
}

/******************************** NAMESPACE ***********************************/

inline
void diffusivefast::VortexInfluence(const TObj &v, const TObj &vj, TVec *i2, double *i1)
{
	if ( sign(v) != sign(vj) ) { return; }
	TVec dr = v - vj;
	if ( dr.iszero() ) { return; }
	double drabs = dr.abs();
	double exparg = -drabs*v._1_eps;
	if ( exparg < ExpArgRestriction ) return;
	double i1tmp = vj.g * expdef(exparg);
	*i2 += dr * (i1tmp/drabs);
	*i1 += i1tmp;
}

inline
void diffusivefast::SegmentInfluence(const TObj &v, TAtt *rk,
                                     TVec *i3, double *i0, bool calc_friction)
{
	TVec dr = v - *rk;
	double drabs2 = dr.abs2();

	if (drabs2 < rk->dl.abs2()*0.05) { *i0 = C_PI; } //exceed the limit and later (at line 57) crop it

	double drabs = sqrt( drabs2 );
	double exparg = -drabs*v._1_eps;
	if ( exparg < ExpArgRestriction ) {return;}
	double expres = expdef(exparg);
	TVec dS = rotl(rk->dl);
	*i3 += dS * expres;
	*i0 += (drabs*v._1_eps+1.)/drabs2*(dr*dS)*expres;

	if (calc_friction)
		rk->fric += sqr(v._1_eps) * v.g * expres * dS.abs();
}


