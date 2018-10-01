#pragma once

#include "TSortedTree.hpp"
#include "TMatrix.hpp"

// #include <cmath>

class MConvectiveFast
{
    public:
        MConvectiveFast() = delete;
        MConvectiveFast(Space *S, const TSortedTree *tree);
        MConvectiveFast(const MConvectiveFast&) = delete;
        MConvectiveFast& operator=(const MConvectiveFast&) = delete;

        void process_all_lists();
        TVec velocity(TVec p) const;
        void calc_circulation(
            // 1) during collision
            // pointer to body.kspring.?.?
            // for which the collision equation will be filled
            // 2) after collision
            // pointer to body.force_holder.?.?
            // which is added to Hooke equation as (1+bounce)*force
            const void** collision
        );

    private:
        Space *S;
        const TSortedTree *tree;
        Matrix matrix;

        bool can_use_inverse();
        void fill_matrix(bool rightColOnly, const void** collision);
        TVec biot_savart(const TObj &obj, const TVec &p) const;
        TVec near_nodes_influence(const TSortedNode &Node, const TVec &p) const;
        TVec far_nodes_influence(const TSortedNode &Node, const TVec &p) const;
        TVec sink_list_influence(const TVec &p) const;
        TVec body_list_influence(const TVec &p) const;

        static double _2PI_Xi_g_near(TVec p, TVec pc, TVec dl, double rd);
        static double _2PI_Xi_g_dist(TVec p, TVec p1, TVec p2);
        static double _2PI_Xi_g(TVec p, const TAtt &seg, double rd); // in doc 2\pi\Xi_\gamma (1.7)
        static double _2PI_Xi_q_near(TVec p, TVec pc, TVec dl, double rd);
        static double _2PI_Xi_q_dist(TVec p, TVec p1, TVec p2);
        //double _2PI_Xi_q(const TVec &p, const TAtt &seg, double rd); // in doc 2\pi\Xi_q (1.8)
        static TVec _2PI_Xi(const TVec &p, const TAtt &seg, double rd);
        void _2PI_A123(const TAtt &seg, const TBody* ibody, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3);
        double ConvectiveInfluence(TVec p, const TAtt &seg, double rd) const;
        double NodeInfluence(const TSortedNode &Node, const TAtt &seg) const;
        double AttachInfluence(const TAtt &seg, double rd) const;
        TVec SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2) const;

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

        void fillHookeXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillHookeYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillHookeOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);

        void fillSpeedXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillSpeedYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
        void fillSpeedOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly);
};
