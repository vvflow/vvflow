#include "libVVHD/core.h"
#include "libVVHD/convectivefast.h"
#include "libVVHD/diffusive.h"
#include "libVVHD/diffusivefast.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/merge.h"
#include "libVVHD/mergefast.h"
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
	S->ConstructCircle(BodyVortexes);
	InitTree(S, 10, 1E-4);
	InitConvectiveFast(S, 1E-4, 1);
	InitDiffusiveFast(S, 100);
	InitFlowMove(S, 2E-2, 1E-7);
	InitMerge(S, 3.14/BodyVortexes);

	
	fstream fout;
	time_t start, stop;

	cout << endl;
	time(&start);
	for ( int k=0; k<1000; k++ )
	{
//		cout << "Building tree... " << flush;
		BuildTree(1, 1, 0);
//		cout << "done.\n";
		//cout << "\tCirculation\tNN% = " << GetAverageNearNodesCount() << " " << GetMaxDepth() << endl;
		CalcCirculationFast();
		//Merge();
		DestroyTree();

		VortexFlight();

//		cout << "Building tree... " << flush;
		BuildTree(1, 0, 0);
//		cout << "done.\n";
		//cout << "\tSpeed\t\tNN% = " << GetAverageNearNodesCount() << " " << GetMaxDepth() << endl;
		CalcConvectiveFast();
		CalcVortexDiffusiveFast();
		DestroyTree();


		MoveAndClean(1);
		if ( !(k%1) ) 
		{
			char fname[16];
			sprintf(fname, "results/data%03d", k);
			fout.open(fname, ios::out);
			PrintVorticity(fout, S, true);
			//fout << *S->VortexList << endl;
			fout.close();
			cout << "step " << k << " done. Vorticity size = " << S->VortexList->size << endl;
		}

	}
	time(&stop);
	cout << "\nTotal time: " << (stop-start) << endl;
	

	return 0;
}

