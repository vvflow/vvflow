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
	merge_criteria_sq = 0.09*sqr(S->AverageSegmentLength());

	auto bnodes = GetTreeBottomNodes();
	const_for(bnodes, llbnode)
	{
		TNode &bnode = **llbnode;

		auto vlist = bnode.VortexLList;
		if (vlist)
		const_for (vlist, llobj)
		{
			if (!*llobj) { continue; }
			TObj &obj = **llobj;

			obj._1_eps = 1./max(epsv(bnode, llobj, merge), eps_restriction);
		}

		auto hlist = bnode.HeatLList;
		if (hlist)
		const_for (hlist, llobj)
		{
			if (!*llobj) { continue; }
			TObj &obj = **llobj;

			obj._1_eps = 1./max(epsh(bnode, *llobj), eps_restriction);
		}
	}
}

/******************************** NAMESPACE ***********************************/

void epsfast::MergeVortexes(TObj **lv1, TObj **lv2)
{
//	if (!lv1 || !lv2 || (lv1==lv2)) return;
//	if (!*lv1 || !*lv2) return;
	TObj &v1 = **lv1;
	TObj &v2 = **lv2;
	merged_++;

	if ( sign(v1) == sign(v2) )
	{
		v1 = (v1*v1.g + v2*v2.g)/(v1.g + v2.g);
	}
	else if ( fabs(v1.g) < fabs(v2.g) ) { TVec(v1) = TVec(v2); }
	v1.v = (v1.v*v1.g + v2.v*v2.g)/(v1.g + v2.g);
	v1.g+= v2.g;
	v2.g = 0;
	*lv2 = NULL;
}

double epsfast::epsv(const TNode &Node, TObj **lv, bool merge)
{
	if (!lv || !*lv) { return DBL_MIN; }
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	TObj &v = **lv;
	TObj **lv1, **lv2;
	lv1 = lv2 = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		TNode &nnode = **llnnode;

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
	}

	if ( !lv1 ) return DBL_MIN;
	if ( !lv2 ) return sqrt(res1);
	if (!merge) return sqrt(res2);

	TObj &v1 = **lv1;
	TObj &v2 = **lv2;
	if ( 
		(res1 < merge_criteria_sq)
		||
		( (sign(v1) == sign(v2)) && (sign(v1) != sign(v)) ) 
	   )
	{
		MergeVortexes(lv, lv1);
		return epsv(Node, lv, false);
	}

	return sqrt(res2);
}

double epsfast::epsh(const TNode &Node, TObj *lv)
{
	if (!lv) { return DBL_MIN; }
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	TObj &v = *lv;

	const_for(Node.NearNodes, llnnode)
	{
		TNode &nnode = **llnnode;

		auto *hlist = nnode.HeatLList;
		if (!hlist) {continue;}
		const_for (hlist, llobj)
		{
			// if (!*llobj) { continue; } useless
			TObj &obj = **llobj;
			dr = v - obj;
			double drabs2 = dr.abs2();
			if ( drabs2 ) {
			if ( res1 > drabs2 )
			{
				res2 = res1;
				res1 = drabs2;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2;
			}}
		}
	}

	if ( res1 == DBL_MAX ) return DBL_MIN;
	if ( res2 == DBL_MAX ) return sqrt(res1);
	return sqrt(res2);
}

