#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <limits>
#include <fstream>
#include <math.h>
#include <assert.h>
#include "hdf5.h"

#include "libvvplot_api.h"
#include "flowmove.h"
#include "epsfast.h"

static const double ExpArgRestriction = -8.;
static double dl;
static double EPS_MULT;
using namespace std;

double eps2h(const TSortedNode &Node, TVec p)
{
    TVec dr;
    double res1, res2;
    res2 = res1 = std::numeric_limits<double>::max();

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            dr = p - lobj->r;
            double drabs2 = dr.abs2();
            if ( !drabs2 ) continue;
            if ( res1 > drabs2 )
            {
                res2 = res1;
                res1 = drabs2;
            }
            else if ( res2 > drabs2 )
            {
                res2 = drabs2;
            }
        }
    }

    if ( res1 == std::numeric_limits<double>::max() ) return std::numeric_limits<double>::lowest();
    if ( res2 == std::numeric_limits<double>::max() ) return std::numeric_limits<double>::lowest();//res1;

    return res2;
}

TObj* Nearest(TSortedNode &Node, TVec p)
{
    TVec dr(0, 0);
    double resr = std::numeric_limits<double>::max();
    TObj *res = NULL;

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            dr = p - lobj->r;
            double drabs2 = dr.abs2();
            if ( !drabs2 ) continue;
            if ( drabs2 <= resr )
            {
                res = lobj;
                resr = drabs2;
            }
        }
    }

    return res;
}

double h2(TSortedNode &Node, TVec p)
{
    double resh2 = std::numeric_limits<double>::max();

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        for (TObj* latt: lnnode->bllist)
        {
            resh2 = min(resh2, (p-latt->r).abs2());
        }
    }

    return resh2;
}

double Vorticity(Space* S, TVec p)
{
    double T=0;
    TSortedNode* bnode = S->Tree->findNode(p);

    //return  bnode->NearNodes->size_safe();
    // TObj* nrst = Nearest(*bnode, p);

    //return nrst - S->HeatList->begin();

    for (TSortedNode* lnnode: *bnode->NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            double exparg = -(p-lobj->r).abs2() * lobj->v.x; // v.rx stores eps^(-2)
            T+= (exparg>-10) ? lobj->v.y * exp(exparg) : 0; // v.ry stores g*eps(-2)
        }
    }

    T*= C_1_PI;

    double erfarg = h2(*bnode, p)/sqr(dl*EPS_MULT);
    T+= (erfarg<3) ? 0.5*(1-erf(erfarg)) : 0;

    return T;
}

extern "C" {
int map_vorticity(hid_t fid, double xmin, double xmax, double ymin, double ymax, double spacing)
{
    char *mult_env = getenv("VV_EPS_MULT");
    EPS_MULT = mult_env ? atof(mult_env) : 2;

    /**************************************************************************
    *** MAIN JOB **************************************************************
    **************************************************************************/

    Space *S = new Space();
    S->Load(fid);
    flowmove fm(S);
    fm.VortexShed();
    S->HeatList.clear();
    S->StreakList.clear();

    dl = S->AverageSegmentLength();
    S->Tree = new TSortedTree(S, 8, dl*20, std::numeric_limits<double>::max());
    S->Tree->build();

    auto& bnodes = S->Tree->getBottomNodes();
    #pragma omp parallel for
    for (auto llbnode = bnodes.begin(); llbnode < bnodes.end(); llbnode++)
    {
        TSortedNode* lbnode = *llbnode;
        for (TObj *lobj = lbnode->vRange.first; lobj < lbnode->vRange.last; lobj++)
        {
            lobj->v.x = 1./(sqr(EPS_MULT)*max(eps2h(*lbnode, lobj->r), sqr(0.6*dl)));
            lobj->v.y = lobj->v.x * lobj->g;
        }
    }

    // Calculate field ********************************************************
    hsize_t dims[2] = {
        static_cast<size_t>((xmax-xmin)/spacing) + 1,
        static_cast<size_t>((ymax-ymin)/spacing) + 1
    };
    float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);

    for (size_t xi=0; xi<dims[0]; xi++)
    {
        double x = xmin + double(xi)*spacing;
        #pragma omp parallel for ordered schedule(dynamic, 100)
        for (size_t yj=0; yj<dims[1]; yj++)
        {
            double y = ymin + double(yj)*spacing;
            TVec xy(x,y);
            mem[xi*dims[1]+yj] = S->PointIsInBody(xy) ? 0 : Vorticity(S, xy);
        }
    }

    /**************************************************************************
    *** SAVE RESULTS **********************************************************
    **************************************************************************/

    map_save(fid, "map_vorticity", mem, dims, xmin, xmax, ymin, ymax, spacing);
    free(mem);

    return 0;
}}
