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
	Space *S = new Space(1E6, 400, 0);
	InitConvective(S, 1E-8, 1);
	InitDiffusive(S, 1E3);
	InitFlowMove(S, 1E-1, 1E-7);
	S->ConstructCircle();
	
	time_t start, stop;

	time(&start);
	for ( int k=0; k<30; k++ )
	{
		CalcCirculation();
		VortexFlight();
		CalcConvective();
		CalcDiffusive();
		
		MoveAndClean(1);
	}
	time(&stop);
	cout << (stop-start) << endl;
	

	return 0;
}

