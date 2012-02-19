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

#include "omp.h"
#define dbg(a) a
//#define dbg(a) cerr << "Doing " << #a << "... " << flush; a; cerr << "done\n";
#define seek(f) while (fgetc(f)!='\n'){}
using namespace std;

char* infx_sh;
char* infy_sh;

char* gen_mask(char *mask, int n)
{
	//generates mask to be used with sscanf
	// mask consists of n "%*s" and 1 "%s"
	int i;
	for (i=0; i<n;i++)
	{
		sprintf(mask+i*4, "%%*s ");
	}
	sprintf(mask+i*4, "%%s");
	return mask;
}

double inf(const char *sh, double t) 
{
	if (!sh) return 0;
	double result;
	char *exec = (char*)malloc(1024);
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, sh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	dbg(if (!fscanf(pipe, "%lf", &result)) result=0;)
	pclose(pipe);

	free(exec);
	return result;
}

TVec InfSpeed(double t)
{
	const double k=1;
	return TVec(inf(infx_sh, t), inf(infy_sh, t));
}

void getenv_alert(bool warning, const char* var)
{
	fprintf(stderr, "%s. Environment variable \'%s\' isnt set. Type \'export %s=...\' to fix.\n", warning?"Warning":"Error", var, var);
}

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		cerr << "The only possible arg is filename to load.\n";
		cerr << "All regime info is read from environment variables:\n";
		cerr << "VV_name - Name of regime (uniq, no spaces)\n";
		cerr << "VV_body - Bodies file names (different filenames are separated with space)\n";
		cerr << "VV_infx - InfSpeedX (bash command, assuming $t is time)\n";
		cerr << "VV_infy - InfSpeedY --||--\n";
		cerr << "VV_1_nyu - \\frac{1}{\\nyu}\n";
		cerr << "VV_Pr (optional) - Prandtl Number (default = 1)\n";
		cerr << "VV_dt - dt\n";
		cerr << "VV_save_dt (optional) - dt between saves (default = 0.05)\n";
		cerr << "VV_streak (optional) - file with streak particles\n";
		cerr << "VV_streak_source (optional) - file with streak sources\n";
		cerr << "\ninfx example:\n";
		cerr << "echo \"print(\\\"%g\\\", $t<1?$t:1)\" | gnuplot - smooth start\n";
		return -1;
	}

	bool error = false;
	char *name_env = getenv("VV_name");
	char *body_env = getenv("VV_body");
	char *infx_env = getenv("VV_infx");
	char *infy_env = getenv("VV_infy");
	char *nyu_env = getenv("VV_1_nyu");
	char *Pr_env = getenv("VV_Pr");
	char *dt_env = getenv("VV_dt");
	char *save_dt_env = getenv("VV_save_dt");
	char *streak_env = getenv("VV_streak");
	char *streak_source_env = getenv("VV_streak_source");
	if (!name_env) { getenv_alert(false, "VV_name"); error = true; }
	if (!body_env) { getenv_alert(false, "VV_body"); error = true; }
	if (!infx_env) { getenv_alert(false, "VV_infx"); error = true; }
	if (!infy_env) { getenv_alert(false, "VV_infy"); error = true; }
	if (!nyu_env) { getenv_alert(false, "VV_1_nyu"); error = true; }
	if (!Pr_env) { getenv_alert(true, "VV_Pr"); }
	if (!dt_env) { getenv_alert(false, "VV_dt"); error = true; }
	if (!save_dt_env) { getenv_alert(true, "VV_save_dt"); }
	if (!streak_env) { getenv_alert(true, "VV_streak"); }
	if (!streak_source_env) { getenv_alert(true, "VV_streak_source"); }
	if (error) { cerr << "Exiting.\n"; return -1; }

	char dir[256]; sprintf(dir, "results_%s", name_env);
	char stepdata[256]; sprintf(stepdata, "stepdata_%s", name_env);
	char profile[256]; sprintf(profile, "profile_%s", name_env);
	infx_sh = infx_env ? infx_env : NULL;
	infy_sh = infy_env ? infy_env : NULL;

	double _1_nyu = atof(nyu_env);
	double Pr = Pr_env?atof(Pr_env):1;
	double dt = atof(dt_env);
	double save_dt = save_dt_env?atof(save_dt_env):0.05;

	mkdir(dir, 0777);

	FILE *f = fopen(stepdata, "a");
	#pragma omp parallel
	#pragma omp master
		fprintf(f, "Omp num threads = \t%d\n", omp_get_num_threads());
	fprintf(f, "Working dir = \t%s\n", dir);
	fprintf(f, "Body file = \t%s\n", body_env);
	fprintf(f, "InfSpeedX sh = \t%s\n", infx_env);
	fprintf(f, "InfSpeedY sh = \t%s\n", infy_env);
	fprintf(f, "1/nyu = \t%g\n", _1_nyu);
	fprintf(f, "Pr = \t%g\n", Pr);
	fprintf(f, "dt = \t%g\n", dt);
	fprintf(f, "save dt = \t%g\n", save_dt);
	fflush(f);

	/**************************************************************************/

	Space *S = new Space(InfSpeed);
	if (argc>=2)
	{
		S->Load(argv[1]);
		const_for(S->BodyList, llbody)
			delete *llbody;
		S->BodyList->clear();
		if (S->StreakList->size() < 2)
			if (streak_env) S->LoadStreak(streak_env);
		if (!S->StreakSourceList->size())
			if (streak_source_env) S->LoadStreakSource(streak_source_env);
	} else
	{
		S->StreakList->push_back(TObj(1000, 0, 0));
		if (streak_env) S->LoadStreak(streak_env);
		if (streak_source_env) S->LoadStreakSource(streak_source_env);
	}

	{char tmp[256], mask[256];
	while(sscanf(body_env, gen_mask(mask, S->BodyList->size()), tmp) > 0)
	{
		S->LoadBody(tmp);
	}}

	fprintf(f, "%-10s \t", "Time");
	for (int i=0; i<S->BodyList->size(); i++)
	fprintf(f, "%-20s \t%-20s \t%-20s \t%-20s \t%-20s \t%-20s \t%-20s \t",
			"Fx", "Fy", "Mz", "Friction_x", "Friction_y", "Friction_m", "Nusselt/L");
	fprintf(f, "%-10s \t%-10s \t%-10s\n", "N vorts", "N heats", "time");

	double dl = S->AverageSegmentLength();
	InitTree(S, 8, dl*20, 0.1);
	InitConvectiveFast(S, dl*dl/25);
	epsfast eps(S);
	diffusivefast diff(S, _1_nyu, Pr);
	flowmove fm(S, dt);

	while (true)
	{
		time_t begining = clock();
		dbg(BuildTree());
		dbg(CalcCirculationFast(true));
		dbg(DestroyTree());

		dbg(fm.HeatShed());

		if (divisible(S->Time, save_dt, dt/2))
		{
			char tmp[256]; sprintf(tmp, "%s/%%06d.vb", dir);
			S->Save(tmp);
		}

		dbg(fm.VortexShed());
		dbg(fm.StreakShed(save_dt));

		dbg(S->CalcForces());
		S->SaveProfile(profile, save_dt, val::Cp | val::Fr | val::Nu);
		fprintf(f, "%-10g \t", S->Time);
		const_for(S->BodyList, llbody)
		fprintf(f, "%-+20e \t%-+20e \t%-+20e \t%-+20e \t%-+20e \t%-+20e \t%-+20e \t",
		             (**llbody).Force.rx,
		             (**llbody).Force.ry,
		             (**llbody).Force.g,
		             (**llbody).Friction.rx,
		             (**llbody).Friction.ry,
		             (**llbody).Friction.g,
		             (**llbody).Nusselt);
		fprintf(f, "%-10ld \t%-10ld\t",
		             S->VortexList->size_safe(),
		             S->HeatList->size_safe());
		fflush(f);
		S->ZeroForces();

		dbg(BuildTree());
		dbg(eps.CalcEpsilonFast(true));
		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(diff.CalcVortexDiffusiveFast());
		dbg(diff.CalcHeatDiffusiveFast());
		dbg(DestroyTree());

		//FIXME move bodies 
		dbg(fm.MoveAndClean(true));
		dbg(fm.CropHeat());
		S->Time += S->dt;
		
		double steptime = double(clock()-begining)/CLOCKS_PER_SEC;
		fprintf(f, "%-10lf \n", steptime); fflush(f);
		fprintf(stderr, "%-10g \t%-10d \t%-10d \t%6.2lfs\r", S->Time, S->VortexList->size_safe(), S->HeatList->size_safe(), steptime);
	}

	fclose(f);
}
