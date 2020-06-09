#pragma once

#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "MConvectiveFast.hpp"
#include "MEpsilonFast.hpp"
#include "XField.hpp"

#include <list>

class XVelocity: public XField {
public:
    XVelocity(
        const Space &S,
        double xmin, double ymin,
        double dxdy,
        int xres, int yres
    );
    char mode;
    char ref_frame;
    void evaluate();
private:
    Space S;

    double dl; // = S.AverageSenmentLength()
    TSortedTree tree;
    MEpsilonFast eps;
    MConvectiveFast convective;
};
