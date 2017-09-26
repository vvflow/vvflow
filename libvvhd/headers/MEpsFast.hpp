#pragma once

#include "TSortedTree.hpp"

class epsfast
{
    public:
        epsfast(Space *S, const TSortedTree *Tree):
            S(S), Tree(Tree), merged_(0) {}
        void CalcEpsilonFast(bool merge);
        int Merged(){return merged_;}

    private:
        Space *S;
        const TSortedTree *Tree;
        //double merge_criteria_sq; // = (0.3*AverageSegmentLenght)^2
        // double eps_restriction; // = 0.6*AverageSegmentLenght 
        int merged_;

        void MergeVortexes(TObj *lv1, TObj *lv2);
        double epsv(const TSortedNode &Node, TObj *lv, double merge_criteria_sq);
        double epsh(const TSortedNode &Node, TObj *lv, double merge_criteria_sq);
        TAtt* nearestBodySegment(TSortedNode &node, TVec p);
};
