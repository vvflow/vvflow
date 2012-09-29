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
namespace bc{
BoundaryCondition bc(int i)
{
	switch (i)
	{
		case 'l': return slip;
		case 'n': return noslip;
		case 'z': return zero;
		case 's': return steady;
		case 'i': return inf_steady;
	}
	return slip;
}}

namespace hc{
HeatCondition hc(int i)
{
	switch (i)
	{
		case 'n': return neglect;
		case 'i': return isolate;
		case 't': return const_t;
		case 'w': return const_W;
	}
	return neglect;
}}

TBody::TBody(Space *sS)
{
	S = sS;
	List = new vector<TAtt>();
	HeatLayerList = new vector<TVec>();

	RotSpeed_link = NULL;
	RotSpeed_const = 0;
	MotSpeed_link = NULL;
	MotSpeed_const = TVec(0,0);

	Angle = deltaAngle = 0; Position = deltaPosition = TVec(0,0);
	g_dead = 0;
	Position = TVec(0,0);
	Friction = Friction_prev = TObj(0,0,0);
	Force_born = Force_dead = TObj(0,0,0);
	_surface = _area = 0;
	_com.zero();
	_moi_c = _moi_com = 0;
	kx = ky = ka = -1;
	density = 1;
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

void TBody::doRotationAndMotion()
{
	if (!this) return;

	doRotation();
	doMotion();
	doFillProperties();
	doUpdateSegments();
}

void TBody::doRotation()
{
	double angle_slae = RotationSpeed_slae * S->dt; //in doc \omega_? \Delta t
	double angle_solid = getRotation(S->Time) * S->dt; // in doc \omega \Delta t
	const_for (List, lobj)
	{
		TVec dr = lobj->corner - (Position + deltaPosition);
		lobj->corner = Position + deltaPosition + dr*cos(angle_slae) + rotl(dr)*sin(angle_slae);
	}
	Angle += angle_solid;
	deltaAngle += angle_slae - angle_solid;
	RotationSpeed_slae_prev = RotationSpeed_slae;
}

void TBody::doMotion()
{
	TVec delta_slae = MotionSpeed_slae * S->dt;
	TVec delta_solid = getMotion(S->Time) * S->dt;
	const_for (List, lobj)
	{
		lobj->corner += delta_slae;
	}
	Position += delta_solid;
	deltaPosition += delta_slae - delta_solid;
	MotionSpeed_slae_prev = MotionSpeed_slae;
}

void TBody::doUpdateSegments()
{
	if (!this) return;

	List->push_back(List->at(0));
	List->erase(List->end()-1);

	const_for (List, lobj)
	{
		lobj->dl = (lobj+1)->corner - lobj->corner;
		*lobj = 0.5*((lobj+1)->corner + lobj->corner);
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

