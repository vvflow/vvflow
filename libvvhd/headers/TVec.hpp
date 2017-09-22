#pragma once

#include <iostream>
#include <cmath>

class TVec
{
    public:
        double x, y;

    public:
        TVec()
        {x = y = 0.;}
        TVec(double _x, double _y)
        {x=_x; y=_y;}

        double abs() const {return sqrt(x*x+y*y);}
        double abs2() const {return x*x+y*y;}
        bool iszero() const {return (fabs(x)+fabs(y) < 1E-10); }
        void set(double rx, double ry) {x=rx; y=ry;}

        // friend std::istream& operator>> (std::istream& is, TVec& p) 		{ return is >> p.x >> p.y; }
        // friend std::ostream& operator<< (std::ostream& os, const TVec& p) 	{ return os << p.x << " \t" << p.y; }

        friend const TVec operator* (const double c, const TVec &p) { return TVec(p.x*c, p.y*c); }
        friend const TVec operator* (const TVec &p, const double c) { return TVec(p.x*c, p.y*c); }
        friend const TVec operator/ (const TVec &p, const double c) { return p*(1./c); }
        friend const TVec operator+ (const TVec &p1, const TVec &p2) { return TVec(p1.x+p2.x, p1.y+p2.y); }
        friend const TVec operator- (const TVec &p1, const TVec &p2) { return TVec(p1.x-p2.x, p1.y-p2.y); }

        friend void operator+= (TVec &p1, const TVec& p2) { p1.x+= p2.x; p1.y+= p2.y; }
        friend void operator-= (TVec &p1, const TVec& p2) { p1.x-= p2.x; p1.y-= p2.y; }
        friend void operator*= (TVec &p, const double c) { p.x*= c; p.y*= c; }
        friend void operator/= (TVec &p, const double c) { p*= (1./c); }
        friend bool operator!= (const TVec &p1, const TVec &p2) { return ((p1.x!=p2.x)||(p1.y!=p2.y)); }

        friend double operator* (const TVec &p1, const TVec &p2) 	{ return p1.x*p2.x + p1.y*p2.y; }
        friend const TVec rotl(const TVec &p) { return TVec(-p.y, p.x); }
        //NB: double * rotl(vec) = (in terms of math) (e_z*double) \times vec
        friend const TVec operator- (const TVec &p) { return TVec(-p.x, -p.y); }
    public:
        operator std::string() const
        {
            char buf[64] = {0};
            if (!iszero())
                sprintf(buf, "%lg, %lg", x, y);
            return std::string(buf);
        }
};
