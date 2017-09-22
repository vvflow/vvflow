#include "map_vorticity.h"

#include <flowmove.h>
#include <epsfast.h>

DEFINE_double(EPS_MULT, 2.0, "smoothing factor for gaussian pofiles of vortex domains");
static double DL = 0.0; // average segments length

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

    double erfarg = h2(*bnode, p)/sqr(DL*FLAGS_EPS_MULT);
    T+= (erfarg<3) ? 0.5*(1-erf(erfarg)) : 0;

    return T;
}

void map_vorticity(Space *S, std::stringstream &bin, rect_t &rect, mesh_t &mesh)
{
    S->HeatList.clear();
    S->StreakList.clear();
    flowmove fm(S);
    fm.VortexShed();

    DL = S->AverageSegmentLength();
    S->Tree = new TSortedTree(S, 8, DL*20, std::numeric_limits<double>::max());
    S->Tree->build();

    auto& bnodes = S->Tree->getBottomNodes();
    float mem[mesh.yres+1][mesh.xres+1];
    mem[0][0] = mesh.xres;
    for (size_t xi=0; xi<mesh.xres; xi++)
        mem[0][xi+1] = rect.xmin + double(xi)*mesh.dxdy;
    for (size_t yj=0; yj<mesh.yres; yj++)
        mem[yj+1][0] = rect.ymin + double(yj)*mesh.dxdy;

    #pragma omp parallel
    {
        #pragma omp for
        for (auto llbnode = bnodes.begin(); llbnode < bnodes.end(); llbnode++)
        {
            for (TObj *lobj = (**llbnode).vRange.first; lobj < (**llbnode).vRange.last; lobj++)
            {
                lobj->v.x = 1./(sqr(FLAGS_EPS_MULT)*max(eps2h(**llbnode, lobj->r), sqr(0.6*DL)));
                lobj->v.y = lobj->v.x * lobj->g;
            }
        }

        #pragma omp barrier
        // Calculate field ********************************************************

        #pragma omp for collapse(2) schedule(dynamic, 100)
        for (size_t xi=0; xi<mesh.xres; xi++)
        {
            for (size_t yj=0; yj<mesh.yres; yj++)
            {
                TVec xy(mem[0][xi+1], mem[yj+1][0]);
                mem[yj+1][xi+1] = S->PointIsInBody(xy) ? 0 : Vorticity(S, xy); 
            }
        }
    }

    bin.write(reinterpret_cast<const char*>(mem), sizeof(mem));

    /**************************************************************************
    *** SAVE RESULTS **********************************************************
    **************************************************************************/

    // map_save(fid, "map_vorticity", mem, dims, xmin, xmax, ymin, ymax, spacing);
}
