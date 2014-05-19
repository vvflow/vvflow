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

TBody::TBody(Space *sS):
	SpeedX(),
	SpeedY(),
	SpeedO()
{
	S = sS;
	List = new vector<TAtt>();
	HeatLayerList = new vector<TVec>();

	pos = dPos = TVec3D(0., 0., 0.);
	g_dead = 0;
	Friction = Friction_prev = TVec3D(0,0,0);
	Force_born = Force_dead = TVec3D(0,0,0);
	_surface = _area = 0;
	_com = TVec(0., 0.);
	_moi_c = _moi_com = 0;
	k = TVec3D(-1., -1., -1.);
	damping = TVec3D(0,0,0);
	density = 1.;
}

TBody::~TBody()
{
	delete List;
	delete HeatLayerList;
}

TVec3D TBody::getSpeed() const
{
	return TVec3D(
	              SpeedX.getValue(S->Time),
	              SpeedY.getValue(S->Time),
	              SpeedO.getValue(S->Time)
	             );
}

void TBody::doRotationAndMotion()
{
	if (!this) return;

	doRotation();
	doMotion();
	doUpdateSegments();
	doFillProperties();
}

void TBody::doRotation()
{
	double angle_slae = Speed_slae.o * S->dt; //in doc \omega_? \Delta t
	double angle_solid = getSpeed().o * S->dt; //in doc \omega \Delta t
	const_for (List, lobj)
	{
		TVec dr = lobj->corner - (pos.r + dPos.r);
		lobj->corner = pos.r + dPos.r + dr*cos(angle_slae) + rotl(dr)*sin(angle_slae);
	}
	pos.o += angle_solid;
	dPos.o += angle_slae - angle_solid;
	Speed_slae_prev.o = Speed_slae.o;
}

void TBody::doMotion()
{
	TVec delta_slae = Speed_slae.r * S->dt;
	TVec delta_solid = getSpeed().r * S->dt;
	const_for (List, lobj)
	{
		lobj->corner += delta_slae;
	}
	pos.r += delta_solid;
	dPos.r += delta_slae - delta_solid;
	Speed_slae_prev.r = Speed_slae.r;
}

void TBody::doUpdateSegments()
{
	if (!this) return;

	List->push_back(List->at(0));
	List->erase(List->end()-1);

	const_for (List, lobj)
	{
		lobj->dl = (lobj+1)->corner - lobj->corner;
		lobj->_1_eps = 3.0/lobj->dl.abs();
		lobj->r = 0.5*((lobj+1)->corner + lobj->corner);
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
			tmp = lobj->r + rotl(lobj->dl);
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
			(vi.y < vj.y) && (vi.y < p.y) && (p.y <= vj.y) &&
			((vj.y - vi.y) * (p.x - vi.x) > (vj.x - vi.x) * (p.y - vi.y))
			) || (
			(vi.y > vj.y) && (vi.y > p.y) && (p.y >= vj.y) &&
			((vj.y - vi.y) * (p.x - vi.x) < (vj.x - vi.x) * (p.y - vi.y))
		)) res = !res;
	}

	if (res)
	const_for(List, latt)
	{
		if ((latt->r - p).abs2()<nearest_dr2)
		{
			nearest = latt;
			nearest_dr2 = (latt->r - p).abs2();
		}
	}

	return nearest;
}

void TBody::doFillProperties()
{
	_surface = _area = _moi_c = _moi_com = 0;
	_com = TVec(0., 0.);
	double _12moi_0 = 0;
	const_for (List, latt)
	{
		_surface+= latt->dl.abs();
		_area+= latt->r.y*latt->dl.x;
		_com-= latt->r * (rotl(latt->corner) * (latt+1)->corner);
		_12moi_0 -= (latt->corner.abs2() + latt->corner*(latt+1)->corner + (latt+1)->corner.abs2())
		            *  (rotl(latt->corner) * (latt+1)->corner);
	}
	_com = _com/(3*_area);
	_moi_com = _12moi_0/12. - _area*_com.abs2();
	_moi_c = _moi_com + _area*(pos.r + dPos.r - _com).abs2();
}

inline double atan2(const TVec &p)
{
	return atan2(p.y, p.x);
}

