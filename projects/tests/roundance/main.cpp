#include "libVVHD/core.h"
#include "libVVHD/convective.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"
#include <time.h>

using namespace std;

int main()
{
	Space *S = new Space(1E6, 200, 0);
	InitConvective(S, 1E-8, 0);
	InitFlowMove(S, 1E-2, 1E-7);
	S->ConstructCircle();
	TVortex V;
	
	fstream fout;
	InitVortex(V, 1.5, 0, 10);
	S->VortexList->Add(V);

	fout.open("res", ios::out);
	for ( int k=0; k<2000; k++ )
	{
		CalcCirculation();
		CalcConvective(1);
		fout << *S->VortexList << endl;
		MoveAndClean(1);

		//cout << "step " << k << " done. " << S->VortexList->size << endl;
	}
	fout.close();

	return 0;
}

