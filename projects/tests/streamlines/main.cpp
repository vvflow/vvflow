#include "libVVHD/core.h"
#include "libVVHD/diffmerge.h"
#include "libVVHD/diffmergefast.h"
#include "libVVHD/merge.h"
#include "libVVHD/mergefast.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"

#include "iostream"
#include "fstream"
#include <stdio.h>
#include <unistd.h>

using namespace std;

#define BodyVortexes 500
#define DFI 6.28/BodyVortexes

double NyuV(Space* S)
{
	if (!S->VortexList) return 0;
	double Summ = 0;
	int lsize = S->VortexList->size;
	TVortex* Vort = S->VortexList->Elements;

	for ( int i=0; i<lsize; i++ )
	{
		Summ += Vort->g * (Vort->rx*Vort->rx + Vort->ry*Vort->ry);
		Vort++;
	}

	return Summ;
}

int main()
{

	Space *S = new Space(1, 0, 0, NULL, NULL, NULL);
	S->LoadVorticityFromFile("input");

	InitTree(S, 10, 4*DFI);
	InitFlowMove(S, 1E-1, 1E-7);

	InitDiffMerge(S, 100, DFI*DFI*0.09);
	InitDiffMergeFast(S, 100, DFI*DFI*0.09);
	InitMerge(S, DFI*DFI*0.09);
	InitMergeFast(S, DFI*DFI*0.09);

	double A1 = NyuV(S);
	cout << "A1 = " << A1 << endl;

	BuildTree(1, 0, 0);

//	DiffMerge();
	DiffMergeFast();
//	Merge();
//	MergeFast();

	Clean();
	cout << "Merged " << DiffMergedV()+DiffMergedFastV()+MergedV()+MergedFastV() << endl;
	cout << "Cleaned " << CleanedV_toosmall() << endl;
	DestroyTree();
	double A2 = NyuV(S); 
	cout << "A2 = " << A2 << endl;
	cout << "error = " << (A2-A1)/A1 << endl;

	fstream fout;
	fout.open("output", ios::out);
	PrintVorticity(fout, S, true);
	fout.close();

	return 0;
}

