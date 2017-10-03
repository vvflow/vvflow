#include "XIsoline.hpp"

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

XIsoline::XIsoline(
    const XField& field,
    double vmin,
    double vmax,
    double dv
):
    isolines()
{
    if (!isfinite(vmax))
        throw std::invalid_argument("XIsoline(): vmax must finite");
    if (!(vmin <= vmax))
        throw std::invalid_argument("XIsoline(): vmin must be <= vmax");
    if (!(dv > 0))
        throw std::invalid_argument("XIsoline(): dv must be positive");

    for (int i=0; i<field.xres-1; i++)
    {
        // float x = xmin + i*spacing;
        for (int j=0; j<field.yres-1; j++)
        {
            // float y = ymin + j*spacing;

            // 1 2
            // 0 3
            float corners[5] = {field.at(i+0, j+0), field.at(i+0, j+1),
                                field.at(i+1, j+1), field.at(i+1, j+0),
                                field.at(i+0, j+0)};

            for (const TGap& gap: field.gaps)
            {
                if (gap.yj != j)
                    continue;
                if (gap.xi >= i)
                    corners[0] += gap.gap;
                if (gap.xi > i) {
                    corners[3] += gap.gap;
                    corners[4] += gap.gap;
                }

            }

            for (double v=vmin; v<=vmax; v+=dv)
                process_rect(i, j, corners, v);
        }
    }

    for (auto it = isolines->begin(); it!= isolines->end(); it++)
    {
        line_t *l = *it;
        for (auto vec = l->begin(); vec!=l->end(); vec++)
        {
            float xy[2] = { static_cast<float>(xmin+vec->x*spacing), static_cast<float>(ymin+vec->y*spacing) };
            fwrite(xy, sizeof(float), 2, stdout);
        }
        float nans[2] = {NaN, NaN};
        fwrite(nans, sizeof(float), 2, stdout);
    }
    fflush(stdout);

    delete isolines;
    free(mem);
    H5Sclose(map_h5s);
    H5Dclose(map_h5d);
    return 0;
}

inline static
bool inrange(float z1, float z2, float c)
{
    return (z1 <= c && c < z2) || (z1 >= c && c > z2);
}

void XIsoline::merge_lines(TLine* dst, bool dst_side)
{
    TPoint p = dst_side ? dst->front() : dst->back();

    for (auto lit = isolines->begin(); lit != isolines->end(); lit++)
    {
        TLine *l = *lit;
        if (l == dst) continue;
        else if (l->front() == p)
        {
            if (dst_side) dst->insert(dst->begin(), l->rbegin(), l->rend());
            else          dst->insert(dst->end(),   l->begin(), l->end());
        }
        else if (l->back() == p)
        {
            if (dst_side) dst->insert(dst->begin(), l->begin(), l->end());
            else          dst->insert(dst->end(),   l->rbegin(), l->rend());
        }
        else continue;

        delete *lit;
        isolines->erase(lit);
        break;
    }
}

void XIsoline::commit_segment(TPoint p1, TPoint p2)
{
    for (auto it = isolines->begin(); it!= isolines->end(); it++)
    {
        line_t *l = *it;

        /**/ if (l->front() == p1) { l->insert(l->begin(), p2); merge_lines(l, 1); return; }
        else if (l->front() == p2) { l->insert(l->begin(), p1); merge_lines(l, 1); return; }
        else if (l->back()  == p1) { l->insert(l->end(),   p2); merge_lines(l, 0); return; }
        else if (l->back()  == p2) { l->insert(l->end(),   p1); merge_lines(l, 0); return; }
    }
    line_t *new_line = new line_t();
    new_line->push_back(p1);
    new_line->push_back(p2);
    isolines->push_back(new_line);
    return;
}

void XIsoline::process_rect(float x, float y, float z[5], float C)
{
    TPoint p[4];
    int N = 0;
    if (inrange(z[0], z[1], C)) p[N++] = TPoint(x,                        y + (C-z[0])/(z[1]-z[0]));
    if (inrange(z[1], z[2], C)) p[N++] = TPoint(x + (C-z[1])/(z[2]-z[1]), y + 1.0);
    if (inrange(z[2], z[3], C)) p[N++] = TPoint(x + 1.0,                  y + (C-z[3])/(z[2]-z[3]));
    if (inrange(z[3], z[4], C)) p[N++] = TPoint(x + (C-z[4])/(z[3]-z[4]), y);

    if (N>=2) commit_segment(p[0], p[1]);
    if (N>=4) commit_segment(p[2], p[3]);
}

void XStreamfunction::evaluate()
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
                    #pragma omp critical
                    gap_list.emplace_back(xi, yj, psi_gap);
                }
            }
        }
    }

    evaluated = true;
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
    return tmp_4pi_psi_g*C_1_4PI + tmp_2pi_psi_q*C_1_2PI + p*rotl(S.InfSpeed() - ref_frame_speed);
}
