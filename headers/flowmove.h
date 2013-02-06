#ifndef __FLOWMOVE_H__
#define __FLOWMOVE_H__
#include <math.h>
#include "core.h"

class flowmove
{
	public:
		flowmove(Space *sS, double sRemoveEps = 1E-10);
		void MoveAndClean(bool remove, bool zero_speed = true);
		void VortexShed();
		void StreakShed(double shed_dt);
		void HeatShed();
		void CropHeat(double scale = 16);

		int CleanedV() {return CleanedV_;}
		//int CleanedH();
	private:
		Space *S;
		double dt;
		double RemoveEps;
		int CleanedV_;
		//int CleanedH_;
};

#endif
