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
	if (argc < 2)
	{
		cerr << "Missing filename to load. You can create it with vvcompose utility.\n";
		return -1;
	}

	Space *S = new Space();
	S->Load(argv[1]);

	char dir[256]; sprintf(dir, "results_%s", S->name);
	char stepdata[256]; sprintf(stepdata, "stepdata_%s", S->name);
	char profile[256]; sprintf(profile, "profile_%s", S->name);
	char sensors_output[256]; sprintf(sensors_output, "velocity_%s", S->name);

	mkdir(dir, 0777);

	FILE *f = fopen(stepdata, "a");
	#pragma omp parallel
	#pragma omp master
		fprintf(f, "\nOMP_NUM_THREADS = \t%d\n", omp_get_num_threads());
	fprintf(f, "Working dir = \t%s\n", dir);
	fprintf(f, "Git commit  = \t%s\n", S->getGitInfo());
	fprintf(f, "Git diff    = \t%s\n", S->getGitDiff());
	fflush(f);

	/**************************************************************************/

	fprintf(f, "%-10s \t", "Time");
	for (int i=0; i<S->BodyList->size(); i++)
	fprintf(f, "Fx\t Fy\t Mz\t Friction_x\t Firc_y\t Fric_m\t NUsselt/L\t PosX\t PosY\t Angle\t DeltaPosX\t DeltaPosY\t DeltaAngle\t SpeedX\t SpeedY\t SpeedO\t ");
	fprintf(f, "Nv\t Nh \n");

	double dl = S->AverageSegmentLength();
	TSortedTree tr(S, 8, dl*5, dl*100);
	S->Tree = &tr;
	convectivefast conv(S);
	epsfast eps(S);
	diffusivefast diff(S);
	flowmove fm(S);
	sensors sens(S, &conv, (argc>2)?argv[2]:NULL, sensors_output);
	#ifdef OVERRIDEMOI
		S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
	#endif

	while (S->Time < S->Finish)
	{
		dbg(tr.build());
		dbg(conv.CalcCirculationFast());
		dbg(tr.destroy());

		dbg(fm.HeatShed());

		if (S->Time.divisibleBy(S->save_dt)  && (double(S->Time) > 0))
		{
			char tmp_filename[256]; sprintf(tmp_filename, "%s/%%06d.vb", dir);
			S->Save(tmp_filename);
		}

		dbg(fm.VortexShed());
		dbg(fm.StreakShed());

		dbg(S->CalcForces());
		if (S->Time > 0) S->SaveProfile(profile, val::Cp | val::Fr | val::Nu);
		fprintf(f, "%-10g \t", double(S->Time));
		const_for(S->BodyList, llbody)
		{
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Force_export.r.x,
		             (**llbody).Force_export.r.y,
		             (**llbody).Force_export.o,
		             (**llbody).Friction.r.x,
		             (**llbody).Friction.r.y,
		             (**llbody).Friction.o,
		             (**llbody).Nusselt);
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).pos.r.x,
		             (**llbody).pos.r.y,
		             (**llbody).pos.o,
		             (**llbody).dPos.r.x,
		             (**llbody).dPos.r.y,
		             (**llbody).dPos.o);
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Speed_slae.r.x,
		             (**llbody).Speed_slae.r.y,
		             (**llbody).Speed_slae.o);
		}
		fprintf(f, "%-10ld \t%-10ld \n",
		             S->VortexList->size_safe(),
		             S->HeatList->size_safe());
		fflush(f);

		S->ZeroForces();

		dbg(tr.build());
		dbg(eps.CalcEpsilonFast(true));
		dbg(conv.CalcBoundaryConvective());
		dbg(conv.CalcConvectiveFast());
		dbg(sens.output());
		dbg(diff.CalcVortexDiffusiveFast());
		dbg(diff.CalcHeatDiffusiveFast());
		dbg(tr.destroy());

		dbg(fm.MoveAndClean(true));
	        #ifdef OVERRIDEMOI
        	        S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
	        #endif
		dbg(fm.CropHeat());
		S->Time = TTime::add(S->Time, S->dt);

		fprintf(stderr, "\r%-10g \t%-10d \t%-10d", double(S->Time), S->VortexList->size_safe(), S->HeatList->size_safe());
	}

	fprintf(stderr, "\n");
	fclose(f);
}
