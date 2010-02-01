#include "libVVHD/core.h"
#include "libVVHD/convectivefast.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"

using namespace std;

int main()
{
	Space *S = new Space(1, 0, 0, NULL, NULL, NULL);
	InitTree(S, 10, 1E-4);
	InitConvectiveFast(S, 1E-8);
	InitFlowMove(S, 1E-1, 1E-7);
	TVortex V;
	fstream fout;
	
	InitVortex(V,  0.5,  0.5, 1);
	S->VortexList->Add(V);
	InitVortex(V, -0.5, -0.5, 1);
	S->VortexList->Add(V);
	InitVortex(V,  0.5, -0.5, 1);
	S->VortexList->Add(V);
	InitVortex(V, -0.5,  0.5, 1);
	S->VortexList->Add(V);
	
	fout.open("result", ios::out);
	for ( int k=0; k<3; k++ )
	{
		BuildTree(1, 0, 0);
		//cout << GetAverageNearNodesPercent()*100 << endl;
		//TlList *BottomNodes = GetTreeBottomNodes();
		//cout << BottomNodes->size << endl;
		CalcConvectiveFast();
		DestroyTree();
		PrintVorticity(fout, S, true);
		MoveAndClean(0);
		
//		fout << S->VortexList->Item(0) << "\t" << S->VortexList->Item(1) << endl;
//		fout << S->VortexList->Item(0) << "\t" << S->VortexList->Item(1) << "\t" \
				<< S->VortexList->Item(2) << "\t" << S->VortexList->Item(3) << endl;
	}
	fout.close();

	return 0;
}

