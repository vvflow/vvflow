#ifndef SPACE_H_
#define SPACE_H_

class Space;

#include "body.h"
#include "elementary.h"

using namespace std;

typedef const char TValues;
namespace val
{
	TValues Cp = 1;
	TValues Fr = 2;
	TValues Nu = 4;
}

class Space
{
	public:
		Space(TVec (*sInfSpeed)(double time) = NULL);

		vector<TBody*> *BodyList;
		vector<TObj> *VortexList;
		vector<TObj> *HeatList;
		vector<TObj> *StreakSourceList;
		vector<TObj> *StreakList;

		inline void FinishStep(); //update time and coord variables

		TVec InfSpeed() { return InfSpeed_link?InfSpeed_link(Time):InfSpeed_const; }
		void ZeroSpeed();
		double Time, dt, Re, Pr;

		void Save(const char* format, const double header[]=NULL, int N=0);
		double* Load(const char* filename, int* N = NULL);
		FILE* OpenFile(const char* format);
		void SaveProfile(const char* filename, TValues vals);

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadVorticity_bin(const char* filename);
		int LoadHeatFromFile(const char* filename);

		int LoadBody(const char* filename);
		void EnumerateBodies();
		void ZeroBodies(); //zero Cp, Fr, Nu variables.

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

#endif /*SPACE_H_*/
