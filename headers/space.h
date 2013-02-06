#ifndef SPACE_H_
#define SPACE_H_

class Space;

#include "body.h"
#include "tree.h"
#include "elementary.h"

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
		Space();
		char* name;
		time_t realtime;

		vector<TBody*> *BodyList;
		vector<TObj> *VortexList;
		vector<TObj> *HeatList;
		vector<TObj> *StreakSourceList;
		vector<TObj> *StreakList;
		TTree *Tree;

		inline void FinishStep(); //update time and coord variables

		ShellScript *InfSpeedX;
		ShellScript *InfSpeedY;
		TVec InfSpeed() {return TVec(InfSpeedX->getValue(Time), InfSpeedY->getValue(Time));}
		TVec InfMarker;
		void ZeroSpeed();
		double InfCirculation;
		TVec gravitation; //gravitational acceleration
		double Time, dt, Re, Pr, Finish;
		double save_dt, streak_dt, profile_dt;

		void Save(const char* format);
		void Load(const char* filename);
		FILE* OpenFile(const char* format);
		void CalcForces();
		void SaveProfile(const char* filename, double dt, TValues vals);
		void ZeroForces(); //zero all att-> Cp, Fr, Nu, gsum, fric, hsum variables.

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadVorticity_bin(const char* filename);
		int LoadHeatFromFile(const char* filename);
		int LoadStreak(const char* filename);
		int LoadStreakSource(const char* filename);

		int LoadBody(const char* filename, int cols=5);
		void EnumerateBodies();
		void ZeroBodies(); //zero Cp, Fr, Nu variables.

		/***************** INTEGRALS ******************/
		double integral();
		double gsum();
		double gmax();
		TVec HydroDynamicMomentum();
		double AverageSegmentLength();
};

#endif /*SPACE_H_*/
