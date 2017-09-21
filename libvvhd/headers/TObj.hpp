#pragma once

#include "elementary.h"
#include "TVec.hpp"

class TObj
{
    public:
        TVec r;
        double g;
        TVec v;
        double _1_eps; // eps = max (nearest dl/3 on wall,  dist to 2nd nearest vortex)

    public:
        TObj() :r(), v()
    {g = _1_eps = 0.;}
        TObj(double rx_, double ry_, double g_) :r(rx_, ry_), v()
    { g=g_; _1_eps = 0.;}
        TObj(TVec r_, double g_) :r(r_), v()
    { g=g_; _1_eps = 0.;}

        int sign() { return ::sign(g); }

        //TObj& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }
        friend std::istream& operator>> (std::istream& is, TObj& p) { return is >> p.r.x >> p.r.y >> p.g; }
        friend std::ostream& operator<< (std::ostream& os, const TObj& p) { return os << p.r.x << " \t" << p.r.y << " \t" << p.g; }
        friend short sign(const TObj& p) { return ::sign(p.g); }
};
