#ifndef SPACE_H_
#define SPACE_H_

#include "list.h"

class Space
{
	public:
		Space(bool CreateVortexes,
				bool CreateBody,
				bool CreateHeat, 
				double (*sInfSpeedX)(double Time),
				double (*sInfSpeedY)(double Time),
				double (*sRotationV)(double Time));
		int ConstructCircle(long BodyListSize);

		TList *VortexList;
		TList *BodyList;
		int *BodyControlLayer; //its filled by flowmove
		TList *HeatList;
		double ForceX, ForceY; //dont forget to zero it when u want

		double (*InfSpeedX)(double Time);
		double (*InfSpeedY)(double Time);
		double (*RotationV)(double Time);
		double Time;
		double Angle;


		int LoadVorticityFromFile(char* filename);
		int LoadHeatFromStupidFile(char* filename, double g);
		int LoadHeatFromFile(char* filename);
		int Save(char *filename);
		int Load(char *filename);

		double Integral();
		double gsumm();
};

#endif /*SPACE_H_*/
