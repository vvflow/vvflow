#pragma once

#include <sstream>
#include "TSpace.hpp"
#include "TSortedTree.hpp"

struct gap_t {
    uint16_t xi;
    uint16_t yj;
    float psi_gap;
};

class XMapStreamfunction {
public:
    XMapStreamfunction() = delete;
    XMapStreamfunction(const XMapStreamfunction&) = delete;
    XMapStreamfunction(XMapStreamfunction&&) = delete;
    XMapStreamfunction& operator=(const XMapStreamfunction&) = delete;
    XMapStreamfunction& operator=(XMapStreamfunction&&) = delete;

    XMapStreamfunction(
        const Space &S,
        double xmin, double ymin,
        double dxdy,
        int xres, int yres,
        double eps_mult,
        char ref_frame
    );
    ~XMapStreamfunction();

    //  N   y1  y2  yN
    // x1  z11 z12 z1N
    // x2  z21 z22 z2N
    // xM  zM1 zM2 zMN
    friend std::ostream& operator<< (std::ostream& os, XMapStreamfunction& vrt);

private:
    Space S;
    double xmin, ymin;
    double dxdy;
    int    xres, yres;
    double eps_mult;
    TVec ref_frame_speed;
    float* map;
    std::vector<gap_t> gap_list;

    bool evaluated;
    void evaluate();

    double dl; // = S.AverageSenmentLength()
    double rd2; // = (0.2*dl)^2
    // square distance from p to the second neighbouring domain
    double eps2h(const TSortedNode &Node, TVec p) const;
    // square distance from p to the body surface
    double h2(const TSortedNode &Node, TVec p) const;

    double streamfunction(const TSortedNode &node, TVec p, double* psi_gap) const;
};
