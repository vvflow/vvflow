#include <math.h>
#include <cstdlib>
#include "diffmerge.h"
#define expdef(x) exp(x)
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#ifndef M_1_PI 
	#define M_1_PI 0.318309886 	// = 1/PI 
#endif
#define M_2PI 6.283185308 		// = 2*PI
#ifndef M_2_PI 
	#define M_2_PI 0.636619772 		// = 2/PI
#endif

//#define Const 0.287

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffMerge_S;
double DiffMerge_Re;
double DiffMerge_Nyu;
double DiffMerge_DefaultEpsilon;
double DiffMerge_dfi;

double DiffMerge_MergeSqEps;
int DiffMerge_MergedV;

int MergeVortexes(TList *List, TVortex* v1, TVortex* v2);

void EpsilonV(TList *List, TVortex* v, double &res);
void EpsilonV_faster(TList *List, TVortex* v, double &res);
void EpsilonH_(TList *List, double px, double py, double &res);
void EpsilonH_faster(TList *List, double px, double py, double &res);

void Division_vortex(TList *List, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD );
void Division_heat(TList *List, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD );

} //end of namespce

/********************* SOURCE *****************************/

int InitDiffMerge(Space *sS, double sRe, double sMergeSqEps)
{
	DiffMerge_S = sS;
	DiffMerge_Re = sRe;
	DiffMerge_Nyu = 1/sRe;
	DiffMerge_DefaultEpsilon = DiffMerge_Nyu * M_2PI;
	DiffMerge_MergeSqEps = sMergeSqEps;
	if (sS->BodyList) DiffMerge_dfi = M_2PI/sS->BodyList->size; else DiffMerge_dfi = 0;
	return 0;
}

namespace {
int MergeVortexes(TList *List, TVortex* v1, TVortex* v2)
{
	DiffMerge_MergedV++;
	if (!v1 || !v2 || (v1==v2)) return -1;
	if ( ((v1->g > 0) && (v2->g > 0)) || ((v1 < 0) && (v2 < 0)) )
	{
		double g1sum = 1/(v1->g + v2->g);
		v1->rx = (v1->g*v1->rx + v2->g*v2->rx)*g1sum;
		v1->ry = (v1->g*v1->ry + v2->g*v2->ry)*g1sum;
	}
	else
	{
		if ( fabs(v1->g) < fabs(v2->g) )
		{
			v1->rx = v2->rx;
			v1->ry = v2->ry;
		}
	}
	v1->g+= v2->g;
	List->Remove(v2);
	return 0;
}}

int DiffMergedV()
{
	return DiffMerge_MergedV;
}

namespace {
void EpsilonV(TList *List, TVortex* v, double &res)
{
	double px=v->rx, py=v->ry, drx, dry, drabs2;

	int lsize = List->size;
	TVortex *Obj = List->Elements;

	double res1, res2;
	TVortex *v1, *v2;
	res2 = res1 = 1E10;
	v1 = v2 = NULL;

	for ( int i=0; i<lsize; i++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = drx*drx + dry*dry;
		if ( (res1 > drabs2) && drabs2 ) { res2=res1; v2=v1; res1=drabs2; v1=Obj; }
		else if ( (res2 > drabs2) && drabs2 ) { res2=drabs2; v2=Obj; }
		Obj++;
	}

	if ( (res1 < DiffMerge_MergeSqEps) && (v<v1) )
	{
		MergeVortexes(List, v, v1);
	} else if ( ( (v->g<0)&&(v1->g>0)&&(v2->g>0) ) || ( (v->g>0)&&(v1->g<0)&&(v2->g<0) ) )
	{
		MergeVortexes(List, v, v1);
	}

	res = sqrt(res2);
	//if (res < DiffMerge_dfi) res = DiffMerge_dfi;
}}

namespace {
void EpsilonV_faster(TList *List, TVortex* v, double &res)
{
	double eps; 
	double px=v->rx, py=v->ry, drx, dry, drabs2;

	int lsize = List->size;
	TVortex *Obj = List->Elements;

	double res1, res2;
	TVortex *v1, *v2;
	res2 = res1 = 1E10;
	v1 = v2 = NULL;

	for ( int i=0; i<lsize; i++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = fabs(drx) + fabs(dry);
		if ( (res1 > drabs2) && drabs2 ) { res2 = res1; v2=v1; res1=drabs2; v1=Obj; }
		else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; v2=Obj; }
		Obj++;
	}

	if ( (res1 < DiffMerge_MergeSqEps) && (v<v1) )
	{
		MergeVortexes(List, v, v1);
	} else if ( ( (v->g<0)&&(v1->g>0)&&(v2->g>0) ) || ( (v->g>0)&&(v1->g<0)&&(v2->g<0) ) )
	{
		MergeVortexes(List, v, v1);
	}

	res = res2;
	//if (res < DiffMerge_dfi) res = DiffMerge_dfi;
}}

