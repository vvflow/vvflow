#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include <time.h>

#include "libvvplot_api.h"
#include "convectivefast.h"
#include "epsfast.h"
#include "flowmove.h"
#include "core.h"
#include "hdf5.h"

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
    S->Tree = new TSortedTree(S, 8, dl*20, 0.3);
    convectivefast conv(S);
    flowmove flow(S);
    epsfast eps(S);
    flow.VortexShed();
    S->Tree->build();
    eps.CalcEpsilonFast(/*merge=*/is_viscous);

    for (int i=0; i<count; i++)
    {
        TVec p = points[i];
        TVec v = conv.SpeedSumFast(p);
        printf("%.6le\t %.6le\n", v.x, v.y);
    }

    return 0;
}}
