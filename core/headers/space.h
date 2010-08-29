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

		inline void StartStep(); //update local InfSpeed variables, 
		inline void FinishStep(); //update time and coord variables

		double (*InfSpeedX)(double Time); double InfSpeedXVar;
		double (*InfSpeedY)(double Time); double InfSpeedYVar;
		double (*RotationV)(double Time); double RotationVVar;
		double Time, dt;
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

inline
void Space::StartStep()
{
	InfSpeedXVar = InfSpeedX ? InfSpeedX(Time) : 0;
	InfSpeedYVar = InfSpeedY ? InfSpeedY(Time) : 0;
	RotationVVar = RotationV ? RotationV(Time) : 0;
}

inline
void Space::FinishStep()
{
	if ( RotationV ) Angle+= RotationVVar*dt;
	if ( InfSpeedX ) BodyX-= InfSpeedXVar*dt;
	if ( InfSpeedY ) BodyY-= InfSpeedYVar*dt;
	Time+= dt;
}

#endif /*SPACE_H_*/
