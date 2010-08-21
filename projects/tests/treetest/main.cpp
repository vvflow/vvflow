#include "libVVHD/core.h"
#include "iostream"
#include "fstream"

using namespace std;

int main(int argc, char **argv)
{
	Space *S = new Space(1, 0, 0, NULL, NULL, NULL);
	S->LoadVorticityFromFile(argv[1]);

	char fname[32];
	InitTree(S, 5, 1E-4);
	S->ConstructCircle(2000);
	
	fstream fout;

	BuildTree(1, 0, 0);

		sprintf(fname, "%s.td", argv[1]);
		fout.open(fname, ios::out);
		PrintBottomNodes(fout, true);
		fout.close();

	DestroyTree();

	return 0;
}

