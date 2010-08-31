#include "convective.h"
#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Convective_S;
double Convective_Eps;

void SpeedSum(TList<TVortex> *List, double px, double py, bool VTReal, bool VTIm, double &resx, double &resy);

} //end of namespace

/********************* SOURCE *****************************/

int InitConvective(Space *sS, double sEps)
{
	Convective_S = sS;
	Convective_Eps = sEps;
	return 0;
}

int SpeedSum(TList<TVortex> *List, double px, double py, double &resx, double &resy)
{
	SpeedSum(List, px, py, 1, 0, resx, resy);
	return 0;
}

namespace {
void SpeedSum(TList<TVortex> *List, double px, double py, bool VTReal, bool VTIm, double &resx, double &resy)
{
	resx = resy = 0;
	double drx, dry;
	double multiplier;

	TVortex *Vort = List->First;
	TVortex *&Last = List->Last;
	if ( VTReal && !VTIm )
	{
		for ( ; Vort<Last; Vort++ )
		{
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			//drabs2 = drx*drx + dry*dry;  //optimization attempt
			#define drabs2 drx*drx+dry*dry
			multiplier = Vort->g / ( drabs2 + Convective_Eps );
			#undef drabs2
			resx -= dry * multiplier;
			resy += drx * multiplier;
		}
	} else 
	/*if ( !VTReal && VTIm )
	{} else*/ // THIS OPTION IS IMPOSSIBLE
	if ( VTReal && VTIm )
	{
		for ( ; Vort<Last; Vort++ )
		{
			double vort1rabs2 = 1/(Vort->rx*Vort->rx + Vort->ry*Vort->ry);
			drx = px - Vort->rx * vort1rabs2;
			dry = py - Vort->ry * vort1rabs2;
			//drabs2 = drx*drx + dry*dry;
			#define drabs2 drx*drx+dry*dry
			multiplier = Vort->g  / ( drabs2 + Convective_Eps );
			#undef drabs2
			resx += dry * multiplier;
			resy -= drx * multiplier;

			drx = px - Vort->rx;
			dry = py - Vort->ry;
			//drabs2 = drx*drx + dry*dry;
			#define drabs2 drx*drx+dry*dry
			multiplier = Vort->g / ( drabs2 + Convective_Eps );
			#undef drabs2
			resx -= dry * multiplier;
			resy += drx * multiplier;
		}
	}

	resx *= C_1_2PI;
	resy *= C_1_2PI;
}}

int CalcConvective()
{
	double SpeedSumResX, SpeedSumResY;

	double &InfX = Convective_S->InfSpeedXVar;
	double &InfY = Convective_S->InfSpeedYVar;
	double RotationG = Convective_S->RotationVVar * C_2PI;

	TList<TObject> *&vlist = Convective_S->VortexList;
	double multiplier;
	#define SpeedCycle(List) 												\
	if (List) 																\
	{ 																		\
		TObject *Obj = List->First; 										\
		TObject *&Last = List->Last; 										\
		for( ; Obj<Last; Obj++ ) 											\
		{ 																	\
			multiplier = RotationG ? RotationG/(Obj->rx*Obj->rx + Obj->ry*Obj->ry) : 0; \
			SpeedSum(vlist, Obj->rx, Obj->ry, 1, 0, SpeedSumResX, SpeedSumResY); \
			Obj->vx += InfX + SpeedSumResX - Obj->ry*multiplier; 			\
			Obj->vy += InfY + SpeedSumResY - Obj->rx*multiplier; 			\
		} 																	\
	}

	SpeedCycle(Convective_S->VortexList)
	SpeedCycle(Convective_S->HeatList)

	#undef SpeedCycle

	return 0;
}

int CalcCirculation()
{
	if ( !Convective_S->BodyList ) return -1;

	double TwoInfX = 2*Convective_S->InfSpeedXVar; 
	double TwoInfY = 2*Convective_S->InfSpeedYVar;
	double RotationG = Convective_S->RotationVVar * C_2PI;

	TList<TVortex> *VList = Convective_S->VortexList;
	double SpeedSumResX, SpeedSumResY;
	double dfi = C_2PI/Convective_S->BodyList->size;
	double CirculationAdditionDueToRotation = RotationG/Convective_S->BodyList->size;

	TVortex *BVort = Convective_S->BodyList->First;
	TVortex *&LastBVort = Convective_S->BodyList->Last;
	for( ; BVort<LastBVort; BVort++ )
	{
		SpeedSum(VList, BVort->rx, BVort->ry, 1, 1, SpeedSumResX, SpeedSumResY);
		BVort->g = (-BVort->ry*(SpeedSumResX+TwoInfX) + BVort->rx*(SpeedSumResY+TwoInfY))*dfi - CirculationAdditionDueToRotation;
	}

	return 0;
}
