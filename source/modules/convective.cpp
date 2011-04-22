#include <iostream>

#include "convective.h"

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Convective_S;
double Convective_Eps;

inline
Vector BioSavar(const TObject &obj, const Vector &p);

} //end of namespace

/********************* SOURCE *****************************/

int InitConvective(Space *sS, double sEps)
{
	Convective_S = sS;
	Convective_Eps = sEps;
	return 0;
}

namespace {
inline
Vector BioSavar(const TObject &obj, const Vector &p)
{
	Vector dr = p - obj;
	return rotl(dr)*(obj.g / (dr.abs2() + Convective_Eps) );
}}

Vector SpeedSum(TList<TObject> *List, Vector p)
{
	Vector dr, res(0, 0);

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		res+= BioSavar(*Obj, p);
	}

	res *= C_1_2PI;
	return res;
}

int CalcConvective()
{
	//double SpeedSumResX, SpeedSumResY;

	//double &InfX = Convective_S->InfSpeedXVar;
	//double &InfY = Convective_S->InfSpeedYVar;
	//double RotationG = Convective_S->RotationVVar * C_2PI;

	TList<TObject> *&vlist = Convective_S->VortexList;

	if (Convective_S->VortexList)
	{
		TObject *Obj = Convective_S->VortexList->First;
		TObject *&LastObj = Convective_S->VortexList->Last;
		for( ; Obj<LastObj; Obj++ )
		{
			Obj->v += SpeedSum(vlist, *Obj);
		}
	}

	return 0;
}

int CalcCirculation()
{
	/*if ( !Convective_S->BodyList ) return -1;

	double TwoInfX = 2*Convective_S->InfSpeedXVar; 
	double TwoInfY = 2*Convective_S->InfSpeedYVar;
	double RotationG = Convective_S->RotationVVar * C_2PI;

	TList<TObject> *VList = Convective_S->VortexList;
	double SpeedSumResX, SpeedSumResY;
	double dfi = C_2PI/Convective_S->BodyList->size;
	double CirculationAdditionDueToRotation = RotationG/Convective_S->BodyList->size;

	TObject *BVort = Convective_S->BodyList->First;
	TObject *&LastBVort = Convective_S->BodyList->Last;
	for( ; BVort<LastBVort; BVort++ )
	{
		SpeedSum(VList, BVort->rx, BVort->ry, 1, 1, SpeedSumResX, SpeedSumResY);
		BVort->g = (-BVort->ry*(SpeedSumResX+TwoInfX) + BVort->rx*(SpeedSumResY+TwoInfY))*dfi - CirculationAdditionDueToRotation;
	}
*/
	return 0;
}
