#pragma once

#include "TVec.hpp"

class TVec3D
{
    public:
        TVec r;
        double o;

    public:
        TVec3D():
            r(), o() {}
        TVec3D(double x, double y, double o):
            r(x, y), o(o) {}

        bool iszero() const {return (fabs(r.x)+fabs(r.y)+fabs(o) < 1E-10); }
        friend TVec3D operator+ (const TVec3D &p1, const TVec3D &p2) { return TVec3D(p1.r.x+p2.r.x, p1.r.y+p2.r.y, p1.o+p2.o); }
        friend TVec3D operator- (const TVec3D &p1, const TVec3D &p2) { return TVec3D(p1.r.x-p2.r.x, p1.r.y-p2.r.y, p1.o-p2.o); }
        friend TVec3D operator* (const TVec3D &p, double c) { return TVec3D(c*p.r.x, c*p.r.y, c*p.o); }
        friend TVec3D operator* (double c, const TVec3D &p) { return p*c; }
};
