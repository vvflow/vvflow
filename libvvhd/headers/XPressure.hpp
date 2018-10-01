#pragma once

#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "MConvectiveFast.hpp"
#include "MDiffusiveFast.hpp"
#include "MFlowmove.hpp"
#include "MEpsilonFast.hpp"
#include "XField.hpp"

class XPressure: public XField {
public:
    XPressure(
        const Space &S,
        double xmin, double ymin,
        double dxdy,
        int xres, int yres
    );
    double eps_mult;
    char ref_frame;
    void evaluate();
private:
    Space S;
    TVec ref_frame_speed;
    double dl; // = S.AverageSenmentLength()
    TSortedTree tree;
    MConvectiveFast convective;
    MDiffusiveFast diffusive;
    MFlowmove flowmove;
    MEpsilonFast eps;
    double pressure(TVec p) const;
};
