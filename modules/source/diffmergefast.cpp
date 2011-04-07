#include <math.h>
#include <cstdlib>
#include "diffmergefast.h"
#define expdef(x) fexp(x)
#define sqrtdef(x) sqrt(x)

const double S1Restriction = 1E-6;
const double ExpArgRestriction = -8.;

#include "iostream"
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
	EpsRestriction = (sS->Body) ? 0.6*sS->Body->SurfaceLength()/sS->Body->List->size : 0;
	//DiffMergeFast_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;
	return 0;
}

namespace {
inline
int MergeVortexes(TObject **lv1, TObject **lv2)
{
	if (!lv1 || !lv2 || (lv1==lv2)) return -1;
	if (!*lv1 || !*lv2) return -1;
	TObject &v1 = **lv1;
	TObject &v2 = **lv2;
	//if (fabs(v1.g + v2.g) > GRestriction) return -1;
	DiffMergeFast_MergedV++;

	if ( ((v1.g > 0) && (v2.g > 0)) || ((v1.g < 0) && (v2.g < 0)) )
	{
		double g1sum = 1/(v1.g + v2.g);
		v1.rx = (v1.g*v1.rx + v2.g*v2.rx)*g1sum;
		v1.ry = (v1.g*v1.ry + v2.g*v2.ry)*g1sum;
		cerr << "NOTIFICATION from diffmerge!" << endl;
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

namespace {
//inline
void VortexInfluence(const TObject &v, const TObject &vj, double eps1, double *i2x, double *i2y, double *i1)
{
	double drx = v.rx - vj.rx;
	double dry = v.ry - vj.ry;
	if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { return; }
	double drabs = sqrt(drx*drx+dry*dry);
	double exparg = -drabs*eps1;
	if ( exparg < ExpArgRestriction ) return;
	double i1tmp = vj.g * expdef(exparg);
	*i2x += drx * (i1tmp/drabs);
	*i2y += dry * (i1tmp/drabs);
	*i1 += i1tmp;
	if (*i2x!= *i2x) { cerr << drx << " " << dry << " " << drabs << " " << i1tmp << endl; }
	if (*i2x!= *i2x) { cerr << "ALERT! i2x = " << i2x << endl; sleep(1E6); }
}}

namespace {
//inline
void SegmentInfluence(const TObject &v, const TObject &pk, const TObject &pk1, 
					double eps1, double *i3x, double *i3y, double *i0)
{
	double rkx = (pk.rx+pk1.rx)*0.5;
	double rky = (pk.ry+pk1.ry)*0.5;

	double drx = v.rx - rkx;
	double dry = v.ry - rky;
	double drabs2;
	double drabs = sqrt( drabs2 = drx*drx+dry*dry );
	//cerr << drabs << " " << drabs2 << " " << drx << " " << dry << endl;
	double exparg = -drabs*eps1;
	if ( exparg < ExpArgRestriction ) return;
	double exp = expdef(exparg);
	double dSx = (-pk1.ry + pk.ry);
	double dSy = (pk1.rx - pk.rx);
	*i3x += dSx * exp;
	*i3y += dSy * exp;
	//cerr << (drabs*eps1+1) << " " << drabs2 << " " << (drx*dSx + dry*dSy) << " " << exp << endl;
	//cerr << &pk << " \t" << &pk1 << " \t" << *i0 << endl;
	*i0 += (drabs*eps1+1)/drabs2*(drx*dSx + dry*dSy)*exp;
	//cerr << *i0 << endl;
	if (*i0 != *i0) { sleep(1E6); }
}}

int CalcVortexDiffMergeFast()
{
	DiffMergeFast_MergedV = 0;
	if ( !DiffMergeFast_S->VortexList ) return -1;

	double multiplier;
	TObject *&FirstBVort = DiffMergeFast_S->Body->List->First;
	TObject *&LastBVort = DiffMergeFast_S->Body->List->Last;

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

			double epsilon, eps1;
			Epsilon<true, Vortex>(BNode, lObj, epsilon, true);
			epsilon = (epsilon > EpsRestriction) ? epsilon : EpsRestriction;
			eps1 = 1/epsilon;

			double S2x, S2y, S1, S3x, S3y, S0;//, ResPY, ResD, ResVx, ResVy, ResAbs2;
			S2x = S2y = S1 = S3x = S3y = S0 = 0;

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
					VortexInfluence(Obj, ObjJ, eps1, &S2x, &S2y, &S1);
				}}

				if ( NNode.BodyLList ) {
				TObject **lObjJ = NNode.BodyLList->First;
				TObject **&LastObjJ = NNode.BodyLList->Last;
				for ( ; lObjJ<LastObjJ; lObjJ++ )
				{
					if (!*lObjJ) { continue; }
					TObject *ObjJ = *lObjJ;
					SegmentInfluence(Obj, *ObjJ, *((ObjJ==(LastBVort-1))?FirstBVort:ObjJ+1), eps1, &S3x, &S3y, &S0);
				}}
			}

			if ( ( (S1 <= 0) && (Obj.g > 0) ) || ( (S1 >= 0) && (Obj.g < 0) ) ) { S1 = Obj.g; }

			multiplier = DiffMergeFast_Nyu*eps1/S1;
			double S2abs = S2x*S2x+S2y*S2y;
			if (S2abs > 100) { multiplier*=10/sqrtdef(S2abs); }
			Obj.vx += multiplier * S2x;
			Obj.vy += multiplier * S2y;

			if (multiplier!= multiplier) { cerr << "ALERT! multiplier = " << multiplier << endl; }
			if (S2x!= S2x) { cerr << "ALERT! S2x = " << S2x << endl; }
			if (S2y!= S2y) { cerr << "ALERT! S2y = " << S2y << endl; }
			if (Obj.vx!= Obj.vx) { cerr << "ALERT! Obj.vx = " << Obj.vx << " " << multiplier << " " << S2x << " " << Obj.rx << " " << Obj.ry << endl; sleep(1E6); }
			if (Obj.vy!= Obj.vy) { cerr << "ALERT! Obj.vy = " << Obj.vy << endl; }


			multiplier = DiffMergeFast_Nyu*eps1*eps1/(C_2PI-S0);
			Obj.vx += multiplier * S3x;
			Obj.vy += multiplier * S3y;

			if (multiplier!= multiplier) { cerr << "ALERT! multiplier = " << multiplier << endl; }
			if (S3x!= S3x) { cerr << "ALERT! S3x = " << S3x << endl; }
			if (S3y!= S3y) { cerr << "ALERT! S3y = " << S3y << endl; }

			//FIXME may work unproperly
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
 
			if ( fabs(ResD) > S1Restriction )
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

