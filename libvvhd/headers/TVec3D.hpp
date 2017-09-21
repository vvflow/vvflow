#pragma once

#include "elementary.h"
#include "TVec.hpp"

class TVec3D
{
    public:
        TVec r;
        double o;

    public:
        TVec3D() :r()
    {o = 0.;}
        TVec3D(double _x, double _y, double _o) :r(_x, _y)
    {o = _o;}

        bool iszero() const {return (fabs(r.x)+fabs(r.y)+fabs(o) < 1E-10); }
        friend const TVec3D operator+ (const TVec3D &p1, const TVec3D &p2) { return TVec3D(p1.r.x+p2.r.x, p1.r.y+p2.r.y, p1.o+p2.o); }
        friend const TVec3D operator- (const TVec3D &p1, const TVec3D &p2) { return TVec3D(p1.r.x-p2.r.x, p1.r.y-p2.r.y, p1.o-p2.o); }
        friend const TVec3D operator* (const TVec3D &p, double c) { return TVec3D(c*p.r.x, c*p.r.y, c*p.o); }
        friend const TVec3D operator* (double c, const TVec3D &p) { return p*c; }
        friend std::istream& operator>> (std::istream& is, TVec3D& p) { return is >> p.r.x >> p.r.y >> p.o; }
        friend std::ostream& operator<< (std::ostream& os, const TVec3D& p) { return os << p.r.x << " \t" << p.r.y << " \t" << p.o; }
    public:
        operator std::string() const
        {
            char buf[64] = {0};
            if (!iszero())
                sprintf(buf, "%lg, %lg, %lg", r.x, r.y, o);
            return std::string(buf);
        }
};
