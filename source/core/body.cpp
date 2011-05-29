#include "body.h"
#include "stdio.h"
#include "iostream"
#include "math.h"
using namespace std;

TBody::TBody(double (*sRotationV)(double Time),
				double sRotationAxisX, double sRotationAxisY)
{
	List = new vector<TObj>();
	AttachList = new vector<TAtt>();
	RotationV = sRotationV;
	RotationAxis = TVec(sRotationAxisX, sRotationAxisY);
	InsideIsValid = true;
	Angle = 0;
}

int TBody::LoadFromFile(const char* filename)
{
	if (!this) return -1;
	if ( !List ) return -1;
	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObj obj(0, 0, 0);
	while ( fscanf(fin, "%lf %lf", &obj.rx, &obj.ry)==2 )
	{
		List->push_back(obj);
	}

	fclose(fin);
	InsideIsValid = isInsideValid();
	return 0;
}

void TBody::Rotate(double angle)
{
	if (!this) return;

	const_for (List, obj)
	{
		TVec dr = *obj - RotationAxis;
		*obj = RotationAxis + dr*cos(angle) + rotl(dr)*sin(angle);
	}
	Angle += angle;
	UpdateAttach();
}

bool TBody::PointIsValid(TVec p)
{
	if (!this) return true;

	bool res = !InsideIsValid;

	for (auto i = List->begin(), j = List->end()-1; i<List->end(); j=i++)
	{
		if ((
			(i->ry < j->ry) && (i->ry < p.ry) && (p.ry <= j->ry) &&
			((j->ry - i->ry) * (p.rx - i->rx) > (j->rx - i->rx) * (p.ry - i->ry))
			) || (
			(i->ry > j->ry) && (i->ry > p.ry) && (p.ry >= j->ry) &&
			((j->ry - i->ry) * (p.rx - i->rx) < (j->rx - i->rx) * (p.ry - i->ry))
		)) res = !res;
	}

	return res;
}

double TBody::SurfaceLength()
{
	if (!this || !List->size()) return 0;
	double res=0;

	const_for (List, obj)
	{
		res += abs(*obj - *(obj+1));
	}

	res += abs(*List->begin() - *(List->end()-1));

	return res;
}

inline double atan2(const TVec &p)
{
	return atan2(p.ry, p.rx);
}

bool TBody::isInsideValid()
{
	if (!this) return true;

	auto min = List->begin();
	const_for (List, obj)
	{
		min = (obj->rx < min->rx) ? obj : min;
	}

	return ((atan2(*prev(min)-*min) - atan2(*next(min)-*min)) > 0);
}

void TBody::UpdateAttach()
{
	if (!this) return;
	AttachList->clear();
	TAtt att;

	const_for (List, obj)
	{
		TVec dr = *next(obj) - *obj;
		att = TVec(0.5*(*next(obj) + *obj));

		att.g = rotl(att-RotationAxis)*dr;
		att.q =     (att-RotationAxis)*dr;

		AttachList->push_back(att);
	}
}

/************************** HEAT LAYER ****************************************/

void TBody::CleanHeatLayer()
{
	if (!this) return;
	if (!HeatLayer) return;

	for (long i=0; i<List->size(); i++)
	{
		HeatLayer[i]=0;
	}
}

int* TBody::ObjectIsInHeatLayer(TObj &obj)
{
	return false;
}

