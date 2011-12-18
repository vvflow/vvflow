#include "iostream"
#include "fstream"
#include "stdio.h"

#include <sys/stat.h>
#include <sys/types.h>

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

/*string InfXSpeed;
string InfYSpeed;
string RotSpeed;

double InfSpeedX(double t) 
{
	if ((!InfSpeedXsh) || (!*InfSpeedXsh)) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(InfSpeedXsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, InfSpeedXsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}*/

TVec InfSpeed(double t)
{
	return TVec(1,0);
	const double k=1;
	if (t<k) return TVec(t/k, 0);
	else return TVec(1, 0);
}

int main(int argc, char** argv)
{
	if (argc != 2) 
	{
		cerr << "Not enough arguments. Use: " << argv[0] << " regime.info [file.vb]\n";
		cerr << "regime info munt contain next lines (order matters ofc):\n";
		cerr << " - Name of regime (uniq, no spaces)\n - Body file name\n";
//		cerr << "Rotation speed (bash command, assuming $t is time, and axis = (0,0))\n";
		cerr << " - Reynolds number\n - dt\n";
		return -1;
	}

	FILE* finfo = fopen(argv[1], "r");
	char dir[256]; fscanf(finfo, "%s", dir); seek(finfo);
	char BodyFile[256]; fscanf(finfo, "%s", BodyFile); seek(finfo);
	//rotspeed seek(finfo);
	double Re; fscanf(finfo, "%lf", &Re); seek(finfo);
	double dt; fscanf(finfo, "%lf", &dt); seek(finfo);
	fclose(finfo);

	mkdir(dir, 0777);

	string stepdata = string(dir)+string("_stepdata");
	FILE *f = fopen(stepdata.c_str(), "a");
	#pragma omp parallel
	#pragma omp master
		fprintf(f, "Omp num threads = %d\n", omp_get_num_threads());
	fprintf(f, "Working dir = %s\n", dir);
	fprintf(f, "Body file = %s\n", BodyFile);
	fprintf(f, "Rotataon sh = \n");
	fprintf(f, "Reynolds = %g\n", Re);
	fprintf(f, "dt = %g\n", dt);
	fprintf(f, "%-10s \t%-20s \t%-20s \t%-20s \t%-10s \t%-10s \t%-10s \t%-10s\n",
			"Time", "Fx", "Fy", "Mz", "N vorts", "N heats", "Angle", "RotSpeed");
	//fclose(f);

	TVec ForceTmp(0, 0);

	/**************************************************************************/

	Space *S = new Space(InfSpeed);
	S->LoadBody(BodyFile);
	TBody* body = S->BodyList->at(0);
	//body->SetRotation(rot, TVec(0,0));

	double dl = S->AverageSegmentLength();
	const double _1_dt = 1./dt;

	InitTree(S, 8, dl*20, 0.3);
	InitConvectiveFast(S, dl*dl/25);
	epsfast eps(S);
	diffusivefast diff(S, Re, 1);
	flowmove fm(S, dt);
	S->StreakSourceList->push_back(TObj(-1.1, 0.3, 0));

	while (true)
	{
		dbg(BuildTree());
		dbg(CalcCirculationFast(true));
		dbg(DestroyTree());

		S->SaveProfile("prof", val::Cp | val::Fr | val::Nu);
		if (!(int(S->Time/dt)%10))
		{
			double header[] = { S->Time, body->Angle, body->RotSpeed(S->Time), 
							    ForceTmp.rx/dt, ForceTmp.ry/dt };
			S->Save((string(dir)+string("/%06d.vb")).c_str(), header, 5);
			ForceTmp.zero();
		}

		dbg(fm.VortexShed());
		dbg(fm.HeatShed());
		dbg(fm.StreakShed());

		//save profile
		const_for(S->BodyList, llbody)
		{
			(**llbody).zero_variables();
		}

//		f = fopen(stepdata.c_str(), "a");
//		if (!f) { perror("Error opening file"); return -2; }
		fprintf(f, "%-10g \t%-+20e \t%-+20e \t%-+20e \t%-10ld \t%-10ld \t%-10g \t%-10g\n",
		             S->Time, 
		             body->Force.rx/dt,
		             body->Force.ry/dt,
		             body->Force.g/dt,
		             S->VortexList->size_safe(),
		             S->HeatList->size_safe(),
		             body->Angle, 
		             body->RotSpeed(S->Time));
		fflush(f);
//		fclose(f);
		ForceTmp+= body->Force;
		body->Force.zero();

		dbg(BuildTree());
		dbg(eps.CalcEpsilonFast(true));
		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(diff.CalcVortexDiffusiveFast());
		dbg(diff.CalcHeatDiffusiveFast());
		dbg(DestroyTree());

		//FIXME move bodies 
		dbg(fm.MoveAndClean(true));
		S->Time += S->dt;
	}

	fclose(f);
}
