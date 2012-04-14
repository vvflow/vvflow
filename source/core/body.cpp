#include "body.h"
#include <stdio.h>
#include <iostream>
#include <limits>
#include <math.h>
using namespace std;

/*TAtt::TAtt(TBody *body, int eq_no)
{
	this->body = body;
	this->eq_no = eq_no;
}*/

TBody::TBody(Space *sS)
{
	S = sS;
	List = new vector<TObj>();
	HeatLayerList = new vector<TObj>();
	AttachList = new vector<TAtt>();
	RotSpeed_link = NULL;
	RotSpeed_const = 0;
	RotAxis = TVec(0,0);
	InsideIsValid = true;
	Angle = 0;
	g_dead = 0;
	Position = TVec(0,0);
	Force = TObj(0,0,0);
	area = 0;
	com.zero();
	moi_c = moi_com = 0;
}

TBody::~TBody()
{
	delete List;
	delete HeatLayerList;
	delete AttachList;
}

/*int TBody::LoadFromFile(const char* filename, int start_eq_no)
{
	if (!this) return -1;
	if ( !List ) return -1;
	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObj obj(0, 0, 0);
	TAtt att(this, start_eq_no); att.zero();
	while ( fscanf(fin, "%lf %lf %d", &obj.rx, &obj.ry, &att.bc)==3 )
	{
		List->push_back(obj);
		AttachList->push_back(att);
		att.eq_no++;
	}

	fclose(fin);
	InsideIsValid = isInsideValid();
	UpdateAttach();
	return 0;
}*/

void TBody::setRotation(double (*sRotSpeed)(double time), double sRotSpeed_const)
{
	RotSpeed_link = sRotSpeed;
	RotSpeed_const = sRotSpeed_const;
}

void TBody::setMotion(TVec (*sMotSpeed)(double time), TVec sMotSpeed_const)
{
	MotSpeed_link = sMotSpeed;
	MotSpeed_const = sMotSpeed_const;
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
	HeatLayerList->clear();
}

TAtt* TBody::PointIsInvalid(TVec p)
{
	return PointIsInContour(p, List);
}

TAtt* TBody::PointIsInHeatLayer(TVec p)
{
	if (!this) return NULL;
	if (!HeatLayerList->size())
	{
		TObj tmp(0, 0, 0);
		const_for(List, lobj)
		{
			tmp = *lobj + rotl(att(lobj)->dl);
			HeatLayerList->push_back(tmp);
		}
	}

	return PointIsInContour(p, HeatLayerList);
}

TAtt* TBody::PointIsInContour(TVec p, vector<TObj> *list)
{
	if (!this) return NULL;

	bool res = InsideIsValid;
	TAtt *nearest = NULL;
	double nearest_dr2 = 1E10;

	for (auto i = list->begin(), j = list->end()-1; i<list->end(); j=i++)
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

void TBody::FillProperties()
{
	_surface = _area = _moi_c = _moi_com = 0;
	_com.zero();
	const_for (AttachList, latt)
	{
		_surface+= latt->dl.abs();
		_area+= latt->ry*latt->dl.rx;
		_com+= latt->abs2() * rotl(latt->dl);
		_moi_com-= latt->abs2() * latt->dl * rotl(*latt);
	}
	_com = 0.5*com/area - Position;
	_moi_com = 0.25*_moi_com - _area*(_com+Position).abs2();
	_moi_c = _moi_com + _area*com.abs2();
}

inline double atan2(const TVec &p)
{
	return atan2(p.ry, p.rx);
}

bool TBody::isInsideValid()
{
	if (!this) return true;
	if (!area) { FillProperties(); }
	return (area<=0);
}

void TBody::UpdateAttach()
{
	if (!this) return;

	const_for (List, lobj)
	{
		TAtt &att = *this->att(lobj);
		att.dl = *next(lobj) - *lobj;
		att = TVec(0.5*(*next(lobj) + *lobj));

		att.g = -rotl(att-RotAxis)*att.dl;
		att.q =      (att-RotAxis)*att.dl;
	}
}

/************************** HEAT LAYER ****************************************/

/*void TBody::CleanHeatLayer()
{
	if (!this) return;
	if (!HeatLayer) return;

	for (size_t i=0; i<List->size(); i++)
	{
		HeatLayer[i]=0;
	}
}*/

/*int* TBody::ObjectIsInHeatLayer(TObj &obj)
{
	return false;
}*/

