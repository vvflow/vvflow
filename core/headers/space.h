#ifndef SPACE_H_
#define SPACE_H_

#include "list.h"

class Space
{
	public:
		Space(bool CreateVortexes,
				bool CreateBody,
				bool CreateHeat, 
				double (*sInfSpeedX)(double Time) = NULL,
				double (*sInfSpeedY)(double Time) = NULL,
				double (*sRotationV)(double Time) = NULL);
		int ConstructCircle(long BodyListSize);

		TList<TObject> *VortexList;
		TList<TObject> *BodyList;
		TList<TObject> *HeatList;
		int *BodyControlLayer; //its filled by flowmove
		double ForceX, ForceY; //dont forget to zero it when u want

		double (*InfSpeedX)(double Time);
		double (*InfSpeedY)(double Time);
		double (*RotationV)(double Time);
		double Time;
		double Angle, BodyX, BodyY;


		int LoadVorticityFromFile(const char* filename);
		int LoadHeatFromStupidFile(const char* filename, double g);
		int LoadHeatFromFile(const char* filename);
		int Save(const char *filename);
		int Load(const char *filename);

		double Integral();
		double gsumm();
		void HydroDynamicMomentum(double &ResX, double &ResY);
};

#endif /*SPACE_H_*/
