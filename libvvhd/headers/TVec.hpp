#pragma once

#include <iostream>
#include <cmath>

class TVec
{
    public:
        double x, y;

    public:
        TVec():
            x(), y() {}
        TVec(double x, double y):
            x(x), y(y) {}

        double abs() const {return sqrt(x*x+y*y);}
        double abs2() const {return x*x+y*y;}
        bool iszero() const {return (fabs(x)+fabs(y) < 1E-10); }
        void set(double rx, double ry) {x=rx; y=ry;}

        friend TVec operator* (const double c, const TVec &p)  { return TVec(p.x*c, p.y*c); }
        friend TVec operator* (const TVec &p,  const double c) { return TVec(p.x*c, p.y*c); }
        friend TVec operator/ (const TVec &p,  const double c) { return p*(1./c); }
        friend TVec operator+ (const TVec &p1, const TVec &p2) { return TVec(p1.x+p2.x, p1.y+p2.y); }
        friend TVec operator- (const TVec &p1, const TVec &p2) { return TVec(p1.x-p2.x, p1.y-p2.y); }
        friend TVec operator- (const TVec &p)                  { return TVec(-p.x, -p.y); }

        friend void operator+= (TVec &p1, const TVec& p2) { p1.x+= p2.x; p1.y+= p2.y; }
        friend void operator-= (TVec &p1, const TVec& p2) { p1.x-= p2.x; p1.y-= p2.y; }
        friend void operator*= (TVec &p, const double c) { p.x*= c; p.y*= c; }
        friend void operator/= (TVec &p, const double c) { p*= (1./c); }
        friend bool operator!= (const TVec &p1, const TVec &p2) { return ((p1.x!=p2.x)||(p1.y!=p2.y)); }

        friend double operator* (const TVec &p1, const TVec &p2)    { return p1.x*p2.x + p1.y*p2.y; }
        friend TVec rotl(const TVec &p) { return TVec(-p.y, p.x); }
        //NB: double * rotl(vec) = (in terms of math) (e_z*double) \times vec
};
