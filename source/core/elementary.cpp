#include <math.h>
#include "elementary.h"

double abs(Vector v)
{
	return sqrt(v.x*v.x + v.y*v.y);
}

double abs2(Vector v)
{
	return v.y*v.y + v.x*v.x;
}



double operator * (const Vector &v1, const Vector &v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

double operator & (const Vector &v1, const Vector &v2) //Vector multiplicaton
{
	return v1.x*v2.y-v2.x*v1.y;
}



Point operator + (const Point &p1, const Point &p2)
{
	Point res;
	res.x = p1.x+p2.x;
	res.y = p1.y+p2.y;
	return res;
}

Point operator - (const Point &p1, const Point &p2)
{
	Point res;
	res.x = p1.x-p2.x;
	res.y = p1.y-p2.y;
	return res;
} 



void operator +=(Point &p1,const Point &p2)
{
	p1.x+= p2.x;
	p1.y+= p2.y;
}

void operator -=(Point &p1,const Point &p2)
{
	p1.x-= p2.x;
	p1.y-= p2.y;
}

void operator *=(Point &p,const double a)
{
	p.x*= a;
	p.y*= a;
}

void operator /=(Point &p,const double a)
{
	double a1 = 1/a;
	p.x*= a1;
	p.y*= a1;
}

   

Point operator - (const Point &p)
{
	Point res;
	res.x = -p.x;
	res.y = -p.y;
	return res;
}	

Point operator + (const Point &p)
{
	return p;
}

Point operator * (const Point &p, const double &a)
{
	Point res;
	res.x = a * p.x;
	res.y = a * p.y;
	return res;
}

Point operator * (const double& a, const Point &p)
{
	Point res;
	res.x = a * p.x;
	res.y = a * p.y;
	return res;
}

Point operator / (const Point &p, const double &a)
{
	Point res;
	double a1 = 1/a;
	res.x = p.x * a1;
	res.y = p.y * a1;
	return res;
}



std::ostream& operator << (std::ostream& os, const Point &p)
{
	return os << p.x << "\t" << p.y;
}

std::ostream& operator<< (std::ostream& os, const TVortex &v)
{
	return os << v.rx << "\t" << v.ry << "\t" << v.g;
}



Point rotl(const Point &p)
{
	Point res;
	res.x = -p.y;
	res.y = p.x;
	return res;
}

Point rotr(const Point &p)
{
	Point res;
	res.x = p.y;
	res.y = -p.x;
	return res;
}
