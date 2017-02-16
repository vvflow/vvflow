#include "iostream"
#include "fstream"
#include "stdio.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "core.h"
#include "epsfast.h"
#include "convectivefast.h"
#include "diffusivefast.h"
#include "flowmove.h"

#include "sensors.cpp"
#include "omp.h"
#define dbg(a) a
//#define dbg(a) cerr << "Doing " << #a << "... " << flush; a; cerr << "done\n";
#define seek(f) while (fgetc(f)!='\n'){}
using namespace std;

//#define OVERRIDEMOI 0.35

int main(int argc, char** argv)
{
    bool b_progress = false;
    bool b_save_profile = false;
    while (argc>1)
    {
        /**/ if (!strcmp(argv[1], "--progress")) b_progress = true;
        else if (!strcmp(argv[1], "--profile")) b_save_profile = true;
        else if (!strcmp(argv[1], "--version"))
        {
            cerr << "Git rev: " << Space::getGitRev() << std::endl;
            cerr << "Git info: " << Space::getGitInfo() << std::endl;
            cerr << "Git diff: " << Space::getGitDiff() << std::endl;
            return 0;
        }
        else break;
        argv++;
        argc--;
    }

    if (argc < 2)
    {
        cerr << "Missing filename to load. You can create it with vvcompose utility.\n";
        return -1;
    }

    Space *S = new Space();
    S->Load(argv[1]);

    // error checking
    #define RAISE(STR) { cerr << "vvflow ERROR: " << STR << endl; return -1; }
    if (S->Re <= 0) RAISE("invalid value re<=0");
    #undef RAISE
    bool is_viscous = (S->Re != std::numeric_limits<double>::infinity());

    char f_stepdata[256]; snprintf(f_stepdata, 256, "stepdata_%s.h5", S->caption.c_str());
    char f_results[256]; snprintf(f_results, 256, "results_%s", S->caption.c_str());
    char f_sensors_output[256]; snprintf(f_sensors_output, 256, "velocity_%s", S->caption.c_str());
    mkdir(f_results, 0777);

    Stepdata *stepdata = new Stepdata(S, f_stepdata, b_save_profile);

    double dl = S->AverageSegmentLength();
    double min_node_size = dl>0 ? dl*5 : 0;
    double max_node_size = dl>0 ? dl*100 : std::numeric_limits<double>::max();
    TSortedTree tr(S, 8, min_node_size, max_node_size);
    S->Tree = &tr;
    convectivefast conv(S);
    epsfast eps(S);
    diffusivefast diff(S);
    flowmove fm(S);
    sensors sens(S, &conv, (argc>2)?argv[2]:NULL, f_sensors_output);
    #ifdef OVERRIDEMOI
        S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
    #endif

    while (S->Time < S->Finish + S->dt/2)
    {
        if (S->BodyList.size())
        {
            dbg(tr.build());
            for (int iter=1;; iter++)
            {
                // решаем СЛАУ
                conv.CalcCirculationFast();
                if (!fm.DetectCollision(iter))
                    break;
                else if (iter>2)
                {
                    fprintf(stderr, "Collition is not resolved in 2 iterations\n");
                    break;
                }
            }
            dbg(tr.destroy());
        }

        dbg(fm.HeatShed());

        if (S->Time.divisibleBy(S->dt_save)  && (double(S->Time) > 0))
        {
            char tmp_filename[256]; snprintf(tmp_filename, 256, "%s/%%06d.h5", f_results);
            S->Save(tmp_filename);
            stepdata->flush();
        }

        dbg(fm.VortexShed());
        dbg(fm.StreakShed());

        dbg(S->CalcForces());
        stepdata->write();
        S->ZeroForces();

        dbg(tr.build());
        dbg(eps.CalcEpsilonFast(/*merge=*/is_viscous));
        dbg(conv.CalcBoundaryConvective());
        dbg(conv.CalcConvectiveFast());
        dbg(sens.output());
        if (is_viscous)
        {
            dbg(diff.CalcVortexDiffusiveFast());
            dbg(diff.CalcHeatDiffusiveFast());
        }
        dbg(tr.destroy());

        dbg(fm.MoveAndClean(true));
            #ifdef OVERRIDEMOI
                    S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
            #endif
        dbg(fm.CropHeat());
        S->Time = TTime::add(S->Time, S->dt);

        if (b_progress)
            fprintf(stderr, "\r%-10g \t%-10zd \t%-10zd", double(S->Time), S->VortexList.size(), S->HeatList.size());
    }

    if (b_progress)
        fprintf(stderr, "\n");
}
