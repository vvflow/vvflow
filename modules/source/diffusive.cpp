#include <math.h>
#include <cstdlib>
#include "diffusive.h"
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

Space *Diffusive_S;
double Diffusive_Re;
double Diffusive_Nyu;
double Diffusive_DefaultEpsilon;
double Diffusive_dfi;

void Epsilon(TList *List, double px, double py, double &res);
void Epsilon_faster(TList *List, double px, double py, double &res);

void Division(TList *List, TVortex *v, double eps1, double &ResPX, double &ResPY, double &ResD );

} //end of namespce

/********************* SOURCE *****************************/

int InitDiffusive(Space *sS, double sRe)
{
	Diffusive_S = sS;
	Diffusive_Re = sRe;
	Diffusive_Nyu = 1/sRe;
	Diffusive_DefaultEpsilon = Diffusive_Nyu * M_2PI;
	if (sS->BodyList) Diffusive_dfi = M_2PI/sS->BodyList->size; else Diffusive_dfi = 0;
	return 0;
}

namespace {
void Epsilon(TList *List, double px, double py, double &res)
{
	int i, lsize;
	TVortex *Obj;
	double drx, dry, drabs2;

	lsize = List->size;
	Obj = List->Elements;

	double res1, res2;
	res2 = res1 = 1E10;

	for ( i=0; i<lsize; i++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = drx*drx + dry*dry;
		if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
		Obj++;
	}
	res = sqrt(res2);
	//if (res < Diffusive_dfi) res = Diffusive_dfi;
}}

namespace {
void Epsilon_faster(TList *List, double px, double py, double &res)
{
	int i, lsize;
	TVortex *Obj;
	double eps; 
	double drx, dry, drabs2;

	lsize = List->size;
	Obj = List->Elements;

	double res1, res2;
	res2 = res1 = 1E10;
	for ( i=0; i<lsize; i++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = fabs(drx) + fabs(dry);
		if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
		Obj++;
	}
	res = res2;
	//if (res < Diffusive_dfi) res = Diffusive_dfi;
}}

namespace {
void Division(TList *List, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	int i, lsize;
	double drx, dry, drabs;
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
		drx = v->rx - Obj->rx;
		dry = v->ry - Obj->ry;
		if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { Obj++; continue; }
		drabs = sqrt(drx*drx + dry*dry);

		double exparg = -drabs*eps1;
		if ( exparg > -10 )
		{
			xx = Obj->g * expdef(exparg); //look for define
			dxx = xx/drabs;
			ResPX += drx * dxx;
			ResPY += dry * dxx;
			ResD += xx;
		}

		Obj++;
	}

	if ( ((ResD <= 0) && (v->g > 0)) || ((ResD >= 0) && (v->g < 0)) ) { ResD = v->g; }
}}

int CalcVortexDiffusive()
{
	double multiplier;
	double Four_Nyu = 4 *Diffusive_Nyu;
	double M_2PINyu = Diffusive_Nyu * M_2PI;
	TList* S_BodyList = Diffusive_S->BodyList;

	TList *vlist = Diffusive_S->VortexList;
	if ( !vlist) return -1;

	int lsize = vlist->size;
	TVortex *Obj = vlist->Elements;
	for( int i=0; i<lsize; i++ )
	{
		double epsilon, eps1;
		Epsilon(vlist, Obj->rx, Obj->ry, epsilon);
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(vlist, Obj, eps1, ResPX, ResPY, ResD);


		if ( fabs(ResD) > 1E-6 )
		{
			multiplier = M_2PINyu/ResD*eps1;
			Obj->vx += ResPX*multiplier;
			Obj->vy += ResPY*multiplier;
		}

		if ( S_BodyList )
		{
			//cout << "Body diff" << endl;
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

int CalcHeatDiffusive()
{
	int i, lsize;
	TVortex *Obj;
	
	double multiplier1, multiplier2;
	double M_7PIdfi2 = 21.9911485752 * Diffusive_dfi * Diffusive_dfi;
	double M_2PINyu = Diffusive_Nyu * M_2PI;
	TList* S_BodyList = Diffusive_S->BodyList;

	TList *hlist = Diffusive_S->HeatList;

	if ( !hlist) return -1;

	lsize = hlist->size;
	Obj = hlist->Elements;
	for( i=0; i<lsize; i++ )
	{
		double epsilon, eps1;
		Epsilon_faster(hlist, Obj->rx, Obj->ry, epsilon);
		//cout << Obj->rx << "\t" << Obj->ry << "\t" << epsilon << endl;
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(hlist, Obj, eps1, ResPX, ResPY, ResD);

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
				multiplier2 = Diffusive_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
				Obj->vx += Obj->rx * multiplier2; 
				Obj->vy += Obj->ry * multiplier2;
			}
		}

		Obj++;
	}

	return 0;
}

