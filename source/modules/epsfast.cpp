#include "epsfast.h"
#include <math.h>
#include <float.h>
#include <iostream>

using namespace std;

static vector<TObj> *vList;
static vector<TObj> *hList;

epsfast::epsfast(Space *sS)
{
	S = sS;
	vList = S->VortexList;
	hList = S->HeatList;
	merged_ = 0;
}


void epsfast::CalcEpsilonFast(bool merge)
{
	merged_ = 0;

	auto bnodes = S->Tree->getBottomNodes();
	const_for(bnodes, llbnode)
	{
		#define bnode (**llbnode)
		TAtt *nearestAtt = nearestBodySegment(TVec(bnode.x, bnode.y));

		double merge_criteria_sq = (merge && nearestAtt) ? 
		                             0.16 * nearestAtt->dl.abs2() * (1 + (TVec(bnode.x, bnode.y) - *nearestAtt).abs2())
		                             : 0;
		double _1_eps_restriction = 3.0/nearestAtt->dl.abs();

		for (TObj *lobj = bnode.vRange.first; lobj < bnode.vRange.last; lobj++)
		{
			if (!lobj->g) continue;
			lobj->_1_eps = min(1./epsv(bnode, lobj, merge_criteria_sq), _1_eps_restriction);
			//eps is bounded below (cant be less than restriction)
		}

		for (TObj *lobj = bnode.hRange.first; lobj < bnode.hRange.last; lobj++)
		{
			if (!lobj->g) continue;
			lobj->_1_eps = min(1./epsh(bnode, lobj, merge_criteria_sq), _1_eps_restriction);
			//eps is bounded below (cant be less than restriction)
		}
		#undef bnode
	}
}

/******************************** NAMESPACE ***********************************/

void epsfast::MergeVortexes(TObj *lv1, TObj *lv2)
{
	merged_++;

	if ( sign(*lv1) == sign(*lv2) )
	{
		*lv1 = (*lv1*lv1->g + *lv2*lv2->g)/(lv1->g + lv2->g);
	}
	else if ( fabs(lv1->g) < fabs(lv2->g) )
	{
		lv1->rx = lv2->rx;
		lv1->ry = lv2->ry;
	}
	//lv1->v = (lv1->v*lv1->g + lv2->v*lv2->g)/(lv1->g + lv2->g);
	lv1->g+= lv2->g;
	lv2->g = 0;
}

double epsfast::epsv(const TSortedNode &Node, TObj *lv, double merge_criteria_sq)
{
	double res1, res2;
	res2 = res1 = DBL_MAX;

	TObj *lv1, *lv2;
	lv1 = lv2 = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			if (!lobj->g || (lv == lobj)) { continue; }
			double drabs2 = (*lv - *lobj).abs2();

			if ( res1 > drabs2 )
			{
				res2 = res1; lv2 = lv1;
				res1 = drabs2; lv1 = lobj;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2; lv2 = lobj;
			}
		}
		#undef nnode
	}

	if ( !lv1 ) return DBL_MIN;
	if ( !lv2 ) return sqrt(res1);

	if ( 
		(res1 < merge_criteria_sq)
		||
		( (lv1->sign() == lv2->sign()) && (lv1->sign() != lv->sign()) ) 
	   )
	{
		MergeVortexes(lv, lv1);
		return epsv(Node, lv, 0);
	}

	return sqrt(res2);
}

double epsfast::epsh(const TSortedNode &Node, TObj *lv, double merge_criteria_sq)
{
	double res1, res2;
	res2 = res1 = DBL_MAX;

	TObj *lv1 = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			if (!lobj->g || (lv == lobj)) { continue; }
			double drabs2 = (*lv - *lobj).abs2();

			if ( res1 > drabs2 )
			{
				res2 = res1;
				res1 = drabs2;
				lv1 = lobj;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2;
			}
		}
		#undef nnode
	}

	if ( res1 == DBL_MAX ) return DBL_MIN;
	if ( res2 == DBL_MAX ) return sqrt(res1);

	if (res1 < merge_criteria_sq)
	{
		MergeVortexes(lv, lv1);
		return epsh(Node, lv, 0);
	}

	return sqrt(res2);
}

