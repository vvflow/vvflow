#pragma once

#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "XField.hpp"

class XVorticity: public XField {
public:
    XVorticity(
        const Space &S,
        double xmin, double ymin,
        double dxdy,
        int xres, int yres,
        double eps_mult
    );
    void evaluate();
private:
    Space S;
    double eps_mult;
    double dl; // = S.AverageSenmentLength()
    double vorticity(const TSortedNode &node, TVec p) const;
};
