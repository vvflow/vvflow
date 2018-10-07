#pragma once

#include "TSortedTree.hpp"

class MDiffusiveFast
{
    public:
        MDiffusiveFast(Space *S, const TSortedTree* tree):
            S(S), tree(tree) {}
        void process_vort_list();
        void process_heat_list();

    private:
        Space *S;
        const TSortedTree* tree;
        void vortex_influence(const TObj &v, const TObj &vj, TVec *i2, double *i1);
        void segment_influence(const TObj &v, TAtt *pk, TVec *i3, double *i0, bool calc_friction);
};
