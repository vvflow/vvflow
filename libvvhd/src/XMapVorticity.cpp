#include "XMapVorticity.hpp"

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
// static const double NaN = numeric_limits<double>::quiet_NaN();

XMapVorticity::XMapVorticity(
    const Space &S,
    double xmin, double ymin,
    double dxdy,
    int xres, int yres,
    double eps_mult
):
    S(S),
    xmin(xmin), ymin(ymin),
    dxdy(dxdy),
    xres(xres), yres(yres),
    eps_mult(eps_mult),
    map(new float[xres*yres]),
    evaluated(false),
    dl(S.AverageSegmentLength())
{
    if (xres <= 0)
        throw std::invalid_argument("XMapVorticity(): xres must be positive");
    if (yres <= 0)
        throw std::invalid_argument("XMapVorticity(): yres must be positive");
    if (dxdy <= 0)
        throw std::invalid_argument("XMapVorticity(): dxdy must be positive");
    if (eps_mult <= 0)
        throw std::invalid_argument("XMapVorticity(): eps_mult must be positive");
}

XMapVorticity::~XMapVorticity()
{
    delete[] map;
}


void XMapVorticity::evaluate()
{
    if (evaluated)
        return;

    S.HeatList.clear();
    S.StreakList.clear();
    S.StreakSourceList.clear();
    flowmove fm(&S);
    fm.VortexShed();

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
                lobj->v.x = 1./(sqr(eps_mult)*max(eps2h(**llbnode, lobj->r), sqr(0.6*dl)));
                lobj->v.y = lobj->v.x * lobj->g;
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
                const TSortedNode* bnode = tree.findNode(p);
                map[yj*xres+xi] = S.PointIsInBody(p) ? 0 : vorticity(*bnode, p);
            }
        }
    }

    evaluated = true;
}

double XMapVorticity::eps2h(const TSortedNode &node, TVec p) const
{
    double res[2] = {inf, inf};

    for (const TSortedNode* lnnode: *node.NearNodes)
    {
        for (const TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            double drabs2 = (p-lobj->r).abs2();
            if ( !drabs2 ) {
                continue;
            } else if ( drabs2 < res[0]) {
                res[1] = res[0];
                res[0] = drabs2;
            } else if ( drabs2 < res[1] ) {
                res[1] = drabs2;
            }
        }
    }

    if ( isfinite(res[1]) )
        return res[1];
    if ( isfinite(res[0]) )
        return res[0];
    return numeric_limits<double>::lowest();
}

double XMapVorticity::h2(const TSortedNode &node, TVec p) const
{
    double res = inf;

    for (const TSortedNode* lnnode: *node.NearNodes)
    {
        for (TObj* latt: lnnode->bllist)
        {
            res = min(res, (p-latt->r).abs2());
        }
    }

    return res;
}

double XMapVorticity::vorticity(const TSortedNode &node, TVec p) const
{
    double res = 0;

    for (const TSortedNode* lnnode: *node.NearNodes)
    {
        for (const TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            double exparg = -(p-lobj->r).abs2() * lobj->v.x; // v.x stores eps^(-2)
            res+= (exparg>-6) ? lobj->v.y * exp(exparg) : 0; // v.y stores g*eps(-2)
        }
    }

    res *= C_1_PI;

    double erfarg = h2(node, p)/sqr(dl*eps_mult);
    res += (erfarg<3) ? 0.5*(1-erf(erfarg)) : 0;

    return res;
}

std::ostream& operator<< (std::ostream& os, XMapVorticity& vrt)
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
