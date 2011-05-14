#include <math.h>
#include <cstdlib>
#include "diffmergefast.h"
#define expdef(x) exp(x)
#define sqrtdef(x) sqrt(x)

const double S1Restriction = 1E-6;
const double ExpArgRestriction = -8.;

#include <iostream>
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffMergeFast_S;
double DiffMergeFast_Re;
double DiffMergeFast_Nyu;
//double DiffMergeFast_dfi;

double EpsRestriction;
double GRestriction;
double DiffMergeFast_MergeSqEps;
int DiffMergeFast_MergedV;

void MergeVortexes(TObj **lv1, TObj **lv2);

enum ParticleType {Vortex, Heat};
}

/********************* SOURCE *****************************/

int InitDiffMergeFast(Space *sS, double sRe, double sMergeSqEps)
{
	DiffMergeFast_S = sS;
	DiffMergeFast_Re = sRe;
	DiffMergeFast_Nyu = 1/sRe;
	DiffMergeFast_MergeSqEps = sMergeSqEps;
	EpsRestriction = (sS->Body) ? 0.6*sS->Body->AverageSegmentLength() : 0;
	//DiffMergeFast_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;
	return 0;
}

namespace {
void MergeVortexes(TObj **lv1, TObj **lv2)
{
	if (!lv1 || !lv2 || (lv1==lv2)) return;
	if (!*lv1 || !*lv2) return;
	TObj &v1 = **lv1;
	TObj &v2 = **lv2;
	//if (fabs(v1.g + v2.g) > GRestriction) return -1;
	DiffMergeFast_MergedV++;

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

int DiffMergedFastV()
{
	return DiffMergeFast_MergedV;
}

namespace {
template <ParticleType pt>
double Epsilon(const TNode &Node, TObj **lv, bool merge)
{
	if (!lv || !*lv) { return 1E-20; }
	TVec dr;
	double res1, res2;
	res2 = res1 = 1E10;

	TObj &v = **lv;
	TObj **lv1, **lv2;
	lv1 = lv2 = NULL;

	auto nnodes = Node.NearNodes;
	auto lNNode = nnodes->begin();
	auto lNNodeEnd = nnodes->end();
	for (; lNNode<lNNodeEnd; lNNode++ )
	{
		TNode &NNode = **lNNode;

		vector<TObj*> *list;
		list = (pt==Vortex) ? NNode.VortexLList : NNode.HeatLList;
		if (!list) {continue;}
		auto lObj = list->begin();
		auto lObjEnd = list->end();
		for (; lObj<lObjEnd; lObj++ )
		{
			if (!*lObj) { continue; }
			TObj &Obj = **lObj;
			dr = v - Obj;
			double drabs2 = dr.abs2();
			if (!drabs2) continue;
			if ( res1 > drabs2 ) { res2 = res1; lv2 = lv1; res1 = drabs2; lv1 = &*lObj;} 
			else if ( res2 > drabs2 ) { res2 = drabs2; lv2 = &*lObj; }
		}
	}

	if ( !lv || !lv1 ) { return 1E-20; }
	if ( !lv2 ) { return sqrtdef(res1); }

	if ( (pt == Vortex) && merge)
	{
		TObj &v1 = **lv1;
		TObj &v2 = **lv2;
		if ( 
			((*lv<*lv1) && (res1 < DiffMergeFast_MergeSqEps))
			||
			( (sign(v1) == sign(v2)) && (sign(v1) != sign(v)) ) 
			)
		{
			MergeVortexes(lv, lv1);
			//FIXME Maybe it's better to remember res3 instead of using recursive call
			return Epsilon<pt>(Node, lv, false);
		}
	}

	return sqrt(res2);
}}

namespace {
template <ParticleType pt>
void Division(const TNode &Node, const TObj &v, double _1_eps, TVec* ResP, double *ResD )
{
	TVec dr;

	ResP->zero(); *ResD = 0.;

	TNode **lNNode = Node.NearNodes->begin();
	TNode **&LastNNode = Node.NearNodes->end();
	for ( ; lNNode<LastNNode; lNNode++ )
	{
		TNode &NNode = **lNNode;

		TObj **lObj;
		TObj ***lLastObj;
		switch (pt)
		{
			case Vortex:
				if ( !NNode.VortexLList ) { continue; }
				lObj = NNode.VortexLList->begin();
				lLastObj = &NNode.VortexLList->end();
				break;
			case Heat:
				if ( !NNode.HeatLList ) { continue; }
				lObj = NNode.HeatLList->begin();
				lLastObj = &NNode.HeatLList->end();
				break;
		}
		TObj **&LastObj = *lLastObj;

		for ( ; lObj<LastObj; lObj++ )
		{
			if (!*lObj) { continue; }
			TObj &Obj = **lObj;
			dr = v - Obj;
			if ( dr.iszero() ) { continue; }
			double drabs = dr.abs();

			double exparg = -drabs*_1_eps; //FIXME is it useful?
			if ( exparg > ExpArgRestriction )
			{
				double tmp = Obj.g * expdef(exparg); // look for define
				*ResP += dr * (tmp/drabs);
				*ResD += tmp;
			}
		}
	}

	if (pt == Vortex)
	if ( ((*ResD>0)?1:-1) != sign(v) ) { *ResD = v.g; }
}}

namespace {
//inline
void VortexInfluence(const TObj &v, const TObj &vj, double _1_eps, TVec *i2, double *i1)
{
	if ( sign(v) != sign(vj) ) { return; }
	TVec dr = v - vj;
	if ( dr.iszero() ) { return; }
	double drabs = dr.abs();
	double exparg = -drabs*_1_eps;
	if ( exparg < ExpArgRestriction ) return;
	double i1tmp = vj.g * expdef(exparg);
	*i2 += dr * (i1tmp/drabs);
	*i1 += i1tmp;
}}

namespace {
//inline
void SegmentInfluence(const TObj &v, const TObj &pk, const TObj &pk1, 
					double _1_eps, TVec *i3, double *i0)
{
	TVec rk = (pk+pk1)*0.5;

	TVec dr = v - rk;
	double drabs2 = dr.abs2();
	double drabs = sqrt( drabs2 );
	double exparg = -drabs*_1_eps;
	if ( exparg < ExpArgRestriction ) {return;}
	double expres = expdef(exparg);
	TVec dS = rotl(pk1 - pk);
	*i3 += dS * expres;
	*i0 += (drabs*_1_eps+1)/drabs2*(dr*dS)*expres;
}}

int CalcVortexDiffMergeFast()
{
	DiffMergeFast_MergedV = 0;
	if ( !DiffMergeFast_S->VortexList ) return -1;

	double multiplier;
	GRestriction = DiffMergeFast_S->gmax();

	auto BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

	auto lBNode = BottomNodes->begin();
	auto lBNodeEnd = BottomNodes->end();
	for (; lBNode<lBNodeEnd; lBNode++ )
	{
		TNode &BNode = **lBNode;
		if ( !BNode.VortexLList ) { continue; }

		auto vllist = BNode.VortexLList;
		for (auto lObj = vllist->begin(); lObj<vllist->end(); lObj++ )
		{
			if (!*lObj) { continue; }
			TObj &Obj = **lObj;

			double eps, _1_eps;
			eps = Epsilon<Vortex>(BNode, &*lObj, true);
			eps = (eps > EpsRestriction) ? eps : EpsRestriction;
			_1_eps = 1/eps;

			TVec S2(0,0), S3(0,0);
			double S1 = 0, S0 = 0;

			auto nnodes = BNode.NearNodes;
			auto lNNode = nnodes->begin();
			auto lNNodeEnd = nnodes->end();
			for (; lNNode<lNNodeEnd; lNNode++ )
			{
				TNode &NNode = **lNNode;
				if ( NNode.VortexLList ) {
				auto jlist = NNode.VortexLList;
				auto lObjJ = jlist->begin();
				auto lObjJEnd = jlist->end();
				for ( ; lObjJ<lObjJEnd; lObjJ++ )
				{
					if (!*lObjJ) { continue; }
					TObj &ObjJ = **lObjJ;
					VortexInfluence(Obj, ObjJ, _1_eps, &S2, &S1);
				}}

				if ( NNode.BodyLList ) {
				auto jlist = NNode.BodyLList;
				auto lObjJ = jlist->begin();
				auto lObjJEnd = jlist->end();
				for ( ; lObjJ<lObjJ; lObjJ++ )
				{
					if (!*lObjJ) { continue; }
					TObj *ObjJ = *lObjJ;
					SegmentInfluence(Obj, *ObjJ, *DiffMergeFast_S->Body->next(vector<TObj>::const_iterator(ObjJ)), _1_eps, &S3, &S0);
				}}
			}

			if ( sign(S1) != sign(Obj) ) { S1 = Obj.g; }

			multiplier = DiffMergeFast_Nyu*_1_eps/S1;
			double S2abs = S2.abs2();
			if (S2abs > 100) { multiplier*=10/sqrtdef(S2abs); }
			Obj.v += multiplier * S2;
			Obj.v += (DiffMergeFast_Nyu*_1_eps*_1_eps/(C_2PI-S0)) * S3;
		}
	}

	return 0;
}

int CalcHeatDiffMergeFast()
{
	if ( !DiffMergeFast_S->HeatList ) return -1;

	//double multiplier2;
	//double M_35dfi2 = 3.5 * DiffMergeFast_dfi * DiffMergeFast_dfi;

	//TList<TObj> *BList = DiffMergeFast_S->Body->List;
	auto BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

/*	for (auto lBNode = BottomNodes->begin(); lBNode<BottomNodes->end(); lBNode++ )
	{
		TNode &BNode = **lBNode;
		if ( !BNode.HeatLList ) { continue; }

		TObj **lObj = BNode.HeatLList->begin();
		TObj **&LastObj = BNode.HeatLList->end();
		for ( ; lObj<LastObj; lObj++ )
		{
			TObj &Obj = **lObj;

			double epsilon, eps1;
			epsilon = Epsilon<Heat>(BNode, lObj, false);
			eps1 = 1/epsilon;

			TVec ResP;
			double ResD;
			Division<Heat>(BNode, Obj, eps1, &ResP, &ResD);
 
			if ( fabs(ResD) > S1Restriction )
			{
				Obj.v += ResP * (DiffMergeFast_Nyu/ResD*eps1);
			}

			if ( BList )
			{
				double rabs = sqrtdef(Obj.rx*Obj.rx + Obj.ry*Obj.ry);
				double exparg = (1-rabs)*eps1;
				if ( exparg > ExpArgRestriction )
				{
					//FIXME
					multiplier1 = M_35dfi2*expdef(exparg);
					multiplier2 = DiffMergeFast_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
					Obj.vx += Obj.rx * multiplier2;
					Obj.vy += Obj.ry * multiplier2;
				}
			}

		}
	}
*/
	return 0;
}

