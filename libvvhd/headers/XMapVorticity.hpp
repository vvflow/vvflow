#pragma once

#include <sstream>
#include "TSpace.hpp"
#include "TSortedTree.hpp"

class XMapVorticity {
public:
    XMapVorticity() = delete;
    XMapVorticity(const XMapVorticity&) = delete;
    XMapVorticity(XMapVorticity&&) = delete;
    XMapVorticity& operator=(const XMapVorticity&) = delete;
    XMapVorticity& operator=(XMapVorticity&&) = delete;

    XMapVorticity(
        const Space &S,
        double xmin, double ymin,
        double dxdy,
        int xres, int yres,
        double eps_mult
    );
    ~XMapVorticity();

    //  N   y1  y2  yN
    // x1  z11 z12 z1N
    // x2  z21 z22 z2N
    // xM  zM1 zM2 zMN
    friend std::ostream& operator<< (std::ostream& os, XMapVorticity& vrt);

private:
    Space S;
    double xmin, ymin;
    double dxdy;
    int    xres, yres;
    double eps_mult;
    float* map;

    bool evaluated;
    void evaluate();

    double dl; // = S.AverageSenmentLength()
    // square distance from p to the second neighbouring domain
    double eps2h(const TSortedNode &Node, TVec p) const;
    // square distance from p to the body surface
    double h2(const TSortedNode &Node, TVec p) const;

    double vorticity(const TSortedNode &node, TVec p) const;
};
