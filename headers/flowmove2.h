#ifndef __FLOWMOVE2_H__
#define __FLOWMOVE2_H__
#include <math.h>
#include "core.h"

class flowmove2
{
	public:
		flowmove2(Space *sS, double sdt, double sRemoveEps = 1E-10);
		void MovePart1();
		void MovePart2();
		void VortexShed();
		//void StreakShed(); //FIXME implement
		//void HeatShed();

		int CleanedV(){return CleanedV_;}
		//int CleanedH();
	private:
		void CheckObject(TObj*& lobj, bool remove);

		Space *S;
		double dt;
		double RemoveEps;
		int CleanedV_;
		//int CleanedH_;
};

#endif
