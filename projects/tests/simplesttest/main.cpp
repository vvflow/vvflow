#include "libVVHD/core.h"
#include "libVVHD/convective.h"
#include "libVVHD/diffusive.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"
#include <time.h>

using namespace std;

int main()
{
	Space *S = new Space(1, 1, 0);
	InitConvective(S, 1E-8, 1);
	InitDiffusive(S, 1000);
	InitFlowMove(S, 1E-1, 1E-7);
	S->ConstructCircle(400);
	
	fstream fout;
	time_t start, stop;

	time(&start);
	for ( int k=0; k<40; k++ )
	{
		CalcCirculation();
		VortexFlight();
		CalcConvective();
		CalcVortexDiffusive();
		
		char fname[16];
		sprintf(fname, "results/data%ddirty", k);
		fout.open(fname, ios::out);
		fout << *S->VortexList << endl;
		fout.close();
		cout << "step " << k << " done. " << S->VortexList->size << endl;

		MoveAndClean(1);

//		char fname[16];
		sprintf(fname, "results/data%d", k);
		fout.open(fname, ios::out);
		fout << *S->VortexList << endl;
		fout.close();
		cout << "step " << k << " done. " << S->VortexList->size << endl;
	}
	time(&stop);
	cout << (stop-start) << endl;
	

	return 0;
}

