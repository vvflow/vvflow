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
		//double merge_criteria_sq; // = (0.3*AverageSegmentLenght)^2
		double eps_restriction; // = 0.6*AverageSegmentLenght 
		int merged_;
		
		void MergeVortexes(TObj *lv1, TObj *lv2);
		double epsv(const TSortedNode &Node, TObj *lv, double merge_criteria_sq);
		double epsh(const TSortedNode &Node, TObj *lv, double merge_criteria_sq);
		TAtt* nearestBodySegment(TSortedNode &node, TVec p);
};

#endif
