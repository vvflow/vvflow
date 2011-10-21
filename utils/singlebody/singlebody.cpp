#include "iostream"
#include "fstream"
#include "stdio.h"

#include "core.h"
#include "epsfast.h"
#include "convectivefast.h"
#include "diffusivefast.h"
#include "flowmove.h"

#include "omp.h"
#define dbg(a) a
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
	const double k=1;
	if (t<k) return TVec(t/k, 0);
	else return TVec(1, 0);
}

int main(int argc, char** argv)
{
	if (argc != 2) 
	{
		cerr << "Not enough arguments. Use: " << argv[0] << " regime.info\n";
		cerr << "regime info munt contain next lines (order matters ofc):\n";
		cerr << "Name of regime (uniq, no spaces)\nBody file name\n";
		cerr << "Rotation speed (bash command, assuming $t is time, and axis = (0,0))\n";
		cerr << "Reynolds number\ndt\n";
		return -1;
	}

	FILE* finfo = fopen(argv[1], "r");
	char dir[256]; fscanf(finfo, "%s", dir); while (fgetc(finfo)!='\n'){}
	char BodyFile[256]; fscanf(finfo, "%s", BodyFile); while (fgetc(finfo)!='\n'){}
	fscanf(finfo, "%s", dir); while (fgetc(finfo)!='\n'){}
	//rotspeed
	double Re; fscanf(finfo, "%lf", &Re); while (fgetc(finfo)!='\n'){}
	double dt; fscanf(finfo, "%lf", &dt); while (fgetc(finfo)!='\n'){}

	fstream fout;

	string stepdata = string(dir)+string("_stepdata");
	fout.open(stepdata.c_str(), ios::out | ios::app);
	#pragma omp parallel
	{
		#pragma omp master
		fout << "Running in " << omp_get_num_threads() << " threads.\n";
	}
	fout << "Time\t Fx\t Fy\t Mz\t N\t A\t RotV\n";
	fout.close();

	TVec ForceTmp(0, 0);

	/**************************************************************************/

	Space *S = new Space(true, false, InfSpeed);
	S->LoadBody("cyl_600");
	TBody* body = S->BodyList->at(0);
	//body->SetRotation(rot, TVec(0,0));

	double dl = body->AverageSegmentLength();
	const double _1_dt = 1./dt;

	InitTree(S, 8, dl*20, 0.3);
	InitConvectiveFast(S, dl*dl/25);
	InitEpsilonFast(S, dl*dl*0.09);
	InitDiffusiveFast(S, 200);
	flowmove fm(S, dt);

	while (true)
	{
		dbg(BuildTree());
		dbg(CalcCirculationFast(true));
		dbg(DestroyTree());

		if (!(int(S->Time/dt)%10))
		{
			double header[] = { S->Time, body->Angle, body->RotSpeed(S->Time), 
							    ForceTmp.rx/dt, ForceTmp.ry/dt };

			S->Save((dir+string("/%06d.vb")).c_str(), header, 5);
			ForceTmp.zero();
		}

		dbg(fm.VortexShed());

		dbg(BuildTree());
		dbg(CalcEpsilonFast(true));
		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(CalcVortexDiffusiveFast());
		dbg(DestroyTree());

		//FIXME move bodies 
		dbg(fm.MoveAndClean(true));

		S->Time += S->dt; //S->FinishStep();
		ForceTmp+= body->Force;

		fout.open(stepdata.c_str(), ios::out | ios::app);
		fout << S->Time << " \t";
		fout << body->Force/dt << " \t"; body->Force.zero();
		fout << S->VortexList->size() << " \t";
		fout << body->Angle << "\t";
		fout << body->RotSpeed(S->Time) << endl;
		fout.close();
	}
}
