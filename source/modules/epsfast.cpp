#include "epsfast.h"
#include <math.h>
#include <float.h>
#include <iostream>

using namespace std;

epsfast::epsfast(Space *sS)
{
	S = sS;
	merged_ = 0;
}


void epsfast::CalcEpsilonFast(bool merge)
{
	merged_ = 0;
	eps_restriction = 0.6*S->AverageSegmentLength();
	double tmp_merge_criteria_sq = 0.16*sqr(S->AverageSegmentLength());

	auto bnodes = GetTreeBottomNodes();
	const_for(bnodes, llbnode)
	{
		#define bnode (**llbnode)

		double h2=DBL_MAX; //distance to body surface 
		const_for(S->BodyList, llbody)
		const_for((**llbody).List, lobj)
		{
			h2 = min(h2, (*lobj-TVec(bnode.x, bnode.y)).abs2());
			lobj+= 10; //speed up
		}
		merge_criteria_sq = (h2==DBL_MAX)?0:(h2+1) * tmp_merge_criteria_sq;

		auto vlist = bnode.VortexLList;
		if (vlist)
		const_for (vlist, llobj)
		{
			if (!*llobj) { continue; }
			#define obj (**llobj)
			obj._1_eps = 1./max(epsv(bnode, llobj, merge), eps_restriction);
			#undef obj
		}

		auto hlist = bnode.HeatLList;
		if (hlist)
		const_for (hlist, llobj)
		{
			if (!*llobj) { continue; }
			#define obj (**llobj)
			obj._1_eps = 1./max(epsh(bnode, llobj, merge), eps_restriction);
			#undef obj
		}
		#undef bnode
	}
}

/******************************** NAMESPACE ***********************************/

void epsfast::MergeVortexes(TObj **lv1, TObj **lv2)
{
//	if (!lv1 || !lv2 || (lv1==lv2)) return;
//	if (!*lv1 || !*lv2) return;
	#define v1 (**lv1)
	#define v2 (**lv2)
	merged_++;

	if ( sign(v1) == sign(v2) )
	{
		v1 = (v1*v1.g + v2*v2.g)/(v1.g + v2.g);
	}
	else if ( fabs(v1.g) < fabs(v2.g) )
	{
		v1.rx = v2.rx;
		v1.ry = v2.ry;
	}
	v1.v = (v1.v*v1.g + v2.v*v2.g)/(v1.g + v2.g);
	v1.g+= v2.g;
	v2.g = 0;
	*lv2 = NULL;
	#undef v1
	#undef v2
}

double epsfast::epsv(const TNode &Node, TObj **lv, bool merge)
{
	if (!lv || !*lv) { return DBL_MIN; }
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	#define v (**lv)
	TObj **lv1, **lv2;
	lv1 = lv2 = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)

		auto *vlist = nnode.VortexLList;
		if (!vlist) {continue;}
		const_for (vlist, llobj)
		{
			if (!*llobj) { continue; }
			TObj &obj = **llobj;
			dr = v - obj;
			double drabs2 = dr.abs2();
			if ( drabs2 ) {
			if ( res1 > drabs2 )
			{
				res2 = res1; lv2 = lv1;
				res1 = drabs2; lv1 = llobj;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2; lv2 = llobj;
			}}
		}
		#undef nnode
	}

	if ( !lv1 ) return DBL_MIN;
	if ( !lv2 ) return sqrt(res1);
	if (!merge) return sqrt(res2);

	#define v1 (**lv1)
	#define v2 (**lv2)
	if ( 
		(res1 < merge_criteria_sq)
		||
		( (sign(v1) == sign(v2)) && (sign(v1) != sign(v)) ) 
	   )
	{
		MergeVortexes(lv, lv1);
		return epsv(Node, lv, false);
	}
	#undef v1
	#undef v2
	#undef v

	return sqrt(res2);
}

double epsfast::epsh(const TNode &Node, TObj **lv, bool merge)
{
	if (!lv || !*lv) { return DBL_MIN; }
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	#define v (**lv)
	TObj **lv1 = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)

		auto *hlist = nnode.HeatLList;
		if (!hlist) {continue;}
		const_for (hlist, llobj)
		{
			if (!*llobj) { continue; }
			#define obj (**llobj)
			dr = v - obj;
			double drabs2 = dr.abs2();
			if ( drabs2 ) {
			if ( res1 > drabs2 )
			{
				res2 = res1;
				res1 = drabs2;
				lv1 = llobj;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2;
			}}
			#undef obj
		}
		#undef nnode
	}

	if ( res1 == DBL_MAX ) return DBL_MIN;
	if ( res2 == DBL_MAX ) return sqrt(res1);
	if (!merge) return sqrt(res2);

	#define v1 (**lv1)
	if (res1 < merge_criteria_sq)
	{
		MergeVortexes(lv, lv1);
		return epsh(Node, lv, false);
	}
	#undef v
	#undef v1

	return sqrt(res2);
}

