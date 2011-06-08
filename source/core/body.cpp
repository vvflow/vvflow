#include "body.h"
#include <stdio.h>
#include <iostream>
#include <limits>
#include <math.h>
using namespace std;

TAtt::TAtt(TBody *body) {this->body = body;}

TBody::TBody()
{
	List = new vector<TObj>();
	AttachList = new vector<TAtt>();
	RotSpeed_link = NULL;
	RotAxis = TVec(0,0);
	InsideIsValid = true;
	Angle = 0;
	Position = TVec(0,0);
}

int TBody::LoadFromFile(const char* filename)
{
	if (!this) return -1;
	if ( !List ) return -1;
	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObj obj(0, 0, 0);
	TAtt att(this); att.zero();
	while ( fscanf(fin, "%lf %lf %d", &obj.rx, &obj.ry, &att.bc)==3 )
	{
		List->push_back(obj);
		AttachList->push_back(att);
	}

	fclose(fin);
	InsideIsValid = isInsideValid();
	UpdateAttach();
	return 0;
}

void TBody::Rotate(double angle)
{
	if (!this) return;

	const_for (List, obj)
	{
		TVec dr = *obj - RotAxis;
		*obj = RotAxis + dr*cos(angle) + rotl(dr)*sin(angle);
	}
	Angle += angle;
	UpdateAttach();
}

TAtt* TBody::PointIsInvalid(TVec p)
{
	if (!this) return NULL;

	bool res = InsideIsValid;
	TAtt *nearest = NULL;
	double nearest_dr2 = 1E10;

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

	if (res)
	const_for(AttachList, latt)
	{
		if ((*latt-p).abs2()<nearest_dr2)
		{
			nearest = latt;
			nearest_dr2 = (*latt-p).abs2();
		}
	}

	return nearest;
}

double TBody::SurfaceLength()
{
	if (!this || !List->size()) return 0;
	double res=0;

	const_for (List, obj)
	{
		res += (*obj - *(obj+1)).abs();
	}

	res += (*List->begin() - *(List->end()-1)).abs();

	return res;
}

void TBody::SetRotation(double (*sRotSpeed)(double time), TVec sRotAxis)
{
	RotSpeed_link = sRotSpeed;
	RotAxis = sRotAxis;
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

	const_for (List, lobj)
	{
		TAtt &att = *this->att(lobj);
		att.dl = *next(lobj) - *lobj;
		att = TVec(0.5*(*next(lobj) + *lobj));

		att.g = rotl(att-RotAxis)*att.dl;
		att.q =     (att-RotAxis)*att.dl;
	}
}

void TBody::calc_pressure()
{
	double res =0;
	const_for(AttachList, latt)
	{
		res+= latt->gsum;
		latt->pres = res;
	}
}

void TBody::zero_variables()
{
	const_for(AttachList, latt)
	{
		latt->pres = latt->gsum = latt->fric = 0;
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

