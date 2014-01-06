#ifndef SPACE_H_
#define SPACE_H_

class Space;

#include "body.h"
#include "sorted_tree.h"
#include "elementary.h"

#include <iostream>

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
		std::string caption;
		time_t realtime;

		vector<TBody*> *BodyList;
		vector<TObj> *VortexList;
		vector<TObj> *HeatList;
		vector<TObj> *StreakSourceList;
		vector<TObj> *StreakList;
		TSortedTree *Tree;

		inline void FinishStep(); //update time and coord variables

		ShellScript *InfSpeedX;
		ShellScript *InfSpeedY;
		TVec InfSpeed() {return TVec(InfSpeedX->getValue(Time), InfSpeedY->getValue(Time));}
		TVec InfMarker;
		void ZeroSpeed();
		double InfCirculation;
		TVec gravitation; //gravitational acceleration
		TTime Time, dt, save_dt, streak_dt, profile_dt;
		double Re, Pr, Finish;

		void Save(const char* format);
		void Load(const char* filename);
		FILE* OpenFile(const char* format);
		void CalcForces();
		void SaveProfile(const char* filename, TValues vals);
		void ZeroForces(); //zero all att-> Cp, Fr, Nu, gsum, fric, hsum variables.

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadVorticity_bin(const char* filename);
		int LoadHeatFromFile(const char* filename);
		int LoadStreak(const char* filename);
		int LoadStreakSource(const char* filename);
		const char* getGitInfo();
		const char* getGitDiff();

		int LoadBody(const char* filename, int cols=5);
		void EnumerateBodies();
		void ZeroBodies(); //zero Cp, Fr, Nu variables.

		/***************** INTEGRALS ******************/
		double integral();
		double gsum();
		double gmax();
		TVec HydroDynamicMomentum();
		double AverageSegmentLength();

	private:
		void Load_v1_3(const char* filename);
};

#endif /*SPACE_H_*/
