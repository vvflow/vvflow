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

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Diffusive_S;
double Diffusive_ReR, Diffusive_ReD;
double Diffusive_NyuR, Diffusive_NyuD;
double Diffusive_DefaultEpsilon;
double Diffusive_dfi;

void Epsilon(TList *List, double px, double py, double &res);
void Epsilon_faster(TList *List, double px, double py, double &res);

void Division(TList *List, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD );

} //end of namespce

/********************* SOURCE *****************************/

int InitDiffusive(Space *sS, double sReD)
{
	Diffusive_S = sS;
	Diffusive_ReD = sReD;
	Diffusive_ReR = sReD*0.5;
	Diffusive_NyuD = 1/sReD;
	Diffusive_NyuR = 2/sReD;
	Diffusive_DefaultEpsilon = Diffusive_NyuR * M_2PI;
	if (sS->BodyList) Diffusive_dfi = M_2PI/sS->BodyList->size; else Diffusive_dfi = 0;
	return 0;
}

namespace {
void Epsilon(TList *List, double px, double py, double &res)
{
	int i, lsize;
	TVortex *Vort;
	double eps; 
	double drx, dry, drabs2;

	lsize = List->size;
	Vort = List->Elements;

	if (lsize < 2) res = Diffusive_DefaultEpsilon;
	else
	{
		eps = 1E10; // = inf //Diffusive_DefaultEpsilon*Diffusive_DefaultEpsilon;
	
		for ( i=0; i<lsize; i++ )
		{
				drx = px - Vort->rx;
				dry = py - Vort->ry;
				drabs2 = drx*drx + dry*dry;
				if ( (drabs2 < eps) && (drabs2 > 1E-14) )	eps = drabs2; 
				Vort++;
		}
		res = sqrt(eps);
	}
	//if (res < Diffusive_dfi) res = Diffusive_dfi;
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

	if (lsize < 2) res = Diffusive_DefaultEpsilon;
	else
	{
		eps = 1E10; // = inf //Diffusive_DefaultEpsilon*Diffusive_DefaultEpsilon;
	
		for ( i=0; i<lsize; i++ )
		{
				drx = px - Vort->rx;
				dry = py - Vort->ry;
				drabs2 = fabs(drx) + fabs(dry);
				if ( (drabs2 < eps) && drabs2 )	eps = drabs2; 
				Vort++;
		}
		res = sqrt(eps);
	}
	if (res < Diffusive_dfi) res = Diffusive_dfi;
}}

namespace {
void Division(TList *List, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	int i, lsize;
	double drx, dry, drabs, dr1abs, drx2, dry2;
	double xx, dxx;
	double ResAbs2;
	TVortex *Vort;

	lsize = List->size;
	Vort = List->Elements;

	ResPX = 0;
	ResPY = 0;
	ResD = 0;

/*	if ((px == 0.03) && (py==0)) 
	for ( i=0 ; i<lsize; i++)
	{	cout << List->Elements[i].rx << "\t" << List->Elements[i].ry << endl; }
*/



	for ( i=0 ; i<lsize; i++)
	{
		drx = px - Vort->rx;
		dry = py - Vort->ry;
		if ( (drx < 1E-6) && (dry2 < 1E-6) ) { Vort++; continue; }
		drabs = sqrt(drx*drx + dry*dry);
		dr1abs = 1/drabs;

		double exparg = -drabs*eps1;
		if ( exparg > -10 )
		{
			xx = Vort->g * expdef(exparg); //look for define
			dxx = dr1abs * xx;
			ResPX += drx * dxx;
			ResPY += dry * dxx;
			ResD += xx;
		}

		Vort++;
	}
}}


int CalcVortexDiffusive()
{

	int i, lsize;
	TVortex *Vort;
	
	double multiplier;

	TList *vlist = Diffusive_S->VortexList;

	if ( !Diffusive_S->VortexList) return -1;

	lsize = Diffusive_S->VortexList->size;
	Vort = Diffusive_S->VortexList->Elements;
	for( i=0; i<lsize; i++ )
	{
		double epsilon, eps1;
		Epsilon(vlist, Vort->rx, Vort->ry, epsilon);
		//double rnd = (1. + double(rand())/RAND_MAX);
		epsilon*= 1.5;
		eps1= 1/epsilon;
		double ResPX, ResPY, ResD;
		Division(vlist, Vort->rx, Vort->ry, eps1, ResPX, ResPY, ResD);



		if ( ((ResD <= 0) && (Vort->g > 0)) || ((ResD >= 0) && (Vort->g < 0)) ) { ResD = Vort->g; }
		if ( fabs(ResD) > 1E-12 )
		{
			multiplier = Diffusive_NyuR/ResD*eps1*M_2PI;
			Vort->vx += ResPX*multiplier;
			Vort->vy += ResPY*multiplier;
		}

		if ( Diffusive_S->BodyList )
		{
			cout << "Body diff" << endl;
			double rabs, r1abs, erx, ery, exparg;
			rabs = sqrt(Vort->rx*Vort->rx + Vort->ry*Vort->ry);
			r1abs = 1/rabs;
			erx = Vort->rx*r1abs;
			ery = Vort->ry*r1abs;
			
			exparg = (1-rabs)*eps1;
			if (exparg > -8)
			{
				multiplier = 4 *Diffusive_NyuR*eps1*expdef(exparg);
				Vort->vx += multiplier*erx; 
				Vort->vy += multiplier*ery;
			}
		}

		Vort++;
	}

	return 0;
}

int CalcHeatDiffusive()
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
		Division(vlist, Vort->rx, Vort->ry, eps1, ResPX, ResPY, ResD);

		if ( ( (ResD <= 0) && (Vort->g > 0) ) || ( (ResD >= 0) && (Vort->g < 0) ) ) { ResD = Vort->g; } 
		if ( fabs(ResD) > 1E-12 )
		{
			multiplier1 = Diffusive_NyuR/ResD*M_2PI;
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
				multiplier2 = Diffusive_NyuR*eps1*multiplier1/(ResD+multiplier1);
				//printf ("%lf\n", multiplier2);
				Vort->vx += multiplier2*erx; 
				Vort->vy += multiplier2*ery;
			}
		}

		Vort++;
	}

	return 0;
}

