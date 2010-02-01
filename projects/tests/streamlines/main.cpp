#include "libVVHD/core.h"
#include "libVVHD/mergefast.h"
#include "libVVHD/utils.h"

#include "iostream"
#include "fstream"
#include <stdio.h>
#include <unistd.h>

using namespace std;

#define BodyVortexes 50
#define DFI 6.28/BodyVortexes

int main()
{

	Space *S = new Space(1, 1, 1, NULL, NULL, NULL);
	S->LoadVorticityFromFile("input");

	InitTree(S, 10, 4*DFI);
	InitMergeFast(S, DFI*DFI*0.01);

	BuildTree(1, 0, 1);
	MergeFast();
	cout << "Merged " << MergedFastV() << endl; 
	DestroyTree();

	fstream fout;
	fout.open("output", ios::out);
	PrintVorticity(fout, S, true);
	fout.close();

	return 0;
}

