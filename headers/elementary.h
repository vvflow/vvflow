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
		Vector v;
		double g;

		Object() {}
		Object(double rx_, double ry_, double g_) {rx=rx_; ry=ry_; g=g_; v.zero();}

		void zero() { rx = ry = g = 0; v.zero(); }
		void init(double rx_, double ry_, double g_) {rx=rx_; ry=ry_; g=g_;}

		friend istream& operator>> (istream& is, Object& p) 		{ return is >> p.rx >> p.ry >> p.g; }
		friend ostream& operator<< (ostream& os, const Object& p) 	{ return os << p.rx << " \t" << p.ry << " \t" << p.g; }
};

typedef Object TObject;


#endif

