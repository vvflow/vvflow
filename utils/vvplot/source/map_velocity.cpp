#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <ctime>
#include <hdf5.h>

#include "libvvplot_api.h"
#include "MConvectiveFast.hpp"
#include "MEpsFast.hpp"
#include "MFlowmove.hpp"

static double Rd2;
static TVec RefFrame_Speed;
inline static double atan(TVec p) {return atan(p.y/p.x);}

extern "C" {
#if 0 // dont break vim autoindent
}
#endif
int velocity_print(hid_t fid, TVec* points, int count)
{
    Space *S = new Space();
    S->Load(fid);

    bool is_viscous = (S->Re != std::numeric_limits<double>::infinity());
    double dl = S->AverageSegmentLength(); Rd2 = dl*dl/25;
    TSortedTree tree(S, 8, dl*20, 0.3);
    convectivefast conv(S, &tree);
    flowmove flow(S);
    epsfast eps(S, &tree);
    flow.VortexShed();
    tree.build();
    eps.CalcEpsilonFast(/*merge=*/is_viscous);

    for (int i=0; i<count; i++)
    {
        TVec p = points[i];
        TVec v = conv.SpeedSumFast(p);
        printf("%.6le\t %.6le\n", v.x, v.y);
    }

    return 0;
}}
