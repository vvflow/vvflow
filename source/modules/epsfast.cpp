#include "epsfast.h"
#include <math.h>
#include <float.h>
#include <iostream>

using namespace std;

/********************************** HEADER ************************************/

namespace
{
	Space *S;
	double sq_eps;
	double eps_restriction;
	int merged_count;
	int merge_list(vector<TObj> *list);

	void MergeVortexes(TObj **lv1, TObj **lv2);
	double eps(const TNode &Node, TObj **lv, bool merge);
}

/****************************** MAIN FUNCTIONS ********************************/

void InitEpsilonFast(Space *sS, double sSqEps)
{
	S = sS;
	sq_eps = sSqEps;
	merged_count = 0;
}

void CalcEpsilonFast(bool merge)
{
	merged_count = 0;
	eps_restriction = (S->Body) ? 0.6*S->Body->AverageSegmentLength() : 0;
	if (!S) {cerr << "MergeFast() is called before initialization" << endl; return; }
	auto bnodes = GetTreeBottomNodes();
	if ( !bnodes ) return;

	const_for(bnodes, llbnode)
	{
		TNode &bnode = **llbnode;

		auto vlist = bnode.VortexLList;
		if ( !vlist ) { continue; }
		const_for (vlist, llobj)
		{
			if (!*llobj) { continue; }
			TObj &obj = **llobj;

			double eps_tmp = eps(bnode, llobj, merge);
			eps_tmp = (eps_tmp > eps_restriction) ? eps_tmp : eps_restriction;
			obj._1_eps = 1./eps_tmp;
		}
	}
}

int Merged_count()
{
	return merged_count;
}

/******************************** NAMESPACE ***********************************/

namespace {
void MergeVortexes(TObj **lv1, TObj **lv2)
{
//	if (!lv1 || !lv2 || (lv1==lv2)) return;
//	if (!*lv1 || !*lv2) return;
	TObj &v1 = **lv1;
	TObj &v2 = **lv2;
	merged_count++;

	if ( sign(v1) == sign(v2) )
	{
		v1 = (v1*v1.g + v2*v2.g)/(v1.g + v2.g);
	}
	else if ( fabs(v1.g) < fabs(v2.g) ) { TVec(v1) = TVec(v2); }
	v1.v = (v1.v*v1.g + v2.v*v2.g)/(v1.g + v2.g);
	v1.g+= v2.g;
	v2.g = 0;
	*lv2 = NULL;
}}

namespace {
double eps(const TNode &Node, TObj **lv, bool merge)
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
			if ( drabs2 )
			if ( res1 > drabs2 )
			{
				res2 = res1; lv2 = lv1;
				res1 = drabs2; lv1 = llobj;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2; lv2 = llobj;
			}
		}
	}

	if ( !lv1 ) return DBL_MIN;
	if ( !lv2 ) return sqrt(res1);
	if (!merge) return sqrt(res2);

	TObj &v1 = **lv1;
	TObj &v2 = **lv2;
	if ( 
		(res1 < sq_eps)
		||
		( (sign(v1) == sign(v2)) && (sign(v1) != sign(v)) ) 
	   )
	{
		MergeVortexes(lv, lv1);
		return eps(Node, lv, false);
	}

	return sqrt(res2);
}}
