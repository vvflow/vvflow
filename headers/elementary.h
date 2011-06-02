#ifndef ELEMENTARY_H_
#define ELEMENTARY_H_

#include <math.h>
#include <iostream>
#include "list.h"
using namespace std;


const double C_PI =		3.14159265358979323846;
const double C_2PI =	2.*C_PI;
const double C_1_2PI =	1./(2.*C_PI);
const double C_1_PI = 	1./C_PI;
const double C_2_PI = 	2./C_PI;


/******************* Vectors *******************/

inline double sign(double x) { return (x>0) ? 1 : ((x<0) ? -1 : 0); }
inline double sqr(double x) { return x*x; }

class TVec
{
	public:
		double rx, ry;

		TVec() {}
		TVec(double rx_, double ry_) {rx=rx_; ry=ry_;}

		double abs() {return sqrt(rx*rx+ry*ry);}
		friend double abs(const TVec& p) {return sqrt(p.rx*p.rx+p.ry*p.ry);}
		double abs2() {return rx*rx+ry*ry;}
		friend double abs2(const TVec& p) {return p.rx*p.rx+p.ry*p.ry;}
		bool iszero() {return (fabs(rx)+fabs(ry) < 1E-10); }
		void zero() { rx = ry = 0; }
		void init(double rx_, double ry_) {rx=rx_; ry=ry_;}

		friend istream& operator>> (istream& is, TVec& p) 		{ return is >> p.rx >> p.ry; }
		friend ostream& operator<< (ostream& os, const TVec& p) 	{ return os << p.rx << " \t" << p.ry; }

		friend const TVec operator* (const double c, const TVec &p) { return TVec(p.rx*c, p.ry*c); }
		friend const TVec operator* (const TVec &p, const double c) { return TVec(p.rx*c, p.ry*c); }
		friend const TVec operator/ (const TVec &p, const double c) { return p*(1/c); }
		friend const TVec operator+ (const TVec &p1, const TVec &p2) { return TVec(p1.rx+p2.rx, p1.ry+p2.ry); }
		friend const TVec operator- (const TVec &p1, const TVec &p2) { return TVec(p1.rx-p2.rx, p1.ry-p2.ry); }

		friend void operator+= (TVec &p1, const TVec& p2) { p1.rx+= p2.rx; p1.ry+= p2.ry; }
		friend void operator-= (TVec &p1, const TVec& p2) { p1.rx-= p2.rx; p1.ry-= p2.ry; }
		friend void operator*= (TVec &p, const double c) { p.rx*= c; p.ry*= c; }
		friend void operator/= (TVec &p, const double c) { p*= (1/c); }

		friend double operator* (const TVec &p1, const TVec &p2) 	{ return p1.rx*p2.rx + p1.ry*p2.ry; }
		friend const TVec rotl(const TVec &p) { return TVec(-p.ry, p.rx); }
		friend const TVec operator- (const TVec &p) { return TVec(-p.rx, -p.ry); }
};

class TObj: public TVec
{
	public:
		TVec v;
		double g;
		double _1_eps;

		TObj() {}
		TObj(double rx_, double ry_, double g_):TVec(rx_, ry_) { g=g_; v.zero();}

		void zero() { rx = ry = g = 0; v.zero(); }
		void init(double rx_, double ry_, double g_) {rx=rx_; ry=ry_; g=g_;}

		TObj& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }
		friend istream& operator>> (istream& is, TObj& p) 		{ return is >> p.rx >> p.ry >> p.g; }
		friend ostream& operator<< (ostream& os, const TObj& p) 	{ return os << p.rx << " \t" << p.ry << " \t" << p.g; }
		friend short sign(const TObj& p) { return (p.g>0)? 1:-1; }
};


#endif