namespace {
void EpsilonH(TList *List, double px, double py, double &res)
{
	double drx, dry, drabs2;

	int lsize = List->size;
	TVortex *Obj = List->Elements;

	double res1, res2;
	TVortex *v1, *v2;
	res2 = res1 = 1E10;
	v1 = v2 = NULL;

	for ( int i=0; i<lsize; i++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = drx*drx + dry*dry;
		if ( (res1 > drabs2) && drabs2 ) { res2=res1; v2=v1; res1=drabs2; v1=Obj; }
		else if ( (res2 > drabs2) && drabs2 ) { res2=drabs2; v2=Obj; }
		Obj++;
	}

	res = sqrt(res2);
	//if (res < DiffMerge_dfi) res = DiffMerge_dfi;
}}

namespace {
void EpsilonH_faster(TList *List, double px, double py, double &res)
{
	double eps; 
	double drx, dry, drabs2;

	int lsize = List->size;
	TVortex *Obj = List->Elements;

	double res1, res2;
	res2 = res1 = 1E10;
	for ( int i=0; i<lsize; i++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = fabs(drx) + fabs(dry);
		if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
		Obj++;
	}

	res = res2;
	//if (res < DiffMerge_dfi) res = DiffMerge_dfi;
}}

namespace {
void Division_vortex(TList *List, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double px=v->rx, py=v->ry;
	double xx, dxx;

	int lsize = List->size;
	TVortex *Obj = List->Elements;

	ResPX = 0;
	ResPY = 0;
	ResD = 0;

	for ( int i=0 ; i<lsize; i++)
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { Obj++; continue; }
		drabs = sqrt(drx*drx + dry*dry);

		double exparg = -drabs*eps1;
		if ( exparg > -10 )
		{
			xx = Obj->g * expdef(exparg); //look for define
			dxx = xx / drabs;
			ResPX += drx * dxx;
			ResPY += dry * dxx;
			ResD += xx;
		}

		Obj++;
	}

	if ( ((ResD <= 0) && (v->g > 0)) || ((ResD >= 0) && (v->g < 0)) ) { ResD = v->g; }
}}

namespace {
void Division_vortex(TList *List, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double xx, dxx;

	int lsize = List->size;
	TVortex *Obj = List->Elements;

	ResPX = 0;
	ResPY = 0;
	ResD = 0;

	for ( int i=0 ; i<lsize; i++)
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { Obj++; continue; }
		drabs = sqrt(drx*drx + dry*dry);

		double exparg = -drabs*eps1;
		if ( exparg > -10 )
		{
			xx = Obj->g * expdef(exparg); //look for define
			dxx = xx / drabs;
			ResPX += drx * dxx;
			ResPY += dry * dxx;
			ResD += xx;
		}

		Obj++;
	}
}}

int CalcVortexDiffMerge()
{
	double multiplier;
	double Four_Nyu = 4 *DiffMerge_Nyu;
	double M_2PINyu = DiffMerge_Nyu * M_2PI;
	TList *S_BodyList = DiffMerge_S->BodyList;

	TList *vlist = DiffMerge_S->VortexList;
	if ( !vlist) return -1;

	DiffMerge_MergedV=0;
	TVortex *Obj = vlist->Elements;
	long int *lsize = &(vlist->size);
	for( long int i=0; i<*lsize; i++ )
	{
		double epsilon; EpsilonV_faster(vlist, Obj, epsilon);
		double eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division_vortex(vlist, Obj, eps1, ResPX, ResPY, ResD);

		if ( fabs(ResD) > 1E-12 )
		{
			multiplier = M_2PINyu/ResD*eps1;
			Obj->vx += ResPX*multiplier;
			Obj->vy += ResPY*multiplier;
		}

		if ( S_BodyList )
		{
			double rabs, exparg;
			rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);
			
			exparg = (1-rabs)*eps1;
			if (exparg > -8)
			{
				multiplier = Four_Nyu*eps1*expdef(exparg)/rabs;
				Obj->vx += Obj->rx * multiplier; 
				Obj->vy += Obj->ry * multiplier;
			}
		}

		Obj++;
	}

	return 0;
}

int CalcHeatDiffMerge()
{
	double multiplier1, multiplier2;
	double M_7PIdfi2 = 21.9911485752 * DiffMerge_dfi * DiffMerge_dfi;
	double M_2PINyu = DiffMerge_Nyu * M_2PI;
	TList *S_BodyList = DiffMerge_S->BodyList;

	TList *hlist = DiffMerge_S->HeatList;
	if ( !hlist) return -1;

	int lsize = hlist->size;
	TVortex *Obj = hlist->Elements;
	for( int i=0; i<lsize; i++ )
	{
		double epsilon; EpsilonH_faster(hlist, Obj->rx, Obj->ry, epsilon);
		double eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division_heat(hlist, Obj->rx, Obj->ry, eps1, ResPX, ResPY, ResD);

		if ( fabs(ResD) > 1E-12 )
		{
			multiplier1 = M_2PINyu/ResD;
			Obj->vx += ResPX*multiplier1;
			Obj->vy += ResPY*multiplier1;
		}

		if ( S_BodyList )
		{
			double rabs, exparg;
			rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);

			exparg = (1-rabs)*eps1; //look for define
			if ( exparg > -8 )
			{
				multiplier1 = M_7PIdfi2*expdef(exparg);
				multiplier2 = DiffMerge_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
				Obj->vx += Obj->rx * multiplier2; 
				Obj->vy += Obj->ry * multiplier2;
			}
		}

		Obj++;
	}

	return 0;
}

