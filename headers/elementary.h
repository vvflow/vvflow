#ifndef ELEMENTARY_H_
#define ELEMENTARY_H_

#include <math.h>
#include <iostream>
#include "list.h"
#include "shellscript.h"
#include "stdint.h"
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

		friend istream& operator>> (istream& is, TVec& p) 		{ return is >> p.x >> p.y; }
		friend ostream& operator<< (ostream& os, const TVec& p) 	{ return os << p.x << " \t" << p.y; }

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
};

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
		friend istream& operator>> (istream& is, TObj& p) { return is >> p.r.x >> p.r.y >> p.g; }
		friend ostream& operator<< (ostream& os, const TObj& p) { return os << p.r.x << " \t" << p.r.y << " \t" << p.g; }
		friend short sign(const TObj& p) { return ::sign(p.g); }
};

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
		friend istream& operator>> (istream& is, TVec3D& p) { return is >> p.r.x >> p.r.y >> p.o; }
		friend ostream& operator<< (ostream& os, const TVec3D& p) { return os << p.r.x << " \t" << p.r.y << " \t" << p.o; }
};

class TTime
{
	public:
		// time = value/timescale
		int32_t value;
		uint32_t timescale;

	public:
		TTime()
			{value = timescale = 0;}
		TTime(int32_t _value, uint32_t _timescale)
			{value = _value; timescale=_timescale;}
		static TTime makeWithSeconds(double seconds, uint32_t preferredTimescale)
		{
			return TTime(int32_t(seconds*preferredTimescale), preferredTimescale);
		}
		static TTime makeWithSecondsDecimal(double seconds)
		{
			uint32_t newTS = 1;
			while ((int32_t(seconds*newTS)/double(newTS) != seconds) && (newTS<0x19999999)/*0xffffffff/10*/)
				{ newTS*= 10; }
			while (!(int32_t(seconds*newTS)%10) && (newTS > 1))
				{ newTS/= 10; }
			//fprintf(stderr, "%lf sec -> %d / %u\n", seconds, int32_t(seconds*newTS), newTS);
			return TTime(int32_t(seconds*newTS), newTS);
		}
		static TTime add(TTime time1, TTime time2)
		{
			uint32_t newTS = lcm(time1.timescale, time2.timescale);
			return TTime(
			              time1.value*newTS/time1.timescale
			            + time2.value*newTS/time2.timescale
			            , newTS);
		}
		bool divisibleBy(TTime divisor)
		{
			// divisible = кратно
			// dividend = делимое
			// divisor = делитель
			uint32_t lcm_ts = lcm(timescale, divisor.timescale);
			return !((int64_t(value)*lcm_ts/timescale) % (int64_t(divisor.value)*lcm_ts/divisor.timescale));
		}

	public:
		operator double() const {return double(value)/double(timescale);}

	private:
		static uint32_t lcm(uint32_t x, uint32_t y)
		{
			if (x == y) return x;
			if (!x || !y) return 1;
			uint32_t result = 1;
			uint32_t k = 2;
			while ((x!=1) || (y!=1))
			{
				bool kIsPrime = true;
				for (int d = 2; d<k/2; d++) { if (!k%d) {kIsPrime = false; break;}}
				if (!kIsPrime) {k++; continue;}
				int xk(x%k), yk(y%k);
				if (!xk || !yk)
				{
					if (result > (1<<31)/k) { /*overflow*/ return result; }
					result*=k;
					if (!xk) x/=k;
					if (!yk) y/=k;
					k = 2; continue;
				}
				k++;
			}
			return result;
		}
};

#endif

