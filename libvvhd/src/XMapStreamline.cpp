#include "XMapStreamline.hpp"

#include "MFlowmove.hpp"
#include "MEpsFast.hpp"
#include "elementary.h"

#include <cmath>
#include <limits>

using std::min;
using std::max;
using std::exp;
using std::isfinite;
using std::vector;
using std::numeric_limits;

static const double inf = numeric_limits<double>::infinity();

XMapStreamfunction::XMapStreamfunction(
    const Space &S,
    double xmin, double ymin,
    double dxdy,
    int xres, int yres,
    double eps_mult,
    char ref_frame
):
    S(S),
    xmin(xmin), ymin(ymin),
    dxdy(dxdy),
    xres(xres), yres(yres),
    eps_mult(eps_mult),
    ref_frame_speed(),
    map(new float[xres*yres]),
    gap_list(),
    evaluated(false),
    dl(S.AverageSegmentLength()),
    rd2(sqr(0.2*dl))
{
    if (xres <= 0)
        throw std::invalid_argument("XMapStreamfunction(): xres must be positive");
    if (yres <= 0)
        throw std::invalid_argument("XMapStreamfunction(): yres must be positive");
    if (dxdy <= 0)
        throw std::invalid_argument("XMapStreamfunction(): dxdy must be positive");
    if (eps_mult <= 0)
        throw std::invalid_argument("XMapStreamfunction(): eps_mult must be positive");

    switch (ref_frame) {
    case 'o':
        ref_frame_speed = TVec(0, 0);
        break;
    case 'f':
        ref_frame_speed = S.InfSpeed();
        break;
    case 'b':
        ref_frame_speed = S.BodyList[0]->speed_slae.r;
        break;
    default:
        throw std::invalid_argument("XMapStreamfunction(): bad ref_frame");
    }
}

XMapStreamfunction::~XMapStreamfunction()
{
    delete[] map;
}

inline static
double _4pi_psi_g(TVec dr, double rd2, double g)
{
    return -g * log(dr.abs2() + rd2);
}

inline static
double _2pi_psi_q(TVec dr, double q)
{
    return q * atan2(dr.y, dr.x);
}

inline static
double _2pi_psi_qatt(TVec p, TVec att, TVec cofm, double q)
{
    TVec v1 = cofm-p;
    TVec v2 = att-p;
    return q * atan2(rotl(v2)*v1, v2*v1);
}

void XMapStreamfunction::evaluate()
{
    if (evaluated)
        return;

    S.HeatList.clear();
    S.StreakList.clear();
    S.StreakSourceList.clear();
    // flowmove fm(&S);
    // fm.VortexShed();

    TSortedTree tree(&S, 8, dl*20);
    tree.build();

    const vector<TSortedNode*>& bnodes = tree.getBottomNodes();

    #pragma omp parallel
    {
        #pragma omp for
        for (auto llbnode = bnodes.cbegin(); llbnode < bnodes.cend(); llbnode++)
        {
            for (TObj *lobj = (**llbnode).vRange.first; lobj < (**llbnode).vRange.last; lobj++)
            {
                lobj->v.x = sqr(eps_mult)*max(epsfast::eps2h(**llbnode, lobj->r)*0.25, sqr(0.2*dl));
            }
        }

        #pragma omp barrier
        // Calculate field ********************************************************

        #pragma omp for collapse(2) schedule(dynamic, 256)
        for (int yj=0; yj<yres; yj++)
        {
            for (int xi=0; xi<xres; xi++)
            {
                TVec p = TVec(xmin, ymin) + dxdy*TVec(xi, yj);
                // double x = xmin + double(xi)*spacing;
                // double y = ymin + double(yj)*spacing;
                double psi_gap = 0;
                // mem[xi*dims[1]+yj] = streamfunction(S, &tree, TVec(x, y), spacing, psi_gap);
                const TSortedNode& bnode = *tree.findNode(p);
                map[yj*xres+xi] = streamfunction(bnode, p, &psi_gap);
                if (psi_gap)
                {
                    gap_t gap = {0};
                    gap.xi = static_cast<uint16_t>(xi);
                    gap.yj = static_cast<uint16_t>(yj);
                    gap.psi_gap = static_cast<float>(psi_gap);
                    #pragma omp critical
                    gap_list.push_back(gap);
                }
            }
        }
    }

    evaluated = true;
}

double XMapStreamfunction::streamfunction(const TSortedNode &node, TVec p, double* psi_gap) const
{
    double tmp_4pi_psi_g = 0.0;
    double tmp_2pi_psi_q = 0.0;
    *psi_gap = 0.0;

    for (auto& lbody: S.BodyList)
    {
        for (auto& latt: lbody->alist)
        {
            tmp_4pi_psi_g += _4pi_psi_g(p-latt.r, rd2, latt.g);
        }

        if (lbody->speed_slae.iszero()) continue;

        for (TAtt& latt: lbody->alist)
        {
            TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            double g = -Vs * latt.dl;
            double q = -rotl(Vs) * latt.dl;
            tmp_4pi_psi_g += _4pi_psi_g(p-latt.r, rd2, g);
            tmp_2pi_psi_q += _2pi_psi_qatt(p, latt.r, lbody->get_cofm(), q);
        }
    }

    for (const auto& src: S.SourceList)
    {
        tmp_2pi_psi_q += _2pi_psi_q(p-src.r, src.g);

        if (p.x < src.r.x && src.r.x <= p.x+dxdy &&
            p.y < src.r.y && src.r.y <= p.y+dxdy)
        {
            *psi_gap += src.g;
        }
    }

    for (TSortedNode* lfnode: *node.FarNodes)
    {
        tmp_4pi_psi_g += _4pi_psi_g(p-lfnode->CMp.r, rd2, lfnode->CMp.g);
        tmp_4pi_psi_g += _4pi_psi_g(p-lfnode->CMm.r, rd2, lfnode->CMm.g);
    }
    for (TSortedNode* lnnode: *node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            tmp_4pi_psi_g += _4pi_psi_g(p-lobj->r, lobj->v.x, lobj->g); // v.x stores eps^2
        }
    }

    // printf("GAP %lf\n", psi_gap);
    return tmp_4pi_psi_g*C_1_4PI + tmp_2pi_psi_q*C_1_2PI + p*rotl(S.InfSpeed() - ref_frame_speed);
}

std::ostream& operator<< (std::ostream& os, XMapStreamfunction& vrt)
{
    vrt.evaluate();

    float N = vrt.xres;
    os.write(reinterpret_cast<const char*>(&N), sizeof(float));
    for (int xi=0; xi<vrt.xres; xi++) {
        float x = vrt.xmin + vrt.dxdy*xi;
        os.write(
            reinterpret_cast<const char*>(&x),
            sizeof(float)
        );
    }

    for (int yj=0; yj<vrt.yres; yj++) {
        float y = vrt.ymin + vrt.dxdy*yj;
        os.write(
            reinterpret_cast<const char*>(&y),
            sizeof(float)
        );
        os.write(
            reinterpret_cast<const char*>(vrt.map+yj*vrt.xres),
            sizeof(float)*vrt.xres
        );
    }

    return os;
}
