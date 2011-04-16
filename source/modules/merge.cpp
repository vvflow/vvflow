#include <math.h>
#include "merge.h"
#include <iostream>

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Merge_S;
double Merge_SqEps;
int Merge_MergedV;
int MergeList(TList<TObject> *List);

} //end of namespace

/********************* SOURCE *****************************/

int InitMerge(Space *sS, double sSqEps)
{
	Merge_S = sS;
	Merge_SqEps = sSqEps;
	return 0;
}

namespace {
int MergeList(TList<TObject> *List)
{
	int res=0;
	double drx, dry, drabs2;

	TObject *Vorti = List->First;
	TObject *&LastVort = List->Last;
	for ( ; Vorti<LastVort; Vorti++ )
	{
		TObject *Vortj = Vorti+1;
		for ( ; Vortj<LastVort; Vortj++ )
		{
			drx = Vorti->rx - Vortj->rx;
			dry = Vorti->ry - Vortj->ry;
			drabs2 = drx*drx + dry*dry;

			if ( drabs2 < Merge_SqEps )
			{
				res++;
				if ( ((Vorti->g > 0) && (Vortj->g > 0)) || ((Vorti->g < 0) && (Vortj->g < 0)) )
				{
					double g1sum = 1/(Vorti->g + Vortj->g);
					Vorti->rx = (Vorti->g*Vorti->rx + Vortj->g*Vortj->rx)*g1sum;
					Vorti->ry = (Vorti->g*Vorti->ry + Vortj->g*Vortj->ry)*g1sum;
				}
				else
				{
					if ( fabs(Vorti->g) < fabs(Vortj->g) )
					{
						Vorti->rx = Vortj->rx;
						Vorti->ry = Vortj->ry;
					}
				}
				Vorti->g+= Vortj->g;
				List->Remove(Vortj);
				Vortj--;
				break;
			}
		}
	}

	return res;
}}

int Merge()
{
	Merge_MergedV = (Merge_S->VortexList) ? MergeList(Merge_S->VortexList):0;
	return 0;
}

int MergedV()
{
	return Merge_MergedV;
}
