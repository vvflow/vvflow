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
	fflush(f);

	/**************************************************************************/

	fprintf(f, "%-10s \t", "Time");
	for (int i=0; i<S->BodyList->size(); i++)
	fprintf(f, "%-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t %-20s\t ",
			"Fx", "Fy", "Mz", "Friction_x", "Friction_y", "Friction_m", "Nusselt/L",
			"PosX", "PosY", "Angle", "deltaPosX", "deltaPosY", "deltaAngle");
	fprintf(f, "%-10s\t %-10s\n", "N vorts", "N heats");

	double dl = S->AverageSegmentLength();
	InitTree(S, 8, dl*20, 0.1);
	convectivefast conv(S, dl*0.2);
	epsfast eps(S);
	diffusivefast diff(S);
	flowmove fm(S);
	sensors sens(S, (argc>2)?argv[2]:NULL, sensors_output);

	while (S->Time < S->Finish)
	{
		dbg(BuildTree());
		dbg(CalcCirculationFast());
		dbg(DestroyTree());

		dbg(fm.HeatShed());

		if (divisible(S->Time, S->save_dt, S->dt/4))
		{
			char tmp_filename[256]; sprintf(tmp_filename, "%s/%%06d.vb", dir);
			S->Save(tmp_filename);
		}

		dbg(fm.VortexShed());
		dbg(fm.StreakShed(S->streak_dt));

		dbg(S->CalcForces());
		S->SaveProfile(profile, S->profile_dt, val::Cp | val::Fr | val::Nu);
		fprintf(f, "%-10g \t", S->Time);
		const_for(S->BodyList, llbody)
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Force.rx,
		             (**llbody).Force.ry,
		             (**llbody).Force.g,
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
		fprintf(f, "%-10ld \t%-10ld \t",
		             S->VortexList->size_safe(),
		             S->HeatList->size_safe();
		fflush(f);
		S->ZeroForces();

		dbg(BuildTree());
		dbg(eps.CalcEpsilonFast(true));
		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(sens.output());
		dbg(diff.CalcVortexDiffusiveFast());
		dbg(diff.CalcHeatDiffusiveFast());
		dbg(DestroyTree());

		dbg(fm.MoveAndClean(true));
		dbg(fm.CropHeat());
		S->Time += S->dt;

		fprintf(stderr, "%-10g \t%-10d \t%-10d \t%6.2lfs\r", S->Time, S->VortexList->size_safe(), S->HeatList->size_safe(), steptime);
	}

	fclose(f);
}
