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

void MergeVortexes(TObject **lv1, TObject **lv2);

enum ParticleType {Vortex, Heat};
}

/********************* SOURCE *****************************/

int InitDiffMergeFast(Space *sS, double sRe, double sMergeSqEps)
{
	DiffMergeFast_S = sS;
	DiffMergeFast_Re = sRe;
	DiffMergeFast_Nyu = 1/sRe;
	DiffMergeFast_MergeSqEps = sMergeSqEps;
	EpsRestriction = (sS->Body) ? 0.6*sS->Body->SurfaceLength()/sS->Body->List->size : 0;
	//DiffMergeFast_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;
	return 0;
}

namespace {
void MergeVortexes(TObject **lv1, TObject **lv2)
{
	if (!lv1 || !lv2 || (lv1==lv2)) return;
	if (!*lv1 || !*lv2) return;
	TObject &v1 = **lv1;
	TObject &v2 = **lv2;
	//if (fabs(v1.g + v2.g) > GRestriction) return -1;
	DiffMergeFast_MergedV++;

	if ( sign(v1) == sign(v2) )
	{
		v1 = (v1*v1.g + v2*v2.g)/(v1.g + v2.g);
		//FIXME what happens with velocity?
	}
	else if ( fabs(v1.g) < fabs(v2.g) ) { v1 = v2; }
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
double Epsilon(const TNode &Node, TObject **lv, bool merge)
{
	double res = 0;
	if (!lv || !*lv) { return 1E-20; }
	Vector dr;
	double res1, res2;
	res2 = res1 = 1E10;

	TObject &v = **lv;
	TObject **lv1, **lv2;
	lv1 = lv2 = NULL;

	TNode **lNNode = Node.NearNodes->First;
	TNode **&LastNNode = Node.NearNodes->Last;
	for ( ; lNNode<LastNNode; lNNode++ )
	{
		TNode &NNode = **lNNode;

		TObject **lObj;
		TObject ***lLastObj;
		switch (pt)
		{
			case Vortex:
				if ( !NNode.VortexLList ) { continue; }
				lObj = NNode.VortexLList->First;
				lLastObj = &NNode.VortexLList->Last;
				break;
			case Heat:
				if ( !NNode.HeatLList ) { continue; }
				lObj = NNode.HeatLList->First;
				lLastObj = &NNode.HeatLList->Last;
				break;
		}
		TObject **&LastObj = *lLastObj;

		for ( ; lObj<LastObj; lObj++ )
		{
			if (!*lObj) { continue; }
			TObject &Obj = **lObj;
			dr = v - Obj;
			double drabs2 = dr.abs2();
			if (dr.iszero()) continue;
			if ( res1 > drabs2 ) { res2 = res1; lv2 = lv1; res1 = drabs2; lv1 = lObj;} 
			else if ( res2 > drabs2 ) { res2 = drabs2; lv2 = lObj; }
		}
	}

	res = sqrtdef(res2);

	if ( !lv || !lv1 ) { return 1E-20; }
	if ( !lv2 ) { return sqrtdef(res1)*1.1; }

	if ( (pt == Vortex) && merge)
	{
		TObject &v1 = **lv1;
		TObject &v2 = **lv2;
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

	return res;
}}

namespace {
template <ParticleType pt>
void Division(const TNode &Node, const TObject &v, double _1_eps, Vector* ResP, double *ResD )
{
	Vector dr;

	ResP->zero(); *ResD = 0.;

	TNode **lNNode = Node.NearNodes->First;
	TNode **&LastNNode = Node.NearNodes->Last;
	for ( ; lNNode<LastNNode; lNNode++ )
	{
		TNode &NNode = **lNNode;

		TObject **lObj;
		TObject ***lLastObj;
		switch (pt)
		{
			case Vortex:
				if ( !NNode.VortexLList ) { continue; }
				lObj = NNode.VortexLList->First;
				lLastObj = &NNode.VortexLList->Last;
				break;
			case Heat:
				if ( !NNode.HeatLList ) { continue; }
				lObj = NNode.HeatLList->First;
				lLastObj = &NNode.HeatLList->Last;
				break;
		}
		TObject **&LastObj = *lLastObj;

		for ( ; lObj<LastObj; lObj++ )
		{
			if (!*lObj) { continue; }
			TObject &Obj = **lObj;
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
void VortexInfluence(const TObject &v, const TObject &vj, double _1_eps, Vector *i2, double *i1)
{
	if ( sign(v) != sign(vj) ) { return; }
	Vector dr = v - vj;
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
void SegmentInfluence(const TObject &v, const TObject &pk, const TObject &pk1, 
					double _1_eps, Vector *i3, double *i0)
{
	Vector rk = (pk+pk1)*0.5;

	Vector dr = v - rk;
	double drabs2 = dr.abs2();
	double drabs = sqrt( drabs2 );
	double exparg = -drabs*_1_eps;
	if ( exparg < ExpArgRestriction ) {return;}
	double expres = expdef(exparg);
	Vector dS = rotl(pk1 - pk);
	*i3 += dS * expres;
	*i0 += (drabs*_1_eps+1)/drabs2*(dr*dS)*expres;
}}

int CalcVortexDiffMergeFast()
{
	DiffMergeFast_MergedV = 0;
	if ( !DiffMergeFast_S->VortexList ) return -1;

	double multiplier;
	GRestriction = DiffMergeFast_S->gmax();

	TList<TNode*> *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

	TNode **lBNode = BottomNodes->First;
	TNode **&LastBNode = BottomNodes->Last;
	for ( ; lBNode<LastBNode; lBNode++ )
	{
		TNode &BNode = **lBNode;
		if ( !BNode.VortexLList ) { continue; }

		TObject **lObj = BNode.VortexLList->First;
		TObject **&LastObj = BNode.VortexLList->Last;
		for ( ; lObj<LastObj; lObj++ )
		{
			if (!*lObj) { continue; }
			TObject &Obj = **lObj;

			double eps, _1_eps;
			eps = Epsilon<Vortex>(BNode, lObj, true);
			eps = (eps > EpsRestriction) ? eps : EpsRestriction;
			_1_eps = 1/eps;

			Vector S2(0,0), S3(0,0);
			double S1 = 0, S0 = 0;

			TNode **lNNode = BNode.NearNodes->First;
			TNode **&LastNNode = BNode.NearNodes->Last;
			for ( ; lNNode<LastNNode; lNNode++ )
			{
				TNode &NNode = **lNNode;
				if ( NNode.VortexLList ) {
				TObject **lObjJ = NNode.VortexLList->First;
				TObject **&LastObjJ = NNode.VortexLList->Last;
				for ( ; lObjJ<LastObjJ; lObjJ++ )
				{
					if (!*lObjJ) { continue; }
					TObject &ObjJ = **lObjJ;
					VortexInfluence(Obj, ObjJ, _1_eps, &S2, &S1);
				}}

				if ( NNode.BodyLList ) {
				TObject **lObjJ = NNode.BodyLList->First;
				TObject **&LastObjJ = NNode.BodyLList->Last;
				for ( ; lObjJ<LastObjJ; lObjJ++ )
				{
					if (!*lObjJ) { continue; }
					TObject *ObjJ = *lObjJ;
					SegmentInfluence(Obj, *ObjJ, *DiffMergeFast_S->Body->Next(ObjJ), _1_eps, &S3, &S0);
				}}
			}

			if ( ( (S1 <= 0) && (Obj.g > 0) ) || ( (S1 >= 0) && (Obj.g < 0) ) ) { S1 = Obj.g; }

			multiplier = DiffMergeFast_Nyu*_1_eps/S1;
			double S2abs = S2.abs();
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

	//TList<TObject> *BList = DiffMergeFast_S->Body->List;
	TList<TNode*> *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

	TNode **lBNode = BottomNodes->First;
	TNode **&LastBNode = BottomNodes->Last;
	for ( ; lBNode<LastBNode; lBNode++ )
	{
		TNode &BNode = **lBNode;
		if ( !BNode.HeatLList ) { continue; }

		TObject **lObj = BNode.HeatLList->First;
		TObject **&LastObj = BNode.HeatLList->Last;
		for ( ; lObj<LastObj; lObj++ )
		{
			TObject &Obj = **lObj;

			double epsilon, eps1;
			epsilon = Epsilon<Heat>(BNode, lObj, false);
			eps1 = 1/epsilon;

			Vector ResP;
			double ResD;
			Division<Heat>(BNode, Obj, eps1, &ResP, &ResD);
 
			if ( fabs(ResD) > S1Restriction )
			{
				Obj.v += ResP * (DiffMergeFast_Nyu/ResD*eps1);
			}
/*
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
*/
		}
	}

	return 0;
}

