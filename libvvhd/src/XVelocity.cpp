#include "XVelocity.hpp"

#include <cmath>
#include <limits>

using std::vector;

XVelocity::XVelocity(
    const Space &S,
    double xmin, double ymin,
    double dxdy,
    int xres, int yres
):
    XField(xmin, ymin, dxdy, xres, yres),
    mode(),
    ref_frame(),
    S(S),
    dl(S.average_segment_length()),
    tree(&this->S, 8, dl*20, 0.1),
    eps(&this->S, &tree),
    convective(&this->S, &tree)
{}

void XVelocity::evaluate()
{
    TVec ref_frame_speed = TVec(0, 0);
    switch (ref_frame) {
    case 'o':
        break;
    case 'f':
        ref_frame_speed = S.inf_speed();
        break;
    case 'b':
        ref_frame_speed = S.BodyList[0]->speed_slae.r;
        break;
    default:
        throw std::invalid_argument("XVelocity(): bad ref_frame");
    }

    if (mode != 'x' && mode != 'y') {
        throw std::invalid_argument("XPressure(): bad mode");
    }

    if (evaluated)
        return;

    S.HeatList.clear();
    S.StreakList.clear();
    S.StreakSourceList.clear();
    tree.build();
    eps.CalcEpsilonFast(false);

    #pragma omp parallel
    {
        #pragma omp for collapse(2) schedule(dynamic, 256)
        for (int yj=0; yj<yres; yj++)
        {
            for (int xi=0; xi<xres; xi++)
            {
                TVec p = TVec(xmin, ymin) + dxdy*TVec(xi, yj);
                TVec u = convective.velocity(p) - ref_frame_speed;

                if (mode == 'x') {
                    map[yj*xres+xi] = u.x;
                } else if (mode == 'y') {
                    map[yj*xres+xi] = u.y;
                }
            }
        }
    }

    evaluated = true;
}
