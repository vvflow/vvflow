#ifndef ELEMENTARY_H_
#define ELEMENTARY_H_

#include <math.h>
#include <iostream>
#include "list.h"
#include "shellscript.h"
using namespace std;

#define bugfix volatile

typedef void* pointer;
typedef char* pchar;
const double C_PI =		3.14159265358979323846;
const double C_2PI =	2.*C_PI;
const double C_1_2PI =	1./(2.*C_PI);
const double C_1_PI = 	1./C_PI;
const double C_2_PI = 	2./C_PI;


/******************* Vectors *******************/

inline double sign(double x) { return (x>0) ? 1 : ((x<0) ? -1 : 0); }
inline double sqr(double x) { return x*x; }
inline int sqr(int x) { return x*x; }
inline double max(double a, double b) { return(a>b)?a:b; }
inline double min(double a, double b) { return(a<b)?a:b; }
inline double max(double a, double b, double c) { return max(a, max(b, c)); }
inline double min(double a, double b, double c) { return min(a, min(b, c)); }
inline double max(double a, double b, double c, double d) { return max(a, max(b, max(c,d))); }
inline double min(double a, double b, double c, double d) { return min(a, min(b, min(c,d))); }
inline bool divisible(double dividend, double divisor, double precision) {
	return (fabs(dividend-int(dividend/divisor+0.5)*divisor) < precision);
	// divisible = кратно
	// dividend = делимое
	// divisor = делитель
	// precision = точность
	// возвращает true если первый аргумент кратен второму с заданной точностью
}
inline bool divisible(double dividend, double divisor)
{
	return divisible(dividend, divisor, divisor*0.01);
}

class TVec
{
	public:
		double rx, ry;

		TVec() {}
		TVec(double rx_, double ry_) {rx=rx_; ry=ry_;}

		double abs() const {return sqrt(rx*rx+ry*ry);}
		double abs2() const {return rx*rx+ry*ry;}
		bool iszero() const {return (fabs(rx)+fabs(ry) < 1E-10); }
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
		friend bool operator!= (const TVec &p1, const TVec &p2) { return ((p1.rx!=p2.rx)||(p1.ry!=p2.ry)); }

		friend double operator* (const TVec &p1, const TVec &p2) 	{ return p1.rx*p2.rx + p1.ry*p2.ry; }
		friend const TVec rotl(const TVec &p) { return TVec(-p.ry, p.rx); }
		//NB: double * rotl(vec) = (in terms of math) (e_z*double) \times vec
		friend const TVec operator- (const TVec &p) { return TVec(-p.rx, -p.ry); }
};

class TObj: public TVec
{
	public:
		double g;
		TVec v;
		double _1_eps; // eps = max (nearest dl on wall,  dist to 2nd nearest vortex)

		TObj() {}
		TObj(double rx_, double ry_, double g_):TVec(rx_, ry_) { g=g_; v.zero();}
		TObj(TVec r_, double g_){rx=r_.rx; ry=r_.ry; g=g_; v.zero();}

		void zero() { rx = ry = g = 0; v.zero(); }
		void init(double rx_, double ry_, double g_) {rx=rx_; ry=ry_; g=g_;}
		short sign() { return ::sign(g); }

		TObj& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }
		friend istream& operator>> (istream& is, TObj& p) 		{ return is >> p.rx >> p.ry >> p.g; }
		friend ostream& operator<< (ostream& os, const TObj& p) 	{ return os << p.rx << " \t" << p.ry << " \t" << p.g; }
		friend short sign(const TObj& p) { return ::sign(p.g); }
};


#endif

