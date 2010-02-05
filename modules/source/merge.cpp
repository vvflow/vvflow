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
	int i, j, lsize;
	TVortex *Vorti, *Vortj;
	int res=0;

	lsize = list->size;
	Vorti = list->Elements;
	double drx, dry, drabs2;

	for ( i=0; i<lsize; i++ )
	{
		Vortj = Vorti+1;
		double NearestDr = 1E10;
		TVortex* NearestV = Vortj;
		for ( j=i+1; j<lsize; j++ )
		{
			drx = Vorti->rx - Vortj->rx;
			dry = Vorti->ry - Vortj->ry;
			drabs2 = drx*drx + dry*dry;
			/*if ( drabs2 < NearestDr )
			{
				NearestDr = drabs2;
				NearestV = Vortj;
			}*/

			if ( drabs2 < Merge_SqEps )
			{
				res++;
				//cout << Vorti->rx << "\t" << Vorti->ry << "\t" <<  Vorti->g << "\t" <<  
					//	Vortj->rx << "\t" <<  Vortj->ry << "\t" <<  Vortj->g  << "\t";
				if ( ((Vorti->g > 0) && (Vortj->g > 0)) || ((Vorti->g < 0) && (Vortj->g < 0)) )
				{
					Vorti->g+= Vortj->g;
					double g1sum = 1/(Vorti->g + Vortj->g);
					Vorti->rx = (Vorti->g*Vorti->rx + Vortj->g*Vortj->rx)*g1sum;
					Vorti->ry = (Vorti->g*Vorti->ry + Vortj->g*Vortj->ry)*g1sum;
					//Vortj->flag |= 1;
				}
				else
				{
					
					if ( fabs(Vorti->g) < fabs(Vortj->g) )
					{
						Vorti->rx = Vortj->rx;
						Vorti->ry = Vortj->ry;
					}
					//Vortj->flag |= 1;
					Vorti->g += Vortj->g;
					//list->Remove(j); j--; lsize--;
				}
				//cout << Vorti->rx << "\t" << Vorti->ry << "\t" <<  Vorti->g << endl;
				list->Remove(j); lsize--;
				break;
			} else Vortj++;

		}
/*
		if ( ((Vorti->g > 0) && (NearestV->g < 0)) || ((Vorti->g < 0) && (NearestV->g > 0)) )
		{
			res++;
			Vorti->g += NearestV->g;
			if ( fabs(Vorti->g) < fabs(NearestV->g) )
			{
				Vorti->rx = NearestV->rx;
				Vorti->ry = NearestV->ry;
			}
			//Vortj->flag |= 1;
			list->Remove(NearestV);
		}
*/
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
