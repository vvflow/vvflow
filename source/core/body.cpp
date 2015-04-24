#include "space.h"
#include "body.h"
#include <stdio.h>
#include <iostream>
#include <limits>
#include <math.h>
using namespace std;

TBody::TBody(Space *space):
    alist(),
    speed_x(),
    speed_y(),
    speed_o()
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
    rotation_error = TVec(0., 0.);
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


TVec3D TBody::get_speed() const
{
    return TVec3D(
            speed_x.getValue(S->Time),
            speed_y.getValue(S->Time),
            speed_o.getValue(S->Time)
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
    const double angle_slae = speed_slae.o * S->dt; //in doc \omega_? \Delta t
    const double angle_solid = get_speed().o * S->dt; //in doc \omega \Delta t
    for (auto& att: alist)
    {
        TVec dr = att.corner - get_axis();
        att.corner = get_axis() + dr*cos(angle_slae) + rotl(dr)*sin(angle_slae);
    }

    auto root_body = this->root_body.lock();
    double angle_root = !root_body ? 0 : S->dt * root_body->speed_slae.o;
    holder.o += angle_solid + angle_root;
    dpos.o += angle_slae - angle_solid - angle_root;
    speed_slae_prev.o = speed_slae.o;
}

void TBody::doMotion()
{
    TVec delta_slae = speed_slae.r * S->dt - rotation_error;
    TVec delta_solid = get_speed().r * S->dt - rotation_error;
    for (auto& obj: alist)
    {
        obj.corner += delta_slae;
    }
    auto root_body = this->root_body.lock();
    TVec delta_root(0, 0);
    if (root_body)
    {
	TVec dr = holder.r - root_body->get_axis();
	double da = S->dt * root_body->speed_slae.o;
	delta_root = S->dt * root_body->speed_slae.r + (dr*cos(da)+rotl(dr)*sin(da)-dr);
    }

    holder.r += delta_solid;
    dpos.r += delta_slae - delta_solid;
    speed_slae_prev.r = speed_slae.r;
    rotation_error = TVec3D();
}

void TBody::doUpdateSegments()
{
    alist.push_back(alist.front());

    for (auto lobj=alist.begin(); lobj<alist.end()-1; lobj++)
    {
        lobj->dl = (lobj+1)->corner - lobj->corner;
        lobj->_1_eps = 3.0/lobj->dl.abs();
        lobj->r = 0.5*((lobj+1)->corner + lobj->corner);
    }
    alist.pop_back();
}

TAtt* TBody::isPointInvalid(TVec p)
{
    return isPointInContour(p, alist);
}

TAtt* TBody::isPointInHeatLayer(TVec p)
{
    if (!heat_layer.size())
    {
        for (auto& lobj: alist)
        {
            heat_layer.push_back(lobj.r + rotl(lobj.dl));
        }
    }

    return isPointInContour(p, heat_layer);
}

template <class T> TVec corner(T lobj);
template <> TVec corner(TVec lobj) {return lobj;}
template <> TVec corner(TAtt lobj) {return lobj.corner;}
template <class T>
TAtt* TBody::isPointInContour(TVec p, vector<T> &list)
{
    bool inContour = isInsideValid();
    TAtt *nearest = NULL;
    double nearest_dr2 = std::numeric_limits<double>::max();

    for (auto i = list.begin(), j = list.end()-1; i<list.end(); j=i++)
    {
        TVec vi = corner<T>(*i);
        TVec vj = corner<T>(*j);

        if ((
                    (vi.y < vj.y) && (vi.y < p.y) && (p.y <= vj.y) &&
                    ((vj.y - vi.y) * (p.x - vi.x) > (vj.x - vi.x) * (p.y - vi.y))
            ) || (
                (vi.y > vj.y) && (vi.y > p.y) && (p.y >= vj.y) &&
                ((vj.y - vi.y) * (p.x - vi.x) < (vj.x - vi.x) * (p.y - vi.y))
                )) inContour = !inContour;
    }

    if (!inContour) return NULL;

    // FIXME почитать алгоритмы, поискать оптимизированный поиск минимума
    for (auto& latt: alist)
    {
        double dr2 = (latt.r - p).abs2();
        if ( dr2 < nearest_dr2 )
        {
            nearest = &latt;
            nearest_dr2 = dr2;
        }
    }

    return nearest;
}

void TBody::doFillProperties()
{
    // FIXME mistake: moi_c after load and save changes
    _surface = _area = _moi_c = _moi_com = 0;
    TVec _3S_com= TVec(0., 0.);
    double _12moi_0 = 0.;

    alist.push_back(alist.front());
    for (auto latt=alist.begin(); latt<alist.end()-1; latt++)
    {
        _surface+= latt->dl.abs();
        _area+= latt->r.y*latt->dl.x;
        _3S_com-= latt->r * (rotl(latt->corner) * (latt+1)->corner);
        _12moi_0 -= (latt->corner.abs2() + latt->corner*(latt+1)->corner + (latt+1)->corner.abs2())
            *  (rotl(latt->corner) * (latt+1)->corner);

    }
    alist.pop_back();
    _com = _3S_com/(3*_area);
    _moi_com = _12moi_0/12. - _area*_com.abs2();
    _moi_c = _moi_com + _area*(get_axis() - _com).abs2();
}

inline double atan2(const TVec &p)
{
    return atan2(p.y, p.x);
}

