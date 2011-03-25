#ifndef SPACE_H_
#define SPACE_H_

//#include "elementary.h"
#include "list.h"
#include "body.h"

class Space
{
	public:
		Space(bool CreateVortexes,
				bool CreateHeat, 
				double (*sInfSpeedX)(double Time) = NULL,
				double (*sInfSpeedY)(double Time) = NULL);

		TBody *Body;
		TList<TObject> *VortexList;
		TList<TObject> *HeatList;


		inline void StartStep(); //update local InfSpeed variables, 
		inline void FinishStep(); //update time and coord variables

		double (*InfSpeedX)(double Time); double InfSpeedXVar;
		double (*InfSpeedY)(double Time); double InfSpeedYVar;
		double Time, dt;
		double BodyX, BodyY;

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadHeatFromStupidFile(const char* filename, double g);
		int LoadHeatFromFile(const char* filename);

		int PrintBody(std::ostream& os);
		int PrintVorticity(std::ostream& os);
		int PrintHeat(std::ostream& os);

		int PrintBody(const char* format);
		int PrintVorticity(const char* format);
		int PrintHeat(const char* format);

		int Save(const char *filename);
		int Load(const char *filename);

		double Integral();
		double gsum();
		double gmax();
		void HydroDynamicMomentum(double &ResX, double &ResY);

	private:
		int Print(TList<TObject> *List, std::ostream& os);
		int Print(TList<TObject> *List, const char* format); //format is used for sprintf(filename, "format", time)
};

inline
void Space::StartStep()
{
	InfSpeedXVar = InfSpeedX ? InfSpeedX(Time) : 0;
	InfSpeedYVar = InfSpeedY ? InfSpeedY(Time) : 0;
	Body->RotationVVar = Body->RotationV ? Body->RotationV(Time) : 0;
}

inline
void Space::FinishStep()
{
	if ( Body->RotationV ) Body->Rotate(Body->RotationVVar*dt);
	if ( InfSpeedX ) BodyX-= InfSpeedXVar*dt;
	if ( InfSpeedY ) BodyY-= InfSpeedYVar*dt;
	Time+= dt;
}

#endif /*SPACE_H_*/
