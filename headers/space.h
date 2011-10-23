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

		TVec InfSpeed() { return InfSpeed_link?InfSpeed_link(Time):TVec(0,0); }
		double Time, dt;

		void Save(const char* format, const double header[]=NULL, int N=0);
		void Load(const char* format);
		FILE* OpenFile(const char* format);

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadVorticity_bin(const char* filename);
		int LoadHeatFromFile(const char* filename);

		int LoadBody(const char* filename);

		int PrintBody(const char* format);
		int PrintVorticity(const char* format);
		int PrintVorticity_bin(const char* format);
		int PrintHeat(const char* format);

		void LoadHeader(const char* filename, char* data, streamsize size);
		void PrintHeader(const char* format, const double data[], int size);

		double integral();
		double gsum();
		double gmax();
		TVec HydroDynamicMomentum();

	private:
		int Print_byos(vector<TObj> *list, std::ostream& os, bool bin);
		int Print_bymask(vector<TObj> *list, const char* format, ios::openmode mode = ios::out); //format is used for sprintf(filename, "format", time)

		TVec (*InfSpeed_link)(double time);
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
