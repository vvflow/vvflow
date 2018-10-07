#pragma once

#include "TSortedTree.hpp"

class MEpsilonFast
{
    public:
        MEpsilonFast(Space *S, const TSortedTree *Tree):
            S(S), Tree(Tree), merged_(0) {}
        void CalcEpsilonFast(bool merge);
        int Merged(){return merged_;}

        // square distance from p to the second neighbouring domain
        // or double.lowest if there are no neighbours
        static double eps2h(const TSortedNode &node, TVec p);
        // square distance from p to the body surface
        // or double.infinity if there are no bodies
        static double h2(const TSortedNode &node, TVec p);

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
