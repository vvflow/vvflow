#include <math.h>
#include "merge.h"
#include <iostream>

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *Merge_S;
double Merge_SqEps;
int Merge_MergedV;
int MergeList(TList *list);

} //end of namespace

/********************* SOURCE *****************************/

int InitMerge(Space *sS, double sSqEps)
{
	Merge_S = sS;
	Merge_SqEps = sSqEps;
}

namespace {
int MergeList(TList *list)
{
	TVortex *Vorti, *Vortj;
	int res=0;

	int lsize = list->size;
	Vorti = list->Elements;
	double drx, dry, drabs2;

	for ( int i=0; i<lsize; i++ )
	{
		Vortj = Vorti+1;
		double NearestDr = 1E10;
		TVortex* NearestV = Vortj;
		for ( int j=i+1; j<lsize; j++ )
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
				list->Remove(j); lsize--;
				break;
			} 

			Vortj++;
		}

		Vorti++;
	}

	return res;
}}

int Merge()
{
	if ( Merge_S->VortexList )
		Merge_MergedV = MergeList(Merge_S->VortexList);

	return 0;
}

int MergedV()
{
	return Merge_MergedV;
}
