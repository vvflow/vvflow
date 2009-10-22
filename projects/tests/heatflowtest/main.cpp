#include "libVVHD/core.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/diffusive.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"

#define BodyVortexes 400

using namespace std;

int main()
{
	Space *S = new Space(0, 1, 1);
	S->ConstructCircle(BodyVortexes);
	InitFlowMove(S, 1E-1, 1E-7);
	InitDiffusive(S, 1E2);

	
	fstream fout;

	for (int i=0; i<100; i++)
	{
		HeatFlight();
		CalcHeatDiffusive();

	char fname[64] = "results/data";
	sprintf(fname, "results/data%03d", i);
	fout.open(fname, ios::out);
	PrintHeat(fout, S);
	PrintBody(fout, S);
	fout.close();

		MoveAndClean(1);

		cout << "step " << i << " done. " << CleanedH() << " Cleaned\n";
	}



	return 0;
}

