#pragma once

#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "XField.hpp"

#include <list>

class XStreamfunction: public XField {
public:
    XStreamfunction(
        const Space &S,
        double xmin, double ymin,
        double dxdy,
        int xres, int yres,
        double eps_mult,
        char ref_frame
    );

    void evaluate();
private:
    Space S;
    double eps_mult;
    TVec ref_frame_speed;

    double dl; // = S.AverageSenmentLength()
    double rd2; // = (0.2*dl)^2

    double streamfunction(const TSortedNode &node, TVec p, double* psi_gap) const;
};
