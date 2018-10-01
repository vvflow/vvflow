#include "XStreamfunction.hpp"

#include "MFlowmove.hpp"
#include "MEpsFast.hpp"
#include "elementary.h"

#include <cmath>
#include <limits>

using std::vector;

XStreamfunction::XStreamfunction(
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
    rd2(sqr(0.2*dl))
{}

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
    return -q * atan2(rotl(v2)*v1, v2*v1);
}

void XStreamfunction::evaluate()
{
    if (eps_mult <= 0)
        throw std::invalid_argument("XStreamfunction(): eps_mult must be positive");

    switch (ref_frame) {
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
        throw std::invalid_argument("XStreamfunction(): bad ref_frame");
    }

    if (evaluated)
        return;

    S.HeatList.clear();
    S.StreakList.clear();
    S.StreakSourceList.clear();
    // flowmove fm(&S);
    // fm.VortexShed();

    double min_node_size = dl>0 ? dl*10 : 0;
    double max_node_size = dl>0 ? dl*20 : 1.0l/0.0l;
    TSortedTree tree(&S, 8, min_node_size, max_node_size);
    tree.build();

    const vector<TSortedNode*>& bnodes = tree.getBottomNodes();

    #pragma omp parallel
    {
        #pragma omp for
        for (auto llbnode = bnodes.cbegin(); llbnode < bnodes.cend(); llbnode++)
        {
            for (TObj *lobj = (**llbnode).vRange.first; lobj < (**llbnode).vRange.last; lobj++)
            {
                lobj->v.x = sqr(eps_mult)*std::max(epsfast::eps2h(**llbnode, lobj->r)*0.25, sqr(0.2*dl));
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
                    #pragma omp critical
                    gaps.emplace_back(xi, yj, psi_gap);
                }
            }
        }
    }

    evaluated = true;
}

// static
double XStreamfunction::streamfunction(const Space &S, TVec p)
{
    double tmp_4pi_psi_g = 0.0;
    double tmp_2pi_psi_q = 0.0;

    const double dl = S.average_segment_length();
    const double rd2 = sqr(0.2*dl);

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

    for (const TObj& src: S.SourceList)
    {
        tmp_2pi_psi_q += _2pi_psi_q(p-src.r, src.g);
    }

    for (const TObj& vrt: S.VortexList)
    {
        tmp_4pi_psi_g += _4pi_psi_g(p-vrt.r, rd2, vrt.g);
    }

    return tmp_4pi_psi_g*C_1_4PI + tmp_2pi_psi_q*C_1_2PI + p*rotl(S.inf_speed());
}

double XStreamfunction::streamfunction(const TSortedNode &node, TVec p, double* psi_gap) const
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
    return tmp_4pi_psi_g*C_1_4PI + tmp_2pi_psi_q*C_1_2PI + p*rotl(S.inf_speed() - ref_frame_speed);
}
