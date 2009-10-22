#include "libVVHD/core.h"
#include "libVVHD/diffusivefast.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"
#include <time.h>

#define BodyVortexes 500

using namespace std;

int main()
{
	double ConvectiveFast_Eps = 0;
	Space *S = new Space(1, 1, 0);
	InitTree(S, 4, 1E-4);
	InitDiffusiveFast(S, 1000);
	InitFlowMove(S, 2E-2, 1E-7);
	InitMergeFast(S, 3.14/BodyVortexes);
	S->ConstructCircle(BodyVortexes);
	
	fstream fout;
	time_t start, stop;

	cout << endl;
	time(&start);
	for ( int k=0; k<50; k++ )
	{
		//cout << "Circulation calculation... " << flush;
		BuildTree(1, 1, 0);
		cout << "\tCirculation\tNN% = " << GetAverageNearNodesCount() << " " << GetMaxDepth() << endl;
		CalcCirculationFast();
		//MergeFast();
		DestroyTree();
		//cout << "done." << endl;

		//cout << "Flying... " << flush;
		VortexFlight();
		//cout << "done. " << CleanedV() << " vortexes removed" << endl;

		//cout << "Convective calculation... " << flush;
		BuildTree(1, 0, 0);
		cout << "\tSpeed\t\tNN% = " << GetAverageNearNodesCount() << " " << GetMaxDepth() << endl;
		CalcConvectiveFast();
		CalcVortexDiffusiveFast();
		DestroyTree();
		//cout << "done." << endl;

		//cout << "Moving... " << flush;

		//cout << "done. " << CleanedV() << " vortexes removed" << endl;

		if ( !(k%1) ) 
		{
			char fname[16];
			sprintf(fname, "results/data%d", k);
			fout.open(fname, ios::out);
			PrintVorticity(fout, S, true);
			//fout << *S->VortexList << endl;
			fout.close();
			cout << "step " << k << " done. Vorticity size = " << S->VortexList->size << endl;
		}

		MoveAndClean(1);
	}
	time(&stop);
	cout << "\nTotal time: " << (stop-start) << endl;
	

	return 0;
}

