#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <hdf5.h>

#include "elementary.h"
#include "libvvplot_api.h"
#include "MConvectiveFast.hpp"
#include "MEpsilonFast.hpp"
#include "MDiffusiveFast.hpp"
#include "MFlowmove.hpp"

static const double NaN = std::numeric_limits<double>::quiet_NaN();
static const double S1Restriction = 1E-6;
static const double ExpArgRestriction = -8.;
using namespace std;
#define dbg(a) { cerr << #a << "... "; a; cerr << "done\n"; }
//#define dbg(a) a

double Rd2;

inline
TVec K(const TVec &obj, const TVec &p)
{
    TVec dr = p - obj;
    return dr/(dr.abs2() + Rd2);
}

double Pressure(Space* S, MConvectiveFast *conv, TVec p, char RefFrame, double precision)
{
    double _2PI_Cp=0;

    for (auto& lbody: S->BodyList)
    {
        //first addend
        for (auto& latt: lbody->alist)
        {
            TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            double g = -Vs * latt.dl;
            double q = -rotl(Vs) * latt.dl;
            _2PI_Cp += (rotl(K(latt.r, p))*g + K(latt.r, p)*q) * Vs;
        }

        //second addend
        double gtmp = 0;
        for (auto& latt: lbody->alist)
        {
            // TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            gtmp+= latt.gsum;
            _2PI_Cp -= (latt.dl/S->dt * rotl(K(latt.r, p))) * gtmp;
        }
    }

    for (auto& lobj: S->VortexList)
    {
        _2PI_Cp+= (lobj.v * rotl(K(lobj.r, p))) * lobj.g;
    }

    TVec LocalSpeed = conv->velocity(p);
    double Cp_static = C_1_2PI * _2PI_Cp + 0.5*(S->inf_speed().abs2() - LocalSpeed.abs2());
    switch (RefFrame)
    {
        case 's': return Cp_static;
        case 'o': return Cp_static + 0.5*(LocalSpeed.abs2());
        case 'f': return Cp_static + 0.5*((LocalSpeed - S->inf_speed()).abs2());
        case 'b': return Cp_static + 0.5*((LocalSpeed - S->inf_speed() + TVec(1, 0)).abs2());
        default: cerr << "Bad reference frame!!!" << endl; return -2;
    }
    return Cp_static; // not used
}

extern "C" {
#if 0 // dont break vim autoindent
}
#endif
int map_pressure(hid_t fid, char RefFrame, double xmin, double xmax, double ymin, double ymax, double spacing)
{
    Space *S = new Space();
    S->load_hdf(fid);

    double dl = S->average_segment_length();
    TSortedTree tr(S, 8, dl*20, 0.1);
    MConvectiveFast conv(S, &tr);
    MEpsilonFast eps(S, &tr);
    MDiffusiveFast diff(S, &tr);
    MFlowmove fm(S);

    /**************************** LOAD ARGUMENTS ******************************/

    switch(RefFrame)
    {
        case 's': case 'o': case 'b': case 'f': break;
        fprintf(stderr, "Bad reference frame\n");
        fprintf(stderr, "Available options are:\n");
        fprintf(stderr, " 's' : static pressure\n" );
        fprintf(stderr, " 'o' : dynamic pressure (original reference frame)\n" );
        fprintf(stderr, " 'f' : dynamic pressure (fluid reference frame, body is moving)\n" );
        fprintf(stderr, " 'b' : dynamic pressure (body reference frame, fluid is moving)\n" );
        return -2;
    }

    fm.vortex_shed();

    //дано: условие непротекания выполнено, неизвестные вихри найдены и рождены.
    //требуется: найти скорости вихрей (всех, включая присоединенные) и посчитать давление
    tr.build();
    eps.CalcEpsilonFast(false);
    conv.process_all_lists();
    diff.process_vort_list();

    hsize_t dims[2] = {
        static_cast<size_t>((xmax-xmin)/spacing) + 1,
        static_cast<size_t>((ymax-ymin)/spacing) + 1
    };
    float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);

    for (size_t xi=0; xi<dims[0]; xi++)
    {
        double x = xmin + double(xi)*spacing;
        #pragma omp parallel for ordered schedule(dynamic, 20)
        for (size_t yj=0; yj<dims[1]; yj++)
        {
            double y = ymin + double(yj)*spacing;
            mem[xi*dims[1]+yj] = S->point_is_in_body(TVec(x, y)) ?
                       0 : Pressure(S, &conv, TVec(x, y), RefFrame, NaN);
        }
    }

    /**************************************************************************
    *** SAVE RESULTS **********************************************************
    **************************************************************************/

    char map_name[] = "map_pressure.?";
    map_name[13] = RefFrame;
    map_save(fid, map_name, mem, dims, xmin, xmax, ymin, ymax, spacing);
    free(mem);

    return 0;
}}

extern "C" {
#if 0 // dont break vim autoindent
}
#endif
int pressure_print(hid_t fid, TVec* points, int count)
{
    Space *S = new Space();
    S->load_hdf(fid);

    double dl = S->average_segment_length();
    TSortedTree tr(S, 8, dl*20, 0.1);
    MConvectiveFast conv(S, &tr);
    MEpsilonFast eps(S, &tr);
    MDiffusiveFast diff(S, &tr);
    MFlowmove fm(S);

    fm.vortex_shed();

    //дано: условие непротекания выполнено, неизвестные вихри найдены и рождены.
    //требуется: найти скорости вихрей (всех, включая присоединенные) и посчитать давление
    tr.build();
    eps.CalcEpsilonFast(false);
    conv.process_all_lists();
    diff.process_vort_list();

    for (int i=0; i<count; i++)
    {
        TVec p = points[i];
        double P = S->point_is_in_body(p) ?
            NaN : Pressure(S, &conv, p, 's', NaN);
        printf("%.6le\n", P);
    }

    return 0;
}}