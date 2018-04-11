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
    S->load_hdf(fid);

    bool is_viscous = (S->re != std::numeric_limits<double>::infinity());
    double dl = S->average_segment_length(); Rd2 = dl*dl/25;
    TSortedTree tree(S, 8, dl*20, 0.3);
    MConvectiveFast conv(S, &tree);
    MFlowmove flow(S);
    epsfast eps(S, &tree);
    flow.vortex_shed();
    tree.build();
    eps.CalcEpsilonFast(/*merge=*/is_viscous);

    for (int i=0; i<count; i++)
    {
        TVec p = points[i];
        TVec v = conv.velocity(p);
        printf("%.6le\t %.6le\n", v.x, v.y);
    }

    return 0;
}}
