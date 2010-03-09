#include "convective.h"
#include "iostream"
using namespace std;

#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 		// = 2*PI

/********************* HEADER ****************************/

namespace {

Space *Convective_S;
double Convective_Eps;
double Convective_InfSpeedX;
double Convective_InfSpeedY;
double Convective_RotationG;

//Point Speed(TVortex *sVortex, Point p);
//Point ImSpeed(TVortex *sVortex, Point p);
void SpeedSum(TList *List, double px, double py, bool VTReal, bool VTIm, double &resx, double &resy);

} //end of namespace

/********************* SOURCE *****************************/

int InitConvective(Space *sS, double sEps)
{
	Convective_S = sS;
	Convective_Eps = sEps;
	return 0;
}

int SpeedSum(TList *List, double px, double py, double &resx, double &resy)
{
	double TempX, TempY; //temporary variables
	SpeedSum(List, px, py, 1, 0, TempX, TempY);
	resx = TempX * M_1_2PI;
	resy = TempY * M_1_2PI;
	return 0;
}

namespace {
void SpeedSum(TList *List, double px, double py, bool VTReal, bool VTIm, double &resx, double &resy)
{
	TVortex *Vort;
	resx = resy = 0;
	int i, lsize;
	double drx, dry, drabs2;
	double multiplier;

	lsize = List->size;
	Vort = List->Elements;
	if ( VTReal && !VTIm )
	{
		for ( i=0; i<lsize; i++ )
		{
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = drx*drx + dry*dry;
			multiplier = Vort->g / ( drabs2 + Convective_Eps ); // 1/2PI is in flowmove
			resx -= dry * multiplier;
			resy += drx * multiplier;
			Vort++;
		}
	} else 
	/*if ( !VTReal && VTIm )
	{} else*/ // THIS OPTION IS IMPOSSIBLE
	if ( VTReal && VTIm )
	{
		for ( i=0; i<lsize; i++ )
		{
			double vort1rabs2 = 1/(Vort->rx*Vort->rx + Vort->ry*Vort->ry);
			drx = px - Vort->rx * vort1rabs2;
			dry = py - Vort->ry * vort1rabs2;
			drabs2 = drx*drx + dry*dry;
			multiplier = Vort->g  / ( drabs2 + Convective_Eps ); // 1/2PI is in flowmove
			resx += dry * multiplier;
			resy -= drx * multiplier;

			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = drx*drx + dry*dry;
			multiplier = Vort->g / ( drabs2 + Convective_Eps ); // 1/2PI is in flowmove
			resx -= dry * multiplier;
			resy += drx * multiplier;
			
			Vort++;
		}
	}
}}

int CalcConvective(bool IncludeBody)
{
	int i, lsize;
	TVortex *Vort;
	TList *vlist = Convective_S->VortexList;
	TList *blist = Convective_S->BodyList;
	double SpeedSumResX, SpeedSumResY;

	if (Convective_S->InfSpeedX) { Convective_InfSpeedX = M_2PI*Convective_S->InfSpeedX(Convective_S->Time); } 
	else { Convective_InfSpeedX = 0; }
	if (Convective_S->InfSpeedY) { Convective_InfSpeedY = M_2PI*Convective_S->InfSpeedY(Convective_S->Time); } 
	else { Convective_InfSpeedY = 0; }
	if (Convective_S->RotationV) { Convective_RotationG = M_2PI*Convective_S->RotationV(Convective_S->Time); } 
	else { Convective_RotationG = 0; }

	if (vlist)
	{
		lsize = vlist->size;
		Vort = vlist->Elements;
		for( i=0; i<lsize; i++ )
		{
			SpeedSum(vlist, Vort->rx, Vort->ry, 1, 0, SpeedSumResX, SpeedSumResY);
			Vort->vx += Convective_InfSpeedX + SpeedSumResX;
			Vort->vy += Convective_InfSpeedY + SpeedSumResY;
			Vort++;
		}
		
		Vort = vlist->Elements;
		if ( IncludeBody && blist )
		{
			for( i=0; i<lsize; i++ )
			{
				SpeedSum(blist, Vort->rx, Vort->ry, 1, 0, SpeedSumResX, SpeedSumResY);
				Vort->vx += SpeedSumResX;
				Vort->vy += SpeedSumResY;
				Vort++;
			}
		}
	}

	if (Convective_S->HeatList)
	{
		lsize = Convective_S->HeatList->size;
		Vort = Convective_S->HeatList->Elements;
		for( i=0; i<lsize; i++ )
		{
			SpeedSum(vlist, Vort->rx, Vort->ry, 1, 0, SpeedSumResX, SpeedSumResY);
			Vort->vx += Convective_InfSpeedX + SpeedSumResX;
			Vort->vy += Convective_InfSpeedY + SpeedSumResY;

			Vort++;
		}

		Vort = Convective_S->HeatList->Elements;
		if ( IncludeBody && blist )
		{
			for( i=0; i<lsize; i++ )
			{
				SpeedSum(blist, Vort->rx, Vort->ry, 1, 0, SpeedSumResX, SpeedSumResY);
				Vort->vx += SpeedSumResX;
				Vort->vy += SpeedSumResY;
				Vort++;
			}
		}
	}
	return 0;
}

int CalcCirculation()
{
	if ( !Convective_S->BodyList ) return -1;

	if (Convective_S->InfSpeedX) { Convective_InfSpeedX = Convective_S->InfSpeedX(Convective_S->Time); } 
	else { Convective_InfSpeedX = 0; }
	if (Convective_S->InfSpeedY) { Convective_InfSpeedY = Convective_S->InfSpeedY(Convective_S->Time); } 
	else { Convective_InfSpeedY = 0; }
	if (Convective_S->RotationV) { Convective_RotationG = M_2PI*Convective_S->RotationV(Convective_S->Time); } 
	else { Convective_RotationG = 0; }
	double CirculationAdditionDueToRotation = Convective_RotationG/Convective_S->BodyList->size;

	int i, lsize;
	TVortex *Vort;
	TList* list = Convective_S->VortexList;
	double SpeedSumResX, SpeedSumResY;
	lsize = Convective_S->BodyList->size;
	
	double dfi = M_2PI/lsize; // angle between wto neighbor attached vortexes
	
	Vort = Convective_S->BodyList->Elements;
	for( i=0; i<lsize; i++ )
	{
		SpeedSum(list, Vort->rx, Vort->ry, 1, 1, SpeedSumResX, SpeedSumResY);
		Vort->g = M_1_2PI*(-SpeedSumResX*Vort->ry + SpeedSumResY*Vort->rx);
		Vort->g += 2*(-Vort->ry*Convective_InfSpeedX + Vort->rx*Convective_InfSpeedY); //InfSpeed
		Vort->g *= dfi;
		Vort->g -= CirculationAdditionDueToRotation;

		Vort++;
	}
	
	
	
	return 0;
}
