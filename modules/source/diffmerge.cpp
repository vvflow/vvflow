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

#define Const 0.287

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

void Epsilon(TList *List, double px, double py, double &res);
void Epsilon_faster(TList *List, double px, double py, double &res);

void Division(TList *List, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD );

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
		v1->g+= v2->g;
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
		v1->g += v2->g;
	}
	List->Remove(v2);
	return 0;
}}

int DiffMergedV()
{
	return DiffMerge_MergedV;
}

namespace {
void Epsilon(TList *List, TVortex* v, double &res)
{
	int i, lsize;
	TVortex *Vort;
	double px=v->rx, py=v->ry, drx, dry, drabs2;

	lsize = List->size;
	Vort = List->Elements;

	double res1, res2;
	TVortex *v1, *v2;
	res2 = res1 = 1E10;
	v1 = v2 = NULL;

	for ( i=0; i<lsize; i++ )
	{
		drx = px - Vort->rx;
		dry = py - Vort->ry;
		drabs2 = drx*drx + dry*dry;
		if ( (res1 > drabs2) && drabs2 ) { res2=res1; v2=v1; res1=drabs2; v1=Vort; }
		else if ( (res2 > drabs2) && drabs2 ) { res2=drabs2; v2=Vort; }
		Vort++;
	}

	if ( (res1 < DiffMerge_MergeSqEps) && (v<v1) )
	{
		MergeVortexes(List, v, v1);
	} else if ( ( (v->g<0)&&(v1->g>0)&&(v2->g>0) ) || ( (v->g>0)&&(v1->g<0)&&(v2->g<0) ) )
	{
		//MergeVortexes(List, v, v1);
	}

	res = Const*sqrt(res2);
	//if (res < DiffMerge_dfi) res = DiffMerge_dfi;
}}

namespace {
void Epsilon_faster(TList *List, double px, double py, double &res)
{
	int i, lsize;
	TVortex *Vort;
	double eps; 
	double drx, dry, drabs2;

	lsize = List->size;
	Vort = List->Elements;

	double res1, res2;
	res2 = res1 = 1E10;
	for ( i=0; i<lsize; i++ )
	{
		drx = px - Vort->rx;
		dry = py - Vort->ry;
		drabs2 = fabs(drx) + fabs(dry);
		if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
		Vort++;
	}
	res = Const*res2;
	//if (res < DiffMerge_dfi) res = DiffMerge_dfi;
}}

namespace {
void Division(TList *List, TVortex* Vort, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	int i, lsize;
	double drx, dry, drabs, dr1abs, drx2, dry2;
	double xx, dxx;
	double ResAbs2;
	TVortex *Obj;

	lsize = List->size;
	Obj = List->Elements;

	ResPX = 0;
	ResPY = 0;
	ResD = 0;

	for ( i=0 ; i<lsize; i++)
	{
		drx = Vort->rx - Obj->rx;
		dry = Vort->ry - Obj->ry;
		if ( (drx < 1E-6) && (dry2 < 1E-6) ) { Obj++; continue; }
		drabs = sqrt(drx*drx + dry*dry);
		dr1abs = 1/drabs;

		double exparg = -drabs*eps1;
		if ( exparg > -10 )
		{
			xx = Obj->g * expdef(exparg); //look for define
			dxx = dr1abs * xx;
			ResPX += drx * dxx;
			ResPY += dry * dxx;
			ResD += xx;
		}

		Obj++;
	}

	if ( ((ResD <= 0) && (Vort->g > 0)) || ((ResD >= 0) && (Vort->g < 0)) ) { ResD = Vort->g; }
}}

int DiffMerge()
{

	int i; long int *lsize;
	TVortex *Vort;
	double multiplier;

	TList *vlist = DiffMerge_S->VortexList;
	if ( !vlist) return -1;

	DiffMerge_MergedV=0;
	lsize = &(DiffMerge_S->VortexList->size);
	Vort = DiffMerge_S->VortexList->Elements;
	for( i=0; i<DiffMerge_S->VortexList->size; i++ )
	{
		double epsilon, eps1;
		Epsilon(vlist, Vort, epsilon);
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(vlist, Vort, eps1, ResPX, ResPY, ResD);

		if ( fabs(ResD) > 1E-12 )
		{
			multiplier = DiffMerge_Nyu/ResD*eps1*M_2PI;
			Vort->vx += ResPX*multiplier;
			Vort->vy += ResPY*multiplier;
		}

		if ( DiffMerge_S->BodyList )
		{
			//cout << "Body diff" << endl;
			double rabs, r1abs, erx, ery, exparg;
			rabs = sqrt(Vort->rx*Vort->rx + Vort->ry*Vort->ry);
			r1abs = 1/rabs;
			erx = Vort->rx*r1abs;
			ery = Vort->ry*r1abs;
			
			exparg = (1-rabs)*eps1;
			if (exparg > -8)
			{
				multiplier = 4 *DiffMerge_Nyu*eps1*expdef(exparg);
				Vort->vx += multiplier*erx; 
				Vort->vy += multiplier*ery;
			}
		}

		Vort++;
	}

	return 0;
}

/*int CalcHeatDiffusive()
{
	int i, lsize;
	TVortex *Vort;
	
	double multiplier1, multiplier2, dfi2 = Diffusive_dfi * Diffusive_dfi;

	TList *vlist = Diffusive_S->HeatList;

	if ( !Diffusive_S->HeatList) return -1;

	lsize = Diffusive_S->HeatList->size;
	Vort = Diffusive_S->HeatList->Elements;
	for( i=0; i<lsize; i++ )
	{
		double epsilon, eps1;
		Epsilon_faster(vlist, Vort->rx, Vort->ry, epsilon);
		//cout << Vort->rx << "\t" << Vort->ry << "\t" << epsilon << endl;
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(vlist, Vort, eps1, ResPX, ResPY, ResD);

		if ( fabs(ResD) > 1E-12 )
		{
			multiplier1 = Diffusive_Nyu/ResD*M_2PI;
			Vort->vx += ResPX*multiplier1;
			Vort->vy += ResPY*multiplier1;
		}

		if ( Diffusive_S->BodyList )
		{
			double rabs, r1abs, erx, ery, exparg;
			rabs = sqrt(Vort->rx*Vort->rx + Vort->ry*Vort->ry);
			r1abs = 1/rabs;
			erx = Vort->rx*r1abs;
			ery = Vort->ry*r1abs;
			exparg = (1-rabs)*eps1; //look for define
			if ( exparg > -8 )
			{
				#define M_7PI 21.9911485752
				multiplier1 = M_7PI*dfi2*expdef(exparg);
				multiplier2 = Diffusive_Nyu*eps1*multiplier1/(ResD+multiplier1);
				//printf ("%lf\n", multiplier2);
				Vort->vx += multiplier2*erx; 
				Vort->vy += multiplier2*ery;
			}
		}

		Vort++;
	}

	return 0;
}*/

