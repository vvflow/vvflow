#include <math.h>
#include <cstdlib>
#include "diffusivefast.h"
#define expdef(x) exp(x)

const double ResDRestriction = 1E-6;
const double ExpArgRestriction = -8.;

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffusiveFast_S;
double DiffusiveFast_Re;
double DiffusiveFast_Nyu;
double DiffusiveFast_dfi;

enum ParticleType {Vortex, Heat};

/*
template <bool Faster, ParticleType pt>
void Epsilon(TNode &Node, double px, double py, double &res);
template <ParticleType pt>
void Division(TNode &Node, TObject &v, double eps1, double &ResPX, double &ResPY, double &ResD );
*/
}
/********************* SOURCE *****************************/

int InitDiffusiveFast(Space *sS, double sRe)
{
	DiffusiveFast_S = sS;
	DiffusiveFast_Re = sRe;
	DiffusiveFast_Nyu = 1/sRe;
	DiffusiveFast_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;
	return 0;
}

namespace {
template <bool Faster, ParticleType pt>
void Epsilon(TNode &Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	double res1, res2;
	res2 = res1 = 1E10;

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
			TObject &Obj = **lObj;
			drx = px - Obj.rx;
			dry = py - Obj.ry;
			if (Faster) drabs2 = fabs(drx) + fabs(dry);
			else drabs2 = drx*drx + dry*dry;
			if (!drabs2) continue;
			if ( res1 > drabs2 ) { res2 = res1; res1 = drabs2; } 
			else if ( res2 > drabs2 ) { res2 = drabs2; }
		}
	}

	if (res2 > 1E9)
	{
		TNode **lFNode = Node.FarNodes->First;
		TNode **&LastFNode = Node.FarNodes->Last;
		for ( ; lFNode<LastFNode; lFNode++)
		{
			TNode &FNode = **lFNode;

			switch (pt)
			{
				case Vortex:
					if ( !FNode.VortexLList ) { continue; }
					break;
				case Heat:
					if ( !FNode.HeatLList ) { continue; }
					break;
			}

			drx = px - FNode.x;
			dry = py - FNode.y;
			if (Faster) drabs2 = fabs(drx) + fabs(dry);
			else drabs2 = drx*drx + dry*dry;

			if ( (res2+FNode.h+FNode.w) > drabs2 ) { continue; }

			TObject **lObj;
			TObject ***lLastObj;
			switch (pt)
			{
				case Vortex:
					lObj = FNode.VortexLList->First;
					lLastObj = &FNode.VortexLList->Last;
					break;
				case Heat:
					lObj = FNode.HeatLList->First;
					lLastObj = &FNode.HeatLList->Last;
					break;
			}
			TObject **&LastObj = *lLastObj;
			for ( ; lObj<LastObj; lObj++ )
			{
				TObject &Obj = **lObj;
				drx = px - Obj.rx;
				dry = py - Obj.ry;
				drabs2 = drx*drx + dry*dry;
				if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2; } 
				else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
			}
		}
	}

	if (Faster) res = res2;
	else res = sqrt(res2);
}}

namespace {
template<ParticleType pt>
void Division(TNode &Node, TObject &v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double xx, dxx;

	ResPX = ResPY = ResD = 0.;

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
			TObject &Obj = **lObj;
			drx = v.rx - Obj.rx;
			dry = v.ry - Obj.ry;
			if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { continue; }
			drabs = sqrt(drx*drx + dry*dry);

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

int CalcVortexDiffusiveFast()
{
	if ( !DiffusiveFast_S->VortexList ) return -1;

	double multiplier;
	double C_2Nyu_PI = DiffusiveFast_Nyu * C_2_PI;

	TList<TObject> *BList = DiffusiveFast_S->BodyList;
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
			TObject &Obj = **lObj;

			double epsilon, eps1;
			Epsilon<true, Vortex>(BNode, Obj.rx, Obj.ry, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD, ResVx, ResVy, ResAbs2;
			Division<Vortex>(BNode, Obj, eps1, ResPX, ResPY, ResD);

			if ( fabs(ResD) > ResDRestriction )
			{
				multiplier = DiffusiveFast_Nyu/ResD*eps1;
				ResVx = ResPX * multiplier;
				ResVy = ResPY * multiplier;
				ResAbs2 = ResVx*ResVx+ResVy*ResVy;
				if (ResAbs2 > 10000) { multiplier*=100/sqrt(ResAbs2); }
				Obj.vx += ResPX * multiplier;
				Obj.vy += ResPY * multiplier;
			}

			if ( BList )
			{
				double rabs = sqrt(Obj.rx*Obj.rx + Obj.ry*Obj.ry);
				double exparg = (1-rabs)*eps1;
				if (exparg > ExpArgRestriction)
				{
					multiplier = C_2Nyu_PI * eps1 * expdef(exparg)/rabs;
					Obj.vx += Obj.rx * multiplier;
					Obj.vy += Obj.ry * multiplier;
				}
			}
		}
	}

	return 0;
}

int CalcHeatDiffusiveFast()
{
	if ( !DiffusiveFast_S->HeatList ) return -1;

	double multiplier1, multiplier2;
	double M_35dfi2 = 3.5 * DiffusiveFast_dfi * DiffusiveFast_dfi;

	TList<TObject> *BList = DiffusiveFast_S->BodyList;
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
			Epsilon<true, Heat>(BNode, Obj.rx, Obj.ry, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD;
			Division<Heat>(BNode, Obj, eps1, ResPX, ResPY, ResD);

			if ( fabs(ResD) > ResDRestriction )
			{
				multiplier1 = DiffusiveFast_Nyu/ResD*eps1;
				Obj.vx += ResPX * multiplier1;
				Obj.vy += ResPY * multiplier1;
			}

			if ( BList )
			{
				double rabs = sqrt(Obj.rx*Obj.rx + Obj.ry*Obj.ry);
				double exparg = (1-rabs)*eps1;
				if (exparg > ExpArgRestriction)
				{
					//FIXME
					multiplier1 = M_35dfi2*expdef(exparg);
					multiplier2 = DiffusiveFast_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
					Obj.vx += Obj.rx * multiplier2;
					Obj.vy += Obj.ry * multiplier2;
				}
			}
		}
	}

	return 0;
}

