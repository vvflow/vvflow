#include "libVVHD/core.h"
#include "libVVHD/convective.h"
#include "libVVHD/convectivefast.h"
#include "libVVHD/diffusive.h"
#include "libVVHD/diffusivefast.h"
#include "libVVHD/flowmove.h"
#include "libVVHD/utils.h"
#include "iostream"
#include "fstream"

using namespace std;

//#define HEAT //else vortexes
//#define FAST
#define RE 10
#define DT 1E-3
#define STEPS 50

#ifdef HEAT
	#define OBJ "HEAT"
	#define Nyu NyuH
	#ifdef FAST
		#define SPEED "FAST"
		#define OUTFILE "DataFast"
		#define InitConvectiveDef InitConvectiveFast
		#define InitDiffusiveDef InitDiffusiveFast
		#define CalcConvectiveDef CalcConvectiveFast
		#define CalcDiffusiveDef CalcHeatDiffusiveFast
	#else
		#define SPEED "SLOW"
		#define OUTFILE "DataSlow"
		#define InitConvectiveDef InitConvective
		#define InitDiffusiveDef InitDiffusive
		#define CalcConvectiveDef CalcConvective
		#define CalcDiffusiveDef CalcHeatDiffusive
	#endif
#else
	#define OBJ "VORTEXES"
	#define Nyu NyuV
	#ifdef FAST
		#define SPEED "FAST"
		#define OUTFILE "DataFast"
		#define InitConvectiveDef InitConvectiveFast
		#define InitDiffusiveDef InitDiffusiveFast
		#define CalcConvectiveDef CalcConvectiveFast
		#define CalcDiffusiveDef CalcVortexDiffusiveFast
	#else
		#define SPEED "SLOW"
		#define OUTFILE "DataSlow"
		#define InitConvectiveDef InitConvective
		#define InitDiffusiveDef InitDiffusive
		#define CalcConvectiveDef CalcConvective
		#define CalcDiffusiveDef CalcVortexDiffusive
	#endif
#endif


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

double NyuH(Space* S)
{
	if (!S->HeatList) return 0;
	double Summ = 0;
	int lsize = S->HeatList->size;
	TVortex* Vort = S->HeatList->Elements;

	for ( int i=0; i<lsize; i++ )
	{
		Summ += Vort->g * (Vort->rx*Vort->rx + Vort->ry*Vort->ry);
		Vort++;
	}

	return Summ;
}

int main()
{
	Space *S = new Space(1, 0, 1);
	char fname[16] = "steady1253";
	#ifdef HEAT
		S->LoadHeatFromFile(fname);
	#else
		S->LoadVorticityFromFile(fname);
	#endif
	InitTree(S, 10, 1E-8);
	InitConvectiveDef(S, 1E-4, 0);
	InitDiffusiveDef(S, RE);
	InitFlowMove(S, DT, 0);
	
	fstream fout;

	#ifdef HEAT
		cout << "Init done. Loaded " << S->HeatList->size << " " << OBJ << endl;
	#else
		cout << "Init done. Loaded " << S->VortexList->size << " " << OBJ << endl;
	#endif

	double Nyu1, Nyu2;
	Nyu1 = Nyu(S);
	for ( int k=0; k<STEPS; k++ )
	{
//		BuildTree(1, 0, 0);
//		CalcConvectiveDef();
		CalcDiffusiveDef();
			sprintf(fname, "treedata");
			fout.open(fname, ios::out);
//			PrintBottomNodes(fout);
			//fout << *S->VortexList << endl;
			fout.close();
//		DestroyTree();

		MoveAndClean(0);
//		cout << "step " << k << " done. Vorticity size = " << S->VortexList->size << endl;
	}
	Nyu2 = Nyu(S);

	sprintf(fname, OUTFILE);
	fout.open(fname, ios::out);
	#ifdef HEAT
		PrintHeat(fout, S, true);
	#else
		PrintVorticity(fout, S, true);
	#endif
	//fout << *S->VortexList << endl;
	fout.close();

	cout << "Doing " << OBJ << " " << SPEED << endl;
	cout << "InitRE=" << RE << "; RealRE=" << 4./ ((Nyu2-Nyu1)/(STEPS*DT)) << endl;

	return 0;
}
