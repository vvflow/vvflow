#include <math.h>
#include <cstdlib>
#include "diffusivefast.h"
#define expdef(x) exp(x)

const double ExpArgRestriction = -8.;

#include "iostream"
using namespace std;

diffusivefast::diffusivefast(Space *sS, double sRe, double sPr)
{
	S = sS;
	S->Re = Re = sRe;
	S->Pr = Pr = sPr;
}

void diffusivefast::CalcVortexDiffusiveFast()
{
	if ( !S->VortexList ) return;

	auto bnodes = GetTreeBottomNodes();
	if ( !bnodes ) return;

	#pragma omp parallel for schedule(dynamic, 10)
	const_for(bnodes, llbnode)
	{
		TNode &bnode = **llbnode;

		auto vlist = bnode.VortexLList;
		if ( !vlist ) { continue; }
		const_for (vlist, llobj)
		{
			if (!*llobj) { continue; }
			TObj &obj = **llobj;

			TVec S2(0,0), S3(0,0);
			double S1 = 0, S0 = 0;

			auto nnodes = bnode.NearNodes;
			const_for(nnodes, llnnode)
			{
				TNode &nnode = **llnnode;
				auto jlist = nnode.VortexLList;
				if ( jlist )
				const_for (jlist, lljobj)
				{
					if (!*lljobj) { continue; }
					VortexInfluence(obj, **lljobj, &S2, &S1);
				}

				jlist = nnode.BodyLList;
				if ( jlist )
				const_for(jlist, lljobj)
				{
					if (!*lljobj) { continue; }
					TBody *body = NULL;
					const_for(S->BodyList, llbody)
						if ( (*lljobj >= (**llbody).List->begin()) &&
						     (*lljobj <  (**llbody).List->end()) )
							body = *llbody;
					if (!body) { continue; }
					SegmentInfluence(obj, body->att(*lljobj), &S3, &S0);
				}
			}

			if ( (sign(S1)!=sign(obj)) || (fabs(S1)<fabs(0.1*obj.g)) ) { S1 = 0.1*obj.g; }

			obj.v += obj._1_eps/(Re*S1) * S2;
			obj.v += (sqr(obj._1_eps)/(Re*(C_2PI-S0))) * S3;
		}
	}
}

void diffusivefast::CalcHeatDiffusiveFast()
{
	if ( !S->HeatList ) return;

	auto bnodes = GetTreeBottomNodes();
	if ( !bnodes ) return;

	#pragma omp parallel for schedule(dynamic, 10)
	const_for(bnodes, llbnode)
	{
		TNode &bnode = **llbnode;

		auto hlist = bnode.HeatLList;
		if ( !hlist ) { continue; }
		const_for (hlist, llobj)
		{
			if (!*llobj) { continue; }
			TObj &obj = **llobj;

			TVec S2(0,0), S3(0,0);
			double S1 = 0, S0 = 0;

			auto nnodes = bnode.NearNodes;
			const_for(nnodes, llnnode)
			{
				TNode &nnode = **llnnode;
				auto jlist = nnode.HeatLList;
				if ( jlist )
				const_for (jlist, lljobj)
				{
					if (!*lljobj) { continue; }
					VortexInfluence(obj, **lljobj, &S2, &S1);
				}

				jlist = nnode.BodyLList;
				if ( jlist )
				const_for(jlist, lljobj)
				{
					if (!*lljobj) { continue; }
					TBody *body = NULL;
					const_for(S->BodyList, llbody)
						if ( (*lljobj >= (**llbody).List->begin()) &&
						     (*lljobj <  (**llbody).List->end()) )
							body = *llbody;
					if (!body) { continue; }
					SegmentInfluence(obj, body->att(*lljobj), &S3, &S0);
				}
			}

			if ( (sign(S1)!=sign(obj)) || (fabs(S1)<fabs(0.1*obj.g)) ) { S1 = 0.1*obj.g; }

//			cerr << obj._1_eps << " " << S2 << " " << S3 << ' ' << S1 << ' ' << S0 << endl;
			obj.v += (obj._1_eps*S2+S3)/(S1+S0/sqr(obj._1_eps))/(Pr*Re);
//			obj.v += obj._1_eps/(Re*S1) * S2;
//			obj.v += (sqr(obj._1_eps)/(Re*(C_2PI-S0))) * S3;
		}
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
                                     TVec *i3, double *i0)
{
	TVec dr = v - *rk;
	double drabs2 = dr.abs2();
	double drabs = sqrt( drabs2 );
	double exparg = -drabs*v._1_eps;
	if ( exparg < ExpArgRestriction ) {return;}
	double expres = expdef(exparg);
	TVec dS = rotl(rk->dl);
	*i3 += dS * expres;
	*i0 += (drabs*v._1_eps+1)/drabs2*(dr*dS)*expres;

	rk->fric += sqr(v._1_eps) * v.g * expres * dS.abs();
}


