#ifndef ELEMENTARY_H_
#define ELEMENTARY_H_

#include <math.h>
#include <iostream>
using namespace std;

const double C_PI =		3.14159265358979323846;
const double C_2PI =	2.*C_PI;
const double C_1_2PI =	1./(2.*C_PI);
const double C_1_PI = 	1./C_PI;
const double C_2_PI = 	2./C_PI;


/******************* Vectors *******************/

class Vector
{
	public:
		double rx, ry;

		Vector() {}
		Vector(double rx_, double ry_) {rx=rx_; ry=ry_;}

		double abs() {return sqrt(rx*rx+ry*ry);}
		double abs2() {return rx*rx+ry*ry;}
		void zero() { rx = ry = 0; }
		void init(double rx_, double ry_) {rx=rx_; ry=ry_;}

		friend istream& operator>> (istream& is, Vector& p) 		{ return is >> p.rx >> p.ry; }
		friend ostream& operator<< (ostream& os, const Vector& p) 	{ return os << p.rx << " \t" << p.ry; }

		friend const Vector operator* (const double c, const Vector &p) { return Vector(p.rx*c, p.ry*c); }
		friend const Vector operator* (const Vector &p, const double c) { return Vector(p.rx*c, p.ry*c); }
		friend const Vector operator/ (const Vector &p, const double c) { return p*(1/c); }
		friend const Vector operator+ (const Vector &p1, const Vector &p2) { return Vector(p1.rx+p2.rx, p1.ry+p2.ry); }
		friend const Vector operator- (const Vector &p1, const Vector &p2) { return Vector(p1.rx-p2.rx, p1.ry-p2.ry); }

		friend void operator+= (Vector &p1, const Vector& p2) { p1.rx+= p2.rx; p1.ry+= p2.ry; }
		friend void operator-= (Vector &p1, const Vector& p2) { p1.rx-= p2.rx; p1.ry-= p2.ry; }
		friend void operator*= (Vector &p, const double c) { p.rx*= c; p.ry*= c; }
		friend void operator/= (Vector &p, const double c) { p*= (1/c); }

		friend double operator* (const Vector &p1, const Vector &p2) 	{ return p1.rx*p2.rx + p1.ry*p2.ry; }
		friend const Vector rotl(const Vector &p) { return Vector(-p.ry, p.rx); }
		friend const Vector operator- (const Vector &p) { return Vector(-p.rx, -p.ry); }
};

class Object: public Vector
{
	public:
		double vx, vy;
		double g;

		Object() {}
		Object(double rx_, double ry_, double g_) {rx=rx_; ry=ry_; g=g_; vx=vy=0;}

		void zero() { rx = ry = vx = vy = g = 0; }
		void init(double rx_, double ry_, double g_) {rx=rx_; ry=ry_; g=g_;}

		friend istream& operator>> (istream& is, Object& p) 		{ return is >> p.rx >> p.ry >> p.g; }
		friend ostream& operator<< (ostream& os, const Object& p) 	{ return os << p.rx << " \t" << p.ry << " \t" << p.g; }
};

/********************* Vortex ********************/

/*struct vortex
{
	double rx, ry;
	double vx, vy;
	//double vxtmp, vytmp;
	double g;
	//char flag;
	// flags:
	//	1 - to be removed;
	//	2 - to be merged;
};*/
typedef Object TVortex;
typedef Object TObject;

inline void InitObject(TObject &v, double x, double y, double g)
{ v.rx = x; v.ry = y; v.g = g; v.vx = v.vy = 0; }

//inline void ZeroObject(TObject &v)
//{ v.zero(); }

//#define InitVortex(v, sx, sy, sg) v.rx=sx; v.ry=sy; v.vx=v.vy=0; v.g=sg; //v.flag=0;
//#define ZeroVortex(v) InitVortex(v, 0, 0, 0);

//std::ostream& operator<< (std::ostream& os, const TVortex &v);

#endif

