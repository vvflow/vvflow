#include "libVVHD/core.h"
#include "libVVHD/mergefast.h"
#include "libVVHD/flowmove.h"

#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"

using namespace std;

int main()
{
	Space *S = new Space(1, 0, 0, NULL, NULL, NULL);
	char fname[16] = "1.vort";

	S->LoadVorticityFromFile(fname);
	
	InitTree(S, 10, 1E-8);
	InitFlowMove(S, 0, 1E-12);
	InitMergeFast(S, 0);

	cout << "Init done. Loaded " << S->VortexList->size << " vortexes" << endl;

	double Nyu1, Nyu2, Nyu3, g1, g2, g3;
	int m2, m3;
	fstream fout;

	Nyu1 = S->Integral();
	g1 = S->gsumm();

	BuildTree(1, 0, 0);
	MergeFast();
	DestroyTree();
	m2 = MergedFastV();
	Clean();

	fout.open("2.vort", ios::out);
	PrintVorticity(fout, S, false);
	fout.close();

	Nyu2 = S->Integral();
	g2 = S->gsumm();

	BuildTree(1, 0, 0);
	MergeFast();
	DestroyTree();
	m3 = MergedFastV();
	Clean();

	fout.open("3.vort", ios::out);
	PrintVorticity(fout, S, false);
	fout.close();

	Nyu3 = S->Integral();
	g3 = S->gsumm();

	cout << "Integral before = " << Nyu1 << " GSumm " << g1
		<< "\nIntegral after  = " << Nyu2 << " GSumm " << g2 << " Merged " << m2
		<< "\nIntegral trird  = " << Nyu3 << " GSumm " << g3 << " Merged " << m3 
		<< endl;

	return 0;
}
