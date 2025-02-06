#include "MDiffusiveFast.hpp"
#include "elementary.h"

#define expdef(x) exp(x)

constexpr double ExpArgRestriction = -8.;

void MDiffusiveFast::process_vort_list()
{
    const std::vector<TSortedNode*>& bnodes = tree->getBottomNodes();
    const double re = S->re;

    // FIXME omp here
    // #pragma omp parallel for schedule(dynamic, 10)
    for (auto llbnode = bnodes.begin(); llbnode < bnodes.end(); llbnode++)
    {
        TSortedNode *lbnode = *llbnode;
        for (TObj *lobj = lbnode->vRange.first; lobj < lbnode->vRange.last; lobj++)
        {
            if (!lobj->g) { continue; }

            TVec S2(0,0), S3(0,0);
            double S1 = 0, S0 = 0;

            for (TSortedNode* lnnode: *lbnode->NearNodes)
            {
                for (TObj *ljobj = lnnode->vRange.first; ljobj < lnnode->vRange.last; ljobj++)
                {
                    if (!ljobj->g) { continue; }
                    vortex_influence(*lobj, *ljobj, &S2, &S1);
                }

                for (TObj* ljatt: lnnode->bllist)
                {
                    if (!ljatt) { fprintf(stderr, "diffusivefast.cpp:%d ljatt = NULL. Is it possible?\n", 43 ); continue; }
                    segment_influence(*lobj, static_cast<TAtt*>(ljatt), &S3, &S0, true);
                }
            }

            // trick or treat? ok, trick.
            if ( (sign(S1)!=lobj->sign()) || (fabs(S1)<fabs(0.1*lobj->g)) ) { S1 = 0.1*lobj->g; }

            lobj->v += lobj->_1_eps/(re*S1) * S2;
            if (S0 > C_PI) { S0 = C_PI; }
            lobj->v += (sqr(lobj->_1_eps)/(re*(C_2PI-S0))) * S3;
        }
    }
}

void MDiffusiveFast::process_heat_list()
{
    const std::vector<TSortedNode*>& bnodes = tree->getBottomNodes();
    const double re = S->re;
    const double pr = S->pr;

    // #pragma omp parallel for schedule(dynamic, 10)
    for (auto llbnode = bnodes.begin(); llbnode < bnodes.end(); llbnode++)
    {
        TSortedNode *lbnode = *llbnode;
        for (TObj *lobj = lbnode->hRange.first; lobj < lbnode->hRange.last; lobj++)
        {
            if (!lobj->g) { continue; }

            TVec S2(0,0), S3(0,0);
            double S1 = 0, S0 = 0;

            for (TSortedNode* lnnode: *lbnode->NearNodes)
            {
                for (TObj *ljobj = lnnode->hRange.first; ljobj < lnnode->hRange.last; ljobj++)
                {
                    if (!ljobj->g) { continue; }
                    vortex_influence(*lobj, *ljobj, &S2, &S1);
                }

                for (TObj* ljatt: lnnode->bllist)
                {
                    if (!ljatt) { fprintf(stderr, "diffusivefast.cpp:%d ljatt = NULL. Is it possible?\n", 82 ); continue; }
                    segment_influence(*lobj, static_cast<TAtt*>(ljatt), &S3, &S0, false);
                }
            }

            if ( (sign(S1)!=lobj->sign()) || (fabs(S1)<fabs(0.1*lobj->g)) ) { S1 = 0.1*lobj->g; }

            lobj->v += (lobj->_1_eps*S2+S3)/(S1+S0/sqr(lobj->_1_eps))/(pr*re);
            //			obj.v += obj._1_eps/(Re*S1) * S2;
            //			obj.v += (sqr(obj._1_eps)/(Re*(C_2PI-S0))) * S3;
        }
    }
}

/******************************** NAMESPACE ***********************************/

inline
void MDiffusiveFast::vortex_influence(const TObj &v, const TObj &vj, TVec *i2, double *i1)
{
    if ( sign(v) != sign(vj) ) { return; }
    TVec dr = v.r - vj.r;
    if ( dr.iszero() ) { return; }
    double drabs = dr.abs();
    double exparg = -drabs*v._1_eps;
    if ( exparg < ExpArgRestriction ) return;
    double i1tmp = vj.g * expdef(exparg);
    *i2 += dr * (i1tmp/drabs);
    *i1 += i1tmp;
}

inline
void MDiffusiveFast::segment_influence(const TObj &v, TAtt *rk,
        TVec *i3, double *i0, bool calc_friction)
{
    TVec dr = v.r - rk->r;
    double drabs2 = dr.abs2();
    double drabs = sqrt( drabs2 );
    double exparg = -drabs*v._1_eps;
    if ( exparg < ExpArgRestriction ) {return;}
    double expres = expdef(exparg);
    TVec dS = rotl(rk->dl);
    *i3 += dS * expres;
    *i0 += (drabs*v._1_eps+1)/drabs2*(dr*dS)*expres;

    if (calc_friction)
        rk->fric += sqr(v._1_eps) * v.g * expres * dS.abs();
}
