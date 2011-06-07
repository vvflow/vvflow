#include <math.h>
#include <cstdlib>
#include "diffusivefast.h"
#define expdef(x) exp(x)

const double ResDRestriction = 1E-6;
const double ExpArgRestriction = -8.;

#include "iostream"
using namespace std;

/********************************** HEADER ************************************/

namespace {

	Space *S;
	double Re;
	double Nyu;

	enum ParticleType {Vortex, Heat};
	void VortexInfluence(const TObj &v, const TObj &vj,
	                      TVec *i2, double *i1);
	void SegmentInfluence(const TObj &v, const TObj &pk, const TObj &pk1, TBody *body,
	                      TVec *i3, double *i0);
}
/****************************** MAIN FUNCTIONS ********************************/

void InitDiffusiveFast(Space *sS, double sRe)
{
	S = sS;
	Re = sRe;
	Nyu = 1/sRe;
}

void CalcVortexDiffusiveFast()
{
	if (!S) {cerr << "CalcVortexDiffusiveFast() is called before initialization"
	              << endl; return; }
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
					TBody *body;
					const_for(S->BodyList, llbody)
						if ( (*lljobj >= (**llbody).List->begin()) &&
						     (*lljobj >= (**llbody).List->begin()) )
							body = *llbody;
					if (!body) { continue; }
					SegmentInfluence(obj, **lljobj, *body->next(*lljobj),
					           body, &S3, &S0);
				}
			}

			if ( sign(S1) != sign(obj) ) { S1 = obj.g; }

			double multiplier = obj._1_eps/(Re*S1);
			double S2abs = S2.abs2();
			if (S2abs > 100) { multiplier*=10/sqrt(S2abs); }
			obj.v += multiplier * S2;
			obj.v += (sqr(obj._1_eps)/(Re*(C_2PI-S0))) * S3;
		}
	}
}

/******************************** NAMESPACE ***********************************/

namespace {
inline
void VortexInfluence(const TObj &v, const TObj &vj, TVec *i2, double *i1)
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
}}

namespace {
inline
void SegmentInfluence(const TObj &v, const TObj &pk, const TObj &pk1, TBody *body,
                      TVec *i3, double *i0)
{
	TVec rk = (pk+pk1)*0.5;

	TVec dr = v - rk;
	double drabs2 = dr.abs2();
	double drabs = sqrt( drabs2 );
	double exparg = -drabs*v._1_eps;
	if ( exparg < ExpArgRestriction ) {return;}
	double expres = expdef(exparg);
	TVec dS = rotl(pk1 - pk);
	*i3 += dS * expres;
	*i0 += (drabs*v._1_eps+1)/drabs2*(dr*dS)*expres;

	exparg = -drabs*v._1_eps;
	if ( exparg < ExpArgRestriction ) {return;}
	body->att(&pk)->fric += sqr(v._1_eps) * v.g * expdef(exparg);
}}


