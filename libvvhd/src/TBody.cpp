#include "TBody.hpp"

#include <cstdio>
#include <limits>

using std::numeric_limits;
static const double inf = numeric_limits<double>::infinity();
static const double NaN = numeric_limits<double>::quiet_NaN();

TBody::TBody():
    label(),
    alist(),
    root_body(),

    holder(),
    dpos(),
    kspring(inf, inf, inf),
    damping(0.0, 0.0, 0.0),
    speed_slae(),
    speed_slae_prev(),

    collision_min(NaN, NaN, NaN),
    collision_max(NaN, NaN, NaN),
    bounce(),

    density(1.0),
    special_segment_no(0),
    boundary_condition(bc_t::steady),
    heat_condition(hc_t::neglect),

    friction(), friction_prev(),
    force_hydro(),
    force_holder(),
    nusselt(),
    fdt_dead(),
    g_dead(),

    speed_x(),
    speed_y(),
    speed_o(),
    force_o(),

    eq_forces_no(),
    _slip(),
    _slen(),
    _area(),
    _cofm(),
    _moi_cofm(),
    _min_disc_r2(),
    _min_rect_bl(),
    _min_rect_tr(),
    heat_layer()
{
}

TBody::TBody(const TBody& copy):
    TBody()
{
    *this = copy;
}

TBody& TBody::operator= (const TBody& copy)
{
    label = copy.label;
    alist = copy.alist;
    root_body = copy.root_body;
    holder = copy.holder;
    dpos = copy.dpos;
    kspring = copy.kspring;
    damping = copy.damping;
    speed_slae = copy.speed_slae;
    speed_slae_prev = copy.speed_slae_prev;
    collision_min = copy.collision_min;
    collision_max = copy.collision_max;
    bounce = copy.bounce;
    density = copy.density;
    special_segment_no = copy.special_segment_no;
    boundary_condition = copy.boundary_condition;
    heat_condition = copy.heat_condition;
    friction = copy.friction;
    friction_prev = copy.friction_prev;
    force_hydro = copy.force_hydro;
    force_holder = copy.force_holder;
    nusselt = copy.nusselt;
    fdt_dead = copy.fdt_dead;
    g_dead = copy.g_dead;
    speed_x = copy.speed_x;
    speed_y = copy.speed_y;
    speed_o = copy.speed_o;
    force_o = copy.force_o;
    eq_forces_no = copy.eq_forces_no;
    doUpdateSegments();
    doFillProperties();
    return *this;
}

void TBody::validate(bool critical)
{
    auto validate_damping = [](
        bool critical,
        double* val,
        const char* str
    ) {
        auto estr = std::string("TBody::validate(): spring_damping.") + str + " must be ";
        if (!std::isfinite(*val)) {
            estr += "finite";
            if (critical)
                throw std::invalid_argument(estr);
            else {
                fprintf(stderr, "%s\n", estr.c_str());
            }
        }
    };

    validate_damping(critical, &damping.r.x, "r.x");
    validate_damping(critical, &damping.r.y, "r.y");
    validate_damping(critical, &damping.o,   "o");

    auto validate_kspring = [](
        bool critical,
        double* val,
        const char* str
    ) {
        auto estr = std::string("TBody::validate(): spring_const.") + str + " must be ";
        if (std::isnan(*val)) {
            estr += "a number";
            if (critical)
                throw std::invalid_argument(estr);
            else {
                fprintf(stderr, "%s\n", estr.c_str());
            }
        }
        if ( *val < 0 ) {
            estr += "non-negative";
            if (critical)
                throw std::invalid_argument(estr);
            else {
                *val = inf;
                fprintf(stderr, "warning: %s, changing to %lf\n", estr.c_str(), *val);
            }
        }
    };

    validate_kspring(critical, &kspring.r.x, "r.x");
    validate_kspring(critical, &kspring.r.y, "r.y");
    validate_kspring(critical, &kspring.o,   "o");

    auto validate_collision_minmax = [](
        bool critical,
        double* val_min,
        double* val_max,
        const char* str
    ) {
        if ( *val_min >= *val_max ) {
            auto estr = std::string("TBody::validate(): collision_max.") + str;
            estr = estr + " must be greater than collision_min." + str;
            if (critical)
                throw std::invalid_argument(estr);
            else {
                fprintf(stderr, "%s\n", estr.c_str());
            }
        }
    };

    validate_collision_minmax(critical, &collision_min.r.x, &collision_max.r.x, "r.x");
    validate_collision_minmax(critical, &collision_min.r.y, &collision_max.r.y, "r.y");
    validate_collision_minmax(critical, &collision_min.o,   &collision_max.o,   "o");
}

