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
	List = new vector<TAtt>();
	HeatLayerList = new vector<TVec>();

	RotSpeed_link = NULL;
	RotSpeed_const = 0;
	MotSpeed_link = NULL;
	MotSpeed_const = TVec(0,0);

	Angle = 0; Position = TVec(0,0);
	g_dead = 0;
	Position = TVec(0,0);
	Force = TObj(0,0,0);
	_surface = _area = 0;
	_com.zero();
	_moi_c = _moi_com = 0;
}

TBody::~TBody()
{
	delete List;
	delete HeatLayerList;
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

void TBody::doRotation(double angle)
{
	if (!this) return;

	const_for (List, lobj)
	{
		TVec dr = lobj->corner - Position;
		lobj->corner = Position + dr*cos(angle) + rotl(dr)*sin(angle);
	}
	Angle += angle;
	doUpdateAttach();
	HeatLayerList->clear();
}

void TBody::doMotion(TVec delta)
{
	if (!this) return;

	const_for (List, lobj)
	{
		lobj->corner += delta;
	}
	Position += delta;
	doUpdateAttach();
	HeatLayerList->clear();
}

void TBody::doUpdateAttach()
{
	if (!this) return;

	const_for (List, lobj)
	{
		lobj->dl = (lobj+1)->corner - lobj->corner;
		*lobj = 0.5*((lobj+1)->corner + lobj->corner);

		//TODO what to do with attaches?
	}
}

TAtt* TBody::isPointInvalid(TVec p)
{
	if (!this) return NULL;
	return isPointInContour(p, List);
}

TAtt* TBody::isPointInHeatLayer(TVec p)
{
	if (!this) return NULL;
	if (!HeatLayerList->size())
	{
		TVec tmp(0, 0);
		const_for(List, lobj)
		{
			tmp = *lobj + rotl(lobj->dl);
			HeatLayerList->push_back(tmp);
		}
	}

	return isPointInContour(p, HeatLayerList);
}

template <class T> TVec corner(T *lobj) {return *lobj;}
template <> TVec corner<TAtt>(TAtt *lobj) {return lobj->corner;}
template <class T>
TAtt* TBody::isPointInContour(TVec p, vector<T> *list)
{
	if (!this) return NULL;

	bool res = isInsideValid();
	TAtt *nearest = NULL;
	double nearest_dr2 = 1E10;

	for (auto i = list->begin(), j = list->end()-1; i<list->end(); j=i++)
	{
		TVec vi = corner<T>(i);
		TVec vj = corner<T>(j);

		if ((
			(vi.ry < vj.ry) && (vi.ry < p.ry) && (p.ry <= vj.ry) &&
			((vj.ry - vi.ry) * (p.rx - vi.rx) > (vj.rx - vi.rx) * (p.ry - vi.ry))
			) || (
			(vi.ry > vj.ry) && (vi.ry > p.ry) && (p.ry >= vj.ry) &&
			((vj.ry - vi.ry) * (p.rx - vi.rx) < (vj.rx - vi.rx) * (p.ry - vi.ry))
		)) res = !res;
	}

	if (res)
	const_for(List, latt)
	{
		if ((*latt-p).abs2()<nearest_dr2)
		{
			nearest = latt;
			nearest_dr2 = (*latt-p).abs2();
		}
	}

	return nearest;
}

void TBody::doFillProperties()
{
	_surface = _area = _moi_c = _moi_com = 0;
	_com.zero();
	const_for (List, latt)
	{
		_surface+= latt->dl.abs();
		_area+= latt->ry*latt->dl.rx;
		_com+= latt->abs2() * rotl(latt->dl);
		_moi_com-= latt->abs2() * latt->dl * rotl(*latt);
	}
	_com = 0.5*_com/_area - Position;
	_moi_com = 0.25*_moi_com - _area*(_com+Position).abs2();
	_moi_c = _moi_com + _area*_com.abs2();
}

inline double atan2(const TVec &p)
{
	return atan2(p.ry, p.rx);
}

