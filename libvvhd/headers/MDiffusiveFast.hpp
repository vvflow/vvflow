#pragma once

#include "TSpace.hpp"
#include "TSortedTree.hpp"

class diffusivefast
{
    public:
        diffusivefast(Space *S, const TSortedTree* Tree):
            S(S), Tree(Tree), Re(S->Re), Pr(S->Pr) {}
        void CalcVortexDiffusiveFast();
        void CalcHeatDiffusiveFast();

    private:
        Space *S;
        const TSortedTree* Tree;
        double Re;
        double Pr;
        enum ParticleType {Vortex, Heat};
        void VortexInfluence(const TObj &v, const TObj &vj, TVec *i2, double *i1);
        void SegmentInfluence(const TObj &v, TAtt *pk, TVec *i3, double *i0, bool calc_friction);
};