TVec3D TBody::speed(double t) const
{
    return TVec3D(
            speed_x.eval(t),
            speed_y.eval(t),
            speed_o.eval(t)
            );
}

void TBody::move(TVec3D deltaHolder, TVec3D deltaBody)
{
    TVec axis = get_axis();
    double _cos = cos(deltaBody.o);
    double _sin = sin(deltaBody.o);
    for (auto& att: alist)
    {
        TVec dr = att.corner - axis;
        att.corner = axis + deltaBody.r + dr*_cos + rotl(dr)*_sin;
    }
    holder.r += deltaHolder.r;
    holder.o += deltaHolder.o;
    dpos.r += deltaBody.r - deltaHolder.r;
    dpos.o += deltaBody.o - deltaHolder.o;
    speed_slae_prev.r = speed_slae.r;
    speed_slae_prev.o = speed_slae.o;

    doUpdateSegments();
    doFillProperties();
}

void TBody::doUpdateSegments()
{
    if (!alist.size()) {
        return;
    }

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
template <> inline TVec corner(TVec lobj) {return lobj;}
template <> inline TVec corner(TAtt lobj) {return lobj.corner;}
template <class T>
TAtt* TBody::isPointInContour(TVec p, std::vector<T> &list)
{
    bool inContour = isInsideValid();
    if ( !inContour && (
        p.x < _min_rect_bl.x ||
        p.y < _min_rect_bl.y ||
        p.x > _min_rect_tr.x ||
        p.y > _min_rect_tr.y ||
        (p-_cofm).abs2() > _min_disc_r2
        )) return NULL;

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
    _slip = false;
    _slen = 0;
    _area = 0;

    if (!alist.size()) {
        return;
    }

    alist.push_back(alist.front());
    for (auto latt=alist.begin(); latt<alist.end()-1; latt++)
    {
        _slip = _slip || latt->slip;
        _slen+= latt->dl.abs();
        _area+= latt->r.y*latt->dl.x;

    }

    if (_area == 0) {
        TVec _L_cofm = TVec(0., 0.);

        for (auto latt=alist.begin(); latt<alist.end()-1; latt++) {
            _L_cofm += latt->r * latt->dl.abs();
        }

        _cofm = _L_cofm/_slen;
        _moi_cofm = 0.;
    } else {
        TVec _3S_cofm = TVec(0., 0.);
        double _12moi_0 = 0.; // 12 * moment of inertia around point (0, 0)

        for (auto latt=alist.begin(); latt<alist.end()-1; latt++) {
            _3S_cofm-= latt->r * (rotl(latt->corner) * (latt+1)->corner);
            _12moi_0 -= (latt->corner.abs2() + latt->corner*(latt+1)->corner + (latt+1)->corner.abs2())
                *  (rotl(latt->corner) * (latt+1)->corner);
        }

        _cofm = _3S_cofm/(3*_area);
        _moi_cofm = _12moi_0/12. - _area*_cofm.abs2();
    }

    alist.pop_back();

    _min_disc_r2 = 0;
    _min_rect_bl = TVec(
            std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::infinity());
    _min_rect_tr = TVec(
            -std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity());
    for (auto& latt: alist)
    {
        double r2 = (latt.corner-_cofm).abs2();
        if (r2 > _min_disc_r2) _min_disc_r2 = r2;
        if (latt.corner.x > _min_rect_tr.x) _min_rect_tr.x = latt.corner.x;
        if (latt.corner.y > _min_rect_tr.y) _min_rect_tr.y = latt.corner.y;
        if (latt.corner.x < _min_rect_bl.x) _min_rect_bl.x = latt.corner.x;
        if (latt.corner.y < _min_rect_bl.y) _min_rect_bl.y = latt.corner.y;
    }
}

inline double atan2(const TVec &p)
{
    return atan2(p.y, p.x);
}

/********************************** SAVE/LOAD *********************************/
int TBody::load_txt(const char* filename)
{
    alist.clear();

    FILE *fin = fopen(filename, "r");
    if (!fin) { return errno; }

    TVec corner;
    uint32_t slip = false;
    int  err = 0;
    char line[128];
    while ( !err && !feof(fin) && !ferror(fin) && fgets(line, sizeof(line), fin) )
    {
        err |= sscanf(line, "%lf %lf %u", &corner.x, &corner.y, &slip) < 2;
        alist.emplace_back(corner, slip);
    }

    err |= ferror(fin);
    fclose(fin);

    if (err) {
        return errno?:EINVAL;
    }

    doUpdateSegments();
    doFillProperties();

    return 0;
}
