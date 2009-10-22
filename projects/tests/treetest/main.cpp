#include "libVVHD/core.h"
#include "iostream"
#include "fstream"

using namespace std;

int main()
{
	Space *S = new Space(1, 0, 0);
	char fname[16] = "lamb1253";
	S->LoadVorticityFromFile(fname);

	InitTree(S, 5, 1E-4);
	S->ConstructCircle(200);
	
	fstream fout;

	BuildTree(1, 0, 0);

		sprintf(fname, "treedata");
		fout.open(fname, ios::out);
		PrintBottomNodes(fout);
		fout.close();

	DestroyTree();

	return 0;
}

