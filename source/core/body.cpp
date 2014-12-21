#include "space.h"
#include "body.h"
#include <stdio.h>
#include <iostream>
#include <limits>
#include <math.h>
using namespace std;

TBody::TBody(Space *space):
	speed_x(),
	speed_y(),
	speed_o(),
	alist()
{
	S = space;

	holder = dpos = TVec3D(0., 0., 0.);
	g_dead = 0;
	friction = friction_prev = TVec3D(0,0,0);
	force_born = force_dead = TVec3D(0,0,0);
	force_hydro = force_holder = TVec3D(0,0,0);
	_surface = _area = 0;
	_com = TVec(0., 0.);
	_moi_c = _moi_com = 0;
	kspring = TVec3D(-1., -1., -1.);
	damping = TVec3D(0., 0., 0.);
	density = 1.;
	special_segment_no = 0;
	boundary_condition = bc_t::steady;
	heat_condition = hc_t::neglect;
}

int TBody::get_index() const
{
	int i = 0;
	for (auto lbody: S->BodyList)
	{
		if (lbody.get() == this) return i;
		i++;
	}
	return -1;
}

std::string TBody::get_name() const
{
	char name[8];
	sprintf(name, "body%02d", get_index());
	return std::string(name);
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
	doRotation();
	doMotion();
	doUpdateSegments();
	doFillProperties();
}

void TBody::doRotation()
{
	auto root_body = this->root_body.lock();
	double angle_root = root_body ?
	                  S->dt * root_body->Speed_slae.o
	                  : 0;
	const double angle_slae = Speed_slae.o * S->dt; //in doc \omega_? \Delta t
	const double angle_solid = getSpeed().o * S->dt; //in doc \omega \Delta t
	for (auto& att: List)
	{
		TVec dr = att.corner - get_axis();
		att.corner = get_axis() + dr*cos(angle_slae) + rotl(dr)*sin(angle_slae);
	}
	holder.o += angle_solid + angle_root;
	dpos.o += angle_slae - angle_solid - angle_root;
	speed_slae_prev.o = speed_slae.o;
}

void TBody::doMotion()
{
	auto root_body = this->root_body.lock();
	TVec delta_root = root_body ? 
	                  S->dt * (root_body->speed_slae.r +
	                  root_body->speed_slae.o * rotl(get_axis() - root_body->get_axis()))
	                  : TVec(0,0);
	TVec delta_slae = speed_slae.r * S->dt;
	TVec delta_solid = getSpeed().r * S->dt;
	for (auto& obj: List)
	{
		obj.corner += delta_slae;
	}
	holder.r += delta_solid + delta_root;
	dpos.r += delta_slae - delta_solid - delta_root;
	speed_slae_prev.r = speed_slae.r;
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
	TVec _3S_com= TVec(0., 0.);
	double _12moi_0 = 0.;
	const_for (List, latt)
	{
		_surface+= latt->dl.abs();
		_area+= latt->r.y*latt->dl.x;
		_3S_com-= latt->r * (rotl(latt->corner) * (latt+1)->corner);
		_12moi_0 -= (latt->corner.abs2() + latt->corner*(latt+1)->corner + (latt+1)->corner.abs2())
		            *  (rotl(latt->corner) * (latt+1)->corner);
	}
	_com = _3S_com/(3*_area);
	_moi_com = _12moi_0/12. - _area*_com.abs2();
	_moi_c = _moi_com + _area*(pos.r + dPos.r - _com).abs2();
}

inline double atan2(const TVec &p)
{
	return atan2(p.y, p.x);
}

