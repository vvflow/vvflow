#pragma once

#include <cmath>

#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "TMatrix.hpp"

class convectivefast
{
    public:
        convectivefast(Space *S, const TSortedTree *Tree):
            S(S),
            Tree(Tree) {}
        void CalcConvectiveFast();
        void CalcBoundaryConvective();
        TVec SpeedSumFast(TVec p);

    public:
        void CalcCirculationFast();
        //void fillSlae();
        //void solveSlae();

    private:
        bool canUseInverse();
        void FillMatrix(bool rightColOnly = false);
        Matrix* getMatrix() {return &matrix;}

    private:
        TVec BioSavar(const TObj &obj, const TVec &p);
        TVec SpeedSum(const TSortedNode &Node, const TVec &p);
        TVec SrcSpeed(const TVec &p); // indeuced by S->SourceList in point p

        TVec BoundaryConvective(const TBody &b, const TVec &p);
        TVec BoundaryConvectiveSlip(const TBody &b, const TVec &p);

    private:
        Space *S;
        const TSortedTree *Tree;

        int MatrixSize;
        Matrix matrix;

        double _2PI_Xi_g(TVec p, const TAtt &seg, double rd); // in doc 2\pi\Xi_\gamma (1.7)
        //double _2PI_Xi_q(const TVec &p, const TAtt &seg, double rd); // in doc 2\pi\Xi_q (1.8)
        TVec _2PI_Xi(const TVec &p, const TAtt &seg, double rd);
        void _2PI_A123(const TAtt &seg, const TBody* ibody, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3);
        double ConvectiveInfluence(TVec p, const TAtt &seg, double rd);
        double NodeInfluence(const TSortedNode &Node, const TAtt &seg);
        double AttachInfluence(const TAtt &seg, double rd);
        TVec SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2);

    private:
        void fillSlipEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly);
        void fillZeroEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly);
        void fillSteadyEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly);
        void fillInfSteadyEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly);

        void fillHydroXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillHydroYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillHydroOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);

        void fillNewtonXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillNewtonYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillNewtonOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillCollisionOEquation(unsigned eq_no, TBody* ibody);

        void fillHookeXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillHookeYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillHookeOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);

        void fillSpeedXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillSpeedYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillSpeedOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
};
