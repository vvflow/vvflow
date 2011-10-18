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

double RotSpeed = 0;
double rot(double t)
{
	return RotSpeed;
}

TVec InfSpeed(double t)
{
	const double k=1;
	if (t<k) return TVec(t/k, 0);
	else return TVec(1, 0);
}

int main(int argc, char** argv)
{
	if (argc<2) { cerr << "Not enough arguments" << endl; return -1; }
	RotSpeed = atof(argv[1]);
	Space *S = new Space(true, false, InfSpeed);
	S->LoadBody("cyl_600");
	//S->LoadBody("plate_bot");
	//S->LoadBody("plate_top");
	//S->LoadVorticityFromFile("t380.v");
	//S->Time = 380;
	TBody* body = S->BodyList->at(0);
	body->SetRotation(rot, TVec(0,0));

	double dl = body->AverageSegmentLength();
	const double dt = 2E-3;
	const double _1_dt = 1./dt;

	InitTree(S, 8, dl*20, 0.3);
	InitConvectiveFast(S, dl*dl/25);
	InitEpsilonFast(S, dl*dl*0.09);
	InitDiffusiveFast(S, 200);
	InitFlowMove(S, dt, 1E-8);

	fstream fout;
	string dir = "results_"+string(argv[1]);
	string stepdata = dir+string("_stepdata");
	fout.open(stepdata.c_str(), ios::out | ios::app);
	#pragma omp parallel
	{
		#pragma omp master
		fout << "Running in " << omp_get_num_threads() << " threads.\n";
	}
	fout << "Time\t Fx\t Fy\t N\t A\t RotV\n";
	fout.close();

	TVec ForceTmp(0, 0);
	while (true)
	{
		dbg(BuildTree());
		dbg(CalcCirculationFast(true));
		dbg(DestroyTree());

		if (!(int(S->Time/dt)%5))
		{
			double header[] = { S->Time, body->Angle, body->RotSpeed(S->Time), 
							    ForceTmp.rx/dt, ForceTmp.ry/dt };

			S->Save((dir+string("/%06d.vb")).c_str(), header, 5);
			ForceTmp.zero();
		}

		dbg(VortexShed());

		dbg(BuildTree());
		dbg(CalcEpsilonFast(true));
		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(CalcVortexDiffusiveFast());
		dbg(DestroyTree());

		dbg(MoveAndClean(true));

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
