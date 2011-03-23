#include <math.h>
#include <cstdlib>
#include "diffmergefast.h"
#define expdef(x) fexp(x)
#define sqrtdef(x) sqrt(x)

const double ResDRestriction = 1E-6;
const double ExpArgRestriction = -8.;

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffMergeFast_S;
double DiffMergeFast_Re;
double DiffMergeFast_Nyu;
//double DiffMergeFast_dfi;

double DiffMergeFast_MergeSqEps;
int DiffMergeFast_MergedV;

int MergeVortexes(TObject **lv1, TObject **lv2);

enum ParticleType {Vortex, Heat};
}

/********************* SOURCE *****************************/

int InitDiffMergeFast(Space *sS, double sRe, double sMergeSqEps)
{
	DiffMergeFast_S = sS;
	DiffMergeFast_Re = sRe;
	DiffMergeFast_Nyu = 1/sRe;
	DiffMergeFast_MergeSqEps = sMergeSqEps;
	//DiffMergeFast_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;
	return 0;
}

namespace {
inline
int MergeVortexes(TObject **lv1, TObject **lv2)
{
	DiffMergeFast_MergedV++;
	if (!lv1 || !lv2 || (lv1==lv2)) return -1;
	if (!*lv1 || !*lv2) return -1;
	TObject &v1 = **lv1;
	TObject &v2 = **lv2;
	if ( ((v1.g > 0) && (v2.g > 0)) || ((v1.g < 0) && (v2.g < 0)) )
	{
		double g1sum = 1/(v1.g + v2.g);
		v1.rx = (v1.g*v1.rx + v2.g*v2.rx)*g1sum;
		v1.ry = (v1.g*v1.ry + v2.g*v2.ry)*g1sum;
	}
	else
	{
		if ( fabs(v1.g) < fabs(v2.g) )
		{
			v1.rx = v2.rx;
			v1.ry = v2.ry;
		}
	}
	v1.g+= v2.g; 
	v2.g = 0;
	*lv2 = NULL;
	return 0;
}}

int DiffMergedFastV()
{
	return DiffMergeFast_MergedV;
}

// EPSILON FUNCTIONS FOR VORTEXES

namespace {
template <bool Faster, ParticleType pt>
void Epsilon(TNode &Node, TObject **lv, double &res, bool merge)
{
	if (!lv || !*lv) { res=1E-20; return; }
	double drx, dry, drabs2;
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
			drx = v.rx - Obj.rx;
			dry = v.ry - Obj.ry;
			if (Faster) drabs2 = fabs(drx) + fabs(dry);
			else drabs2 = drx*drx + dry*dry;
			if (!drabs2) continue;
			if ( res1 > drabs2 ) { res2 = res1; lv2 = lv1; res1 = drabs2; lv1 = lObj;} 
			else if ( res2 > drabs2 ) { res2 = drabs2; lv2 = lObj; }
		}
	}

	if (Faster) res = res2;
	else res = sqrtdef(res2);

	if ( !lv || !lv1 ) { res=1E-10; return; }
	if ( !lv2 ) { res = sqrtdef(res1); return; }

	if ( (pt == Vortex) && merge)
	{
		TObject &v1 = **lv1;
		TObject &v2 = **lv2;
		if ( 
			((*lv<*lv1) && (res1 < (DiffMergeFast_MergeSqEps*(v.rx*v.rx+v.ry*v.ry+3.)*0.25) ))
			||
			( (v.g<0)&&(v1.g>0)&&(v2.g>0) ) || ( (v.g>0)&&(v1.g<0)&&(v2.g<0) )
			)
		{
			MergeVortexes(lv, lv1);
			//FIXME Maybe it's better to remember res3 instead of using recursive call
			Epsilon<Faster, pt>(Node, lv, res, false);
		}
	}
}}

namespace {
template <ParticleType pt>
void Division(TNode &Node, TObject &v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double xx, dxx;

	ResPX =	ResPY = ResD = 0.;

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
			drx = v.rx - Obj.rx;
			dry = v.ry - Obj.ry;
			if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { continue; }
			drabs = sqrtdef(drx*drx + dry*dry);

			double exparg = -drabs*eps1;
			if ( exparg > ExpArgRestriction )
			{
				xx = Obj.g * expdef(exparg); // look for define
				dxx = xx/drabs;
				ResPX += drx * dxx;
				ResPY += dry * dxx;
				ResD += xx;
			}
		}
	}

	if (pt == Vortex)
	if ( ( (ResD < 0) && (v.g > 0) ) || ( (ResD > 0) && (v.g < 0) ) ) { ResD = v.g; }
}}

int CalcVortexDiffMergeFast()
{
	DiffMergeFast_MergedV = 0;
	if ( !DiffMergeFast_S->VortexList ) return -1;

	double multiplier;
	double C_2Nyu_PI = DiffMergeFast_Nyu * C_2_PI;

	TList<TObject> *BList = DiffMergeFast_S->Body->List;
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

			double epsilon, eps1;
			Epsilon<true, Vortex>(BNode, lObj, epsilon, true);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD, ResVx, ResVy, ResAbs2;
			Division<Vortex>(BNode, Obj, eps1, ResPX, ResPY, ResD);

			if ( fabs(ResD) > ResDRestriction )
			{
				multiplier = DiffMergeFast_Nyu/ResD*eps1;
				ResVx = ResPX * multiplier;
				ResVy = ResPY * multiplier;
				ResAbs2 = ResVx*ResVx+ResVy*ResVy;
				if (ResAbs2 > 10000) { multiplier*=100/sqrtdef(ResAbs2); }
				Obj.vx += ResPX * multiplier;
				Obj.vy += ResPY * multiplier;
			}

			if ( BList )
			{
				double rabs = sqrtdef(Obj.rx*Obj.rx + Obj.ry*Obj.ry);
				double exparg = (1-rabs)*eps1;
				if (exparg > ExpArgRestriction)
				{
					multiplier = C_2Nyu_PI * eps1 * expdef(exparg) / rabs;
					Obj.vx += Obj.rx * multiplier; 
					Obj.vy += Obj.ry * multiplier;
				}
			}

		}
	}
	return 0;
}

int CalcHeatDiffMergeFast()
{
	if ( !DiffMergeFast_S->HeatList ) return -1;

	double multiplier1;
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
			Epsilon<true, Heat>(BNode, lObj, epsilon, false);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD;
			Division<Heat>(BNode, Obj, eps1, ResPX, ResPY, ResD);
 
			if ( fabs(ResD) > ResDRestriction )
			{
				multiplier1 = DiffMergeFast_Nyu/ResD*eps1;
				Obj.vx += ResPX * multiplier1;
				Obj.vy += ResPY * multiplier1;
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

