#include <math.h>
#include <cstdlib>
#include "diffusive.h"
#define expdef(x) exp(x)

const double ResDRestriction = 1E-6;
const double ExpArgRestriction = -8;

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Diffusive_S;
double Diffusive_Re;
double Diffusive_Nyu;
double Diffusive_dfi;

inline void Epsilon(TList<TObject> *List, double px, double py, double &res);
inline void Epsilon_faster(TList<TObject> *List, double px, double py, double &res);

void Division(TList<TObject> *List, TObject &v, double eps1, double &ResPX, double &ResPY, double &ResD );

} //end of namespce

/********************* SOURCE *****************************/

int InitDiffusive(Space *sS, double sRe)
{
	Diffusive_S = sS;
	Diffusive_Re = sRe;
	Diffusive_Nyu = 1/sRe;
	Diffusive_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;
	return 0;
}

namespace {
inline
void Epsilon(TList<TObject> *List, double px, double py, double &res)
{
	double drx, dry, drabs2;
	double res1, res2;
	res2 = res1 = 1E10;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = drx*drx + dry*dry;
		if ( !drabs2 ) continue;
		if ( res1 > drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( res2 > drabs2 ) { res2 = drabs2; }
	}
	res = sqrt(res2);
}}

namespace {
inline
void Epsilon_faster(TList<TObject> *List, double px, double py, double &res)
{
	double drx, dry, drabs2;
	double res1, res2;
	res2 = res1 = 1E10;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = fabs(drx) + fabs(dry);
		if ( !drabs2 ) continue;
		if ( res1 > drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( res2 > drabs2 ) { res2 = drabs2; }
	}
	res = res2;
}}

namespace {
void Division(TList<TObject> *List, TObject &v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double xx, dxx;

	ResPX = ResPY =	ResD = 0;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++)
	{
		drx = v.rx - Obj->rx;
		dry = v.ry - Obj->ry;
		if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { continue; }
		drabs = sqrt(drx*drx + dry*dry);

		double exparg = -drabs*eps1;
		if ( exparg > ExpArgRestriction )
		{
			xx = Obj->g * expdef(exparg); //look for define
			dxx = xx/drabs;
			ResPX += drx * dxx;
			ResPY += dry * dxx;
			ResD += xx;
		}
	}

	if ( ((ResD <= 0) && (v.g > 0)) || ((ResD >= 0) && (v.g < 0)) ) { ResD = v.g; }
}}

int CalcVortexDiffusive()
{
	double multiplier;
	double C_2Nyu_PI = Diffusive_Nyu * C_2_PI; // 2*Nyu/PI

	TList<TObject> *BList = Diffusive_S->BodyList;
	TList<TObject> *VList = Diffusive_S->VortexList;
	if ( !VList ) return -1;

	TObject *Obj = VList->First;
	TObject *&LastObj = VList->Last;
	for( ; Obj<LastObj; Obj++ )
	{
		double epsilon, eps1;
		Epsilon_faster(VList, Obj->rx, Obj->ry, epsilon);
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(VList, *Obj, eps1, ResPX, ResPY, ResD);


		if ( fabs(ResD) > ResDRestriction )
		{
			multiplier = Diffusive_Nyu/ResD*eps1;
			Obj->vx += ResPX*multiplier;
			Obj->vy += ResPY*multiplier;
		}

		if ( BList )
		{
			double rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);
			double exparg = (1-rabs)*eps1;
			if (exparg > ExpArgRestriction)
			{
				multiplier = C_2Nyu_PI*eps1*expdef(exparg)/rabs;
				Obj->vx += Obj->rx * multiplier; 
				Obj->vy += Obj->ry * multiplier;
			}
		}
	}

	return 0;
}

int CalcHeatDiffusive()
{
	double multiplier1, multiplier2;
	double M_35dfi2 = 3.5 * Diffusive_dfi * Diffusive_dfi; //hope it's clear

	TList<TObject> *BList = Diffusive_S->BodyList;
	TList<TObject> *HList = Diffusive_S->HeatList;
	if ( !HList ) return -1;

	TObject *Obj = HList->First;
	TObject *&LastObj = HList->Last;
	for( ; Obj<LastObj; Obj++ )
	{
		double epsilon, eps1;
		Epsilon_faster(HList, Obj->rx, Obj->ry, epsilon);
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(HList, *Obj, eps1, ResPX, ResPY, ResD);

		if ( fabs(ResD) > ResDRestriction )
		{
			multiplier1 = Diffusive_Nyu/ResD;
			Obj->vx += ResPX*multiplier1;
			Obj->vy += ResPY*multiplier1;
		}

		if ( BList )
		{
			double rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);
			double exparg = (1-rabs)*eps1;
			if ( exparg > ExpArgRestriction )
			{
				multiplier1 = M_35dfi2*expdef(exparg); //look for define
				multiplier2 = Diffusive_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
				Obj->vx += Obj->rx * multiplier2; 
				Obj->vy += Obj->ry * multiplier2;
			}
		}
	}

	return 0;
}

