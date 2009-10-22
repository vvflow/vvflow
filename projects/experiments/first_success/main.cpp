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
//#include <pthread.h>

#define BodyVortexes 400
#define RE 400
#define DT 0.005
#define print 50
#define DFI 6.28/BodyVortexes

using namespace std;

double InfSpeedX(double t)
{
	#define k 0.1
	//return 1-k/(t+k);
	if (t<k) return t/k; else return 1;
}

void * diff (void* args)
{
	CalcVortexDiffusiveFast();
	return NULL;
}

int main()
{
	Space *S = new Space(1, 1, 1, InfSpeedX, NULL);
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
sprintf(fname, "Forces");
fout.open(fname, ios::out);
fout.close();
	pthread_t thread;

	for (int i=0; i<100000; i++)
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


		if (!(i%print)) 
		{
			sprintf(fname, "results/data%04d.vort", i);
			fout.open(fname, ios::out);
			PrintVorticity(fout, S, true);
			fout.close();
/*			sprintf(fname, "results/data%04d.heat", i);
			fout.open(fname, ios::out);
			PrintHeat(fout, S);
			fout.close();
*/			cout << "Printing \"" << fname << "\" finished. \n"; 
			
			sprintf(fname, "Forces");
			fout.open(fname, ios::out | ios::app);
			fout << S->Time << "\t" << S->ForceX/DT/print << "\t" << S->ForceY/DT/print << endl;
			S->ForceX = S->ForceY = 0;
			fout.close();
		}

		MoveAndClean(1);

		cout << "step " << i << " done. \t\t" << CleanedV() << "v cleaned. \t\t" << MergedV() << "v merged. \t\t" << S->VortexList->size << " vortexes.\n";
	}



	return 0;
}

