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

#include <sys/time.h>

//#define dbg(a) a
//#define dbg(a) cerr << "Doing " << #a << "... " << flush; a; cerr << "done\n";
#define dbg(a) { timestamp_t ts=get_timestamp(); myst <<  #a << " "<< omp_get_max_threads() << " " << flush; a; myst << " " << (get_timestamp()-ts) << endl;}

#define seek(f) while (fgetc(f)!='\n'){}
using namespace std;

typedef unsigned long long timestamp_t;

static timestamp_t
get_timestamp ()
{
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}


//#define OVERRIDEMOI 0.35

int main(int argc, char** argv)
{
	ofstream myst;
	myst.open ("stat.txt",ios::app);

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
	fflush(f);

	/**************************************************************************/

	fprintf(f, "%-10s \t", "Time");
	for (int i=0; i<S->BodyList->size(); i++)
	fprintf(f, "Fx\t Fy\t Mz\t Friction_x\t Firc_y\t Fric_m\t NUsselt/L\t PosX\t PosY\t Angle\t DeltaPosX\t DeltaPosY\t DeltaAngle\t SpeedX\t SpeedY\t SpeedO\t ");
	fprintf(f, "Nv\t Nh \n");

	double dl = S->AverageSegmentLength();
	tree tr(S, 8, dl*20, 0.1);
	S->Tree = &tr;
	convectivefast conv(S, dl*0.2);
	epsfast eps(S);
	diffusivefast diff(S);
	flowmove fm(S);
	sensors sens(S, &conv, (argc>2)?argv[2]:NULL, sensors_output);
	#ifdef OVERRIDEMOI
		S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
	#endif

	int trololo=3;
	while (S->Time < S->Finish)
	{
		trololo--;

		dbg(tr.build());
		dbg(conv.CalcCirculationFast());
		dbg(tr.destroy());

		dbg(fm.HeatShed());

		if (divisible(S->Time, S->save_dt, S->dt/4)  && (S->Time > 0))
		{
			char tmp_filename[256]; sprintf(tmp_filename, "%s/%%06d.vb", dir);
			S->Save(tmp_filename);
		}

		dbg(fm.VortexShed());
		dbg(fm.StreakShed(S->streak_dt));

		dbg(S->CalcForces());
		if (S->Time > 0) S->SaveProfile(profile, S->profile_dt, val::Cp | val::Fr | val::Nu);
		fprintf(f, "%-10g \t", S->Time);
		const_for(S->BodyList, llbody)
		{
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Force_export.rx,
		             (**llbody).Force_export.ry,
		             (**llbody).Force_export.g,
		             (**llbody).Friction.rx,
		             (**llbody).Friction.ry,
		             (**llbody).Friction.g,
		             (**llbody).Nusselt);
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Position.rx,
		             (**llbody).Position.ry,
		             (**llbody).Angle,
		             (**llbody).deltaPosition.rx,
		             (**llbody).deltaPosition.ry,
		             (**llbody).deltaAngle);
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).MotionSpeed_slae.rx,
		             (**llbody).MotionSpeed_slae.ry,
		             (**llbody).RotationSpeed_slae);
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
		S->Time += S->dt;

		fprintf(stderr, "\r%-10g \t%-10d \t%-10d", S->Time, S->VortexList->size_safe(), S->HeatList->size_safe());
	}

	fprintf(stderr, "\n");
	fclose(f);

	myst.close();
}
