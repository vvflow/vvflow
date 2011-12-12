#ifndef SPACE_H_
#define SPACE_H_

#include "body.h"
#include "elementary.h"

using namespace std;

class Space
{
	public:
		Space(bool CreateVortexes,
				bool CreateHeat, 
				TVec (*sInfSpeed)(double time) = NULL);

		vector<TBody*> *BodyList;
		vector<TObj> *VortexList;
		vector<TObj> *HeatList;
		vector<TObj> *StreakSourceList;
		vector<TObj> *StreakList;

		inline void FinishStep(); //update time and coord variables

		TVec InfSpeed() { return InfSpeed_link?InfSpeed_link(Time):InfSpeed_const; }
		void ZeroSpeed();
		double Time, dt, Re;

		void Save(const char* format, const double header[]=NULL, int N=0);
		void SaveProfile(const char* filename);
		double* Load(const char* filename, int* N = NULL);
		FILE* OpenFile(const char* format);

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadVorticity_bin(const char* filename);
		int LoadHeatFromFile(const char* filename);

		int LoadBody(const char* filename);
		void EnumerateBodies();

		/***************** INTEGRALS ******************/
		double integral();
		double gsum();
		double gmax();
		TVec HydroDynamicMomentum();
		double AverageSegmentLength();

	private:
		TVec (*InfSpeed_link)(double time);
		TVec InfSpeed_const;
};

inline
void Space::FinishStep()
{
	const_for(BodyList, llbody)
	{
		TBody &body = **llbody;
		body.Rotate(body.RotSpeed(Time) * dt);
		body.Position -= InfSpeed() * dt;
	}
	Time+= dt;
}

#endif /*SPACE_H_*/
