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
		double _1_eps; // eps = max (nearest dl/3 on wall,  dist to 2nd nearest vortex)

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

class TVec3D
{
	public:
		double x, y, o;

	public:
		TVec3D() {x = y = o = 0;}
		TVec3D(double _x, double _y, double _o) {x = _x; y = _y; o = _o;}
		friend istream& operator>> (istream& is, TVec3D& p) 		{ return is >> p.x >> p.y >> p.o; }
		friend ostream& operator<< (ostream& os, const TVec3D& p) 	{ return os << p.x << " \t" << p.y << " \t" << p.o; }
};

uint32_t lcm(uint32_t x, uint32_t y)
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
			if (result > (1<<31)/k) { /*overflow*/ fprintf(stderr, "overflow, %u %u %u %u\n", x, y, k, result ); return result; }
			result*=k;
			if (!xk) x/=k;
			if (!yk) y/=k;
			k = 2; continue;
		}
		k++;
	}
	return result;
}

class TTime
{
	private:
		// time = value/timescale
		int32_t value;
		uint32_t timescale;

	public:
		TTime()
			:v(value),ts(timescale)
			{value = timescale = 0;}
		TTime(int32_t _value, uint32_t _timescale)
			:v(value),ts(timescale)
			{value = _value; timescale=_timescale;}
		static TTime makeWithSeconds(double seconds, uint32_t preferredTimescale)
		{
			return TTime(int32_t(seconds*preferredTimescale), preferredTimescale);
		}
		static TTime makeWithSecondsDecimal(double seconds)
		{
			uint32_t newTS = 1;
			while ((int32_t(seconds*newTS)/double(newTS) != seconds) && (newTS<0x19999999)/*0xffffffff/10*/) newTS*= 10;
			while (!(int32_t(seconds*newTS)%10)) newTS/=10;
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
		const int32_t &v;
		const uint32_t &ts;
};

#endif

