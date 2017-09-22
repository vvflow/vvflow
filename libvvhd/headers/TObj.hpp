#pragma once

#include "TVec.hpp"

inline static
double sign(double x) { return (x>0) ? 1 : ((x<0) ? -1 : 0); }

class TObj
{
    public:
        TVec r;
        double g;
        TVec v;
        double _1_eps; // eps = max (nearest dl/3 on wall,  dist to 2nd nearest vortex)

    public:
        TObj():
            r(), g(),
            v(), _1_eps() {}
        TObj(double rx, double ry, double g):
            r(rx, ry), g(g),
            v(), _1_eps() {}
        TObj(TVec r, double g):
            r(r), g(g),
            v(), _1_eps() {}

        int sign() { return ::sign(g); }

        //TObj& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }
        friend std::istream& operator>> (std::istream& is, TObj& p) { return is >> p.r.x >> p.r.y >> p.g; }
        friend std::ostream& operator<< (std::ostream& os, const TObj& p) { return os << p.r.x << " \t" << p.r.y << " \t" << p.g; }
        friend short sign(const TObj& p) { return ::sign(p.g); }
};
