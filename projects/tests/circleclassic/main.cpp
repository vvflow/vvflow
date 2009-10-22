#include "libVVHD/core.h"
#include "libVVHD/convective.h"
#include "libVVHD/diffusive.h"
#include "libVVHD/merge.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"
#include <time.h>

using namespace std;

int main()
{
	Space *S = new Space(1, 1, 0);
	S->ConstructCircle(200);
	InitConvective(S, 1E-8, 1);
	InitDiffusive(S, 200);
	//InitMerge(S, 1E-6);
	InitFlowMove(S, 2E-1, 1E-7);

	
	fstream fout;
	time_t start, stop;

	time(&start);
	for ( int k=0; k<1; k++ )
	{
		CalcCirculation();
		VortexFlight();
		//Merge();
		//CalcConvective();
		CalcVortexDiffusive();
		
		char fname[16];
		sprintf(fname, "results/data%d", k);
		fout.open(fname, ios::out);
		PrintVorticity(fout, S, true);
		fout.close();
		cout << "step " << k << " done. " << S->VortexList->size << endl;

		MoveAndClean(1);
	}
	time(&stop);
	cout << (stop-start) << endl;
	

	return 0;
}

