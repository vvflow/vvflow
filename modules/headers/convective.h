#ifndef __CONVECTIVE_H__
#define __CONVECTIVE_H__
#include <math.h>
#include "libVVHD/core.h"

int InitConvective(Space *sS, double sEps);
int CalcConvective(bool IncludeBody = false);
int CalcCirculation();
int SpeedSum(TList *List, double px, double py, double &resx, double &resy);

/*
class Convective
{
	public: 
		Convective(Space *sS, double sEps, double sInfSpeed);
		void CalcConvective();
		void CalcCirculation();
		Point SpeedSum(TList *List, Point p);

	private:
		Space *S;
		double Eps;
		double InfSpeed;
		Point InfSpeedP;

		Point Speed(TVortex *sVortex, Point p);
		Point ImSpeed(TVortex *sVortex, Point p);
		Point SpeedSum(TList *List, Point p, bool VTReal, bool VTIm);

};
*/
#endif
