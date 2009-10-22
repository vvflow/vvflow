#include "libVVHD/core.h"

#include "libVVHD/convective.h"
#include "libVVHD/convectivefast.h"
#include "libVVHD/diffusive.h"
#include "libVVHD/diffusivefast.h"
#include "libVVHD/merge.h"
#include "libVVHD/mergefast.h"
#include "libVVHD/utils.h"
#include "libVVHD/flowmove.h"

#include "iostream"
#include "fstream"
#include <stdio.h>
#include <unistd.h>
#include "math.h"
//#include <pthread.h>

#define BodyVortexes 400
#define RE 111
#define DT 0.005
#define print 10
#define M_2PI 6.2831853072
#define DFI M_2PI/BodyVortexes


using namespace std;

#define k 0.1
#define Amplitude 8.5
#define Frequency 1.7
double InfSpeedX(double t)
{
	//return 1-k/(t+k);
	if (t<k) return t/k; else return 1;
}

double Rotation(double t)
{
	if (t<k) return 0; else return Amplitude*sin(M_2PI*Frequency*(t-k));
}

void * diff (void* args)
{
	CalcVortexDiffusiveFast();
	return NULL;
}

int main()
{
	Space *S = new Space(1, 1, 1, InfSpeedX, NULL, Rotation);
	S->ConstructCircle(BodyVortexes);
	InitTree(S, 10, 4*DFI);
	InitFlowMove(S, DT, 1E-6);
	InitConvective(S, 1E-4);
	InitConvectiveFast(S, 1E-4);
	InitDiffusive(S, RE);
	InitDiffusiveFast(S, RE);
	InitMerge(S, DFI*DFI*0.09);
	InitMergeFast(S, DFI*DFI*0.09);

	fstream fout;
	char fname[64];
	sprintf(fname, "results/Forces");
	fout.open(fname, ios::out);
	fout.close();
	pthread_t thread;

	for (; S->Time < 100;)
	{
		BuildTree(1, 1, 0);
		CalcCirculationFast();
		DestroyTree();

		Merge();
		VortexFlight();
		//HeatFlight();

		BuildTree(1, 0, 1);
		pthread_create(&thread, NULL, &diff, NULL);
		CalcConvectiveFast();
		//CalcVortexDiffusiveFast();
		pthread_join(thread, NULL);
		DestroyTree();


		if (!(int(S->Time/DT)%print)) 
		{
			sprintf(fname, "results/data%06d.vort", int(S->Time/DT));
			fout.open(fname, ios::out);
			PrintVorticity(fout, S);
			fout.close();
/*			sprintf(fname, "results/data%04d.heat", i);
			fout.open(fname, ios::out);
			PrintHeat(fout, S);
			fout.close();
*/			cout << "Printing \"" << fname << "\" finished. \n"; 
			
			sprintf(fname, "results/Forces");
			fout.open(fname, ios::out | ios::app);
			fout << S->Time << "\t" << S->ForceX/DT/print << "\t" << S->ForceY/DT/print << endl;
			S->ForceX = S->ForceY = 0;
			fout.close();
		}

		MoveAndClean(1);

		cout << "Time = " << S->Time << "\t\t" << CleanedV() << "v cleaned. \t\t" << MergedV() << "v merged. \t\t" << S->VortexList->size << " vortexes.\n";
	}



	return 0;
}

