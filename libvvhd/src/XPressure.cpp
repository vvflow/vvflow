#include "XPressure.hpp"
#include "elementary.h"

#include <cmath>
#include <limits>

using std::max;
using std::exp;
using std::vector;

static const double d_nan = nan("");

XPressure::XPressure(
    const Space &S,
    double xmin, double ymin,
    double dxdy,
    int xres, int yres
):
    XField(xmin, ymin, dxdy, xres, yres),
    eps_mult(),
    ref_frame(),
    S(S),
    ref_frame_speed(),
    dl(S.average_segment_length()),
    tree(&this->S, 8, dl*20, 0.1),
    convective(&this->S, &tree),
    diffusive(&this->S, &tree),
    flowmove(&this->S),
    eps(&this->S, &tree)
{}

void XPressure::evaluate()
{
    if (eps_mult <= 0)
        throw std::invalid_argument("XPressure(): eps_mult must be positive");

    switch (ref_frame) {
    case 's':
        break;
    case 'o':
        ref_frame_speed = TVec(0, 0);
        break;
    case 'f':
        ref_frame_speed = S.inf_speed();
        break;
    case 'b':
        ref_frame_speed = S.BodyList[0]->speed_slae.r;
        break;
    default:
        throw std::invalid_argument("XPressure(): bad ref_frame");
    }

    if (evaluated)
        return;

    S.HeatList.clear();
    S.StreakList.clear();
    S.StreakSourceList.clear();
    flowmove.vortex_shed();

    //дано: условие непротекания выполнено, неизвестные вихри найдены и рождены.
    //требуется: найти скорости вихрей (всех, включая присоединенные) и посчитать давление
    tree.build();
    eps.CalcEpsilonFast(false);
    convective.process_all_lists();
    diffusive.process_vort_list();

    // const vector<TSortedNode*>& bnodes = tree.getBottomNodes();

    #pragma omp parallel
    {
        // #pragma omp for
        // for (auto llbnode = bnodes.cbegin(); llbnode < bnodes.cend(); llbnode++)
        // {
        //     for (TObj *lobj = (**llbnode).vRange.first; lobj < (**llbnode).vRange.last; lobj++)
        //     {
        //         lobj->v.x = 1./(sqr(eps_mult)*std::max(epsfast::eps2h(**llbnode, lobj->r), sqr(0.6*dl)));
        //         lobj->v.y = lobj->v.x * lobj->g;
        //     }
        // }

        // #pragma omp barrier
        // // Calculate field ********************************************************

        #pragma omp for collapse(2) schedule(dynamic, 256)
        for (int yj=0; yj<yres; yj++)
        {
            for (int xi=0; xi<xres; xi++)
            {
                TVec p = TVec(xmin, ymin) + dxdy*TVec(xi, yj);
                map[yj*xres+xi] = S.point_is_in_body(p) ? 0 : pressure(p);
                // map[xi*dims[1]+yj] = S->point_is_in_body(TVec(x, y)) ?
                //        0 : Pressure(S, &conv, TVec(x, y), RefFrame, NaN);

            }
        }
    }

    evaluated = true;
}

inline static
TVec K(const TVec &obj, const TVec &p)
{
    TVec dr = p - obj;
    return dr/dr.abs2();
}

double XPressure::pressure(TVec p) const
{
    double _2PI_Cp=0;

    for (auto& lbody: S.BodyList)
    {
        //first addend
        for (auto& latt: lbody->alist)
        {
            TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            double g = -Vs * latt.dl;
            double q = -rotl(Vs) * latt.dl;
            _2PI_Cp += (rotl(K(latt.r, p))*g + K(latt.r, p)*q) * Vs;
        }

        //second addend
        double gtmp = 0;
        for (auto& latt: lbody->alist)
        {
            // TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            gtmp+= latt.gsum;
            _2PI_Cp -= (latt.dl/S.dt * rotl(K(latt.r, p))) * gtmp;
        }
    }

    for (auto& lobj: S.VortexList)
    {
        _2PI_Cp+= (lobj.v * rotl(K(lobj.r, p))) * lobj.g;
    }

    TVec local_speed = convective.velocity(p);
    double Cp = C_1_2PI * _2PI_Cp + 0.5*(S.inf_speed().abs2() - local_speed.abs2());
    if (ref_frame != 's') {
        Cp += 0.5*((local_speed - ref_frame_speed).abs2());
    }

    return Cp;
}
