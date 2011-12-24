#ifndef EPSFAST_H
#define EPSFAST_H
#include "core.h"

class epsfast
{
	public:
		epsfast(Space *sS);
		void CalcEpsilonFast(bool merge);
		int Merged(){return merged_;}

	private:
		Space *S;
		double merge_criteria_sq; // = (0.3*AverageSegmentLenght)^2
		double eps_restriction; // = 0.6*AverageSegmentLenght 
		int merged_;
		
		void MergeVortexes(TObj **lv1, TObj **lv2);
		double epsv(const TNode &Node, TObj **lv, bool merge);
		double epsh(const TNode &Node, TObj **lv, bool merge);
};

#endif
