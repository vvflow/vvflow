#include <iostream>

#include "convective.h"

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Convective_S;
double Convective_Eps;

inline
TVec BioSavar(const TObj &obj, const TVec &p);

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
TVec BioSavar(const TObj &obj, const TVec &p)
{
	TVec dr = p - obj;
	return rotl(dr)*(obj.g / (dr.abs2() + Convective_Eps) );
}}

TVec SpeedSum(vector<TObj> *list, TVec p)
{
	TVec dr, res(0, 0);

	const_for(list, lobj)
	{
		res+= BioSavar(*lobj, p);
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

	auto list = Convective_S->VortexList;
	if (!list) return -1;

	const_for(list, lobj)
	{
		lobj->v += SpeedSum(list, *lobj) +
					TVec(Convective_S->InfSpeedX->getValue(Convective_S->Time), 
						Convective_S->InfSpeedY->getValue(Convective_S->Time));
	}

	return 0;
}

int CalcCirculation()
{
	/*if ( !Convective_S->BodyList ) return -1;

	double TwoInfX = 2*Convective_S->InfSpeedXVar; 
	double TwoInfY = 2*Convective_S->InfSpeedYVar;
	double RotationG = Convective_S->RotationVVar * C_2PI;

	vector<TObj> *VList = Convective_S->VortexList;
	double SpeedSumResX, SpeedSumResY;
	double dfi = C_2PI/Convective_S->BodyList->size;
	double CirculationAdditionDueToRotation = RotationG/Convective_S->BodyList->size;

	TObj *BVort = Convective_S->BodyList->First;
	TObj *&LastBVort = Convective_S->BodyList->Last;
	for( ; BVort<LastBVort; BVort++ )
	{
		SpeedSum(VList, BVort->rx, BVort->ry, 1, 1, SpeedSumResX, SpeedSumResY);
		BVort->g = (-BVort->ry*(SpeedSumResX+TwoInfX) + BVort->rx*(SpeedSumResY+TwoInfY))*dfi - CirculationAdditionDueToRotation;
	}
*/
	return 0;
}
