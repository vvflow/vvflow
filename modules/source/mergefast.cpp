#include <math.h>
#include "mergefast.h"

/********************* HEADER ****************************/

namespace {

Space *MergeFast_S;
double MergeFast_SqEps;
int MergeFast_MergedV;

//int MergeVortexList(TNode *Node);
int MergeVortexList_NodeOnly(TNode *Node);
int MergeHeatList(TNode *Node);

} //end of namespace

/********************* SOURCE *****************************/

int InitMergeFast(Space *sS, double sSqEps)
{
	MergeFast_S = sS;
	MergeFast_SqEps = sSqEps;
}

/*
namespace {
int MergeVortexList(TNode *Node)
{
	if ( !Node->VortexLList ) return 0;
	int res=0;

	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;

	for ( int i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;

		if ( !NNode->VortexLList ) { lNNode++; continue; }
		TVortex** lNVort = (TVortex**)NNode->VortexLList->Elements; //link to Near Vortex
		TVortex* NVort;
		int lsize = NNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			NVort = *lNVort;
			
			TVortex** lVort = (TVortex**)Node->VortexLList->Elements; //link to Vortex from current node
			TVortex* Vort;
			int clsize = Node->VortexLList->size; // current list size
			for (int k=0; k<clsize; k++ )
			{
				Vort = *lVort;
				if ( Vort == NVort ) { lVort++; continue; }

				double drx, dry, drabs2;
				drx = Vort->rx - NVort->rx;
				dry = Vort->ry - NVort->ry;
				drabs2 = fabs(drx)+fabs(dry);

				if ( drabs2 < MergeFast_SqEps )
				{
					res++;
					if ( ((Vort->g > 0) && (NVort->g > 0)) || ((Vort->g < 0) && (NVort->g < 0)) )
					{
						double g1sum = 1/(Vort->g + NVort->g);
						Vort->rx = (Vort->g*Vort->rx + NVort->g*NVort->rx)*g1sum;
						Vort->ry = (Vort->g*Vort->ry + NVort->g*NVort->ry)*g1sum;
					}
					else
					{
						if ( fabs(Vort->g) < fabs(NVort->g) )
						{
							Vort->rx = NVort->rx;
							Vort->ry = NVort->ry;
						}
					}
					Vort->g+= NVort->g;
					NNode->VortexLList->Remove(j); j--; lsize--;
				}
				lVort++;
			}
			lNVort++;
		}
		lNNode++;
	}

	return res;
}}
*/

namespace {
int MergeVortexList_NodeOnly(TNode *Node)
{
	if ( !Node->VortexLList ) return 0;
	int res=0;

	TVortex** liVort = (TVortex**)Node->VortexLList->Elements;
	TVortex* iVort;
	int lsize = Node->VortexLList->size;

	for (int i=0; i<lsize; i++)
	{
		iVort = *liVort;

		TVortex** ljVort = liVort+1;
		TVortex* jVort;
		double n1, n2; n1=n2=1E10; //distance to two nearest vortexes
		TVortex **ln1, **ln2; ln1=ln2=NULL; //their links

		for (int j=i+1; j<lsize; j++)
		{
			jVort = *ljVort;

			double drx, dry, drabs2;
			drx = iVort->rx - jVort->rx;
			dry = iVort->ry - jVort->ry;
			drabs2 = drx*drx+dry*dry;

			if ( drabs2 < MergeFast_SqEps )
			{
				res++;

				if ( ((iVort->g > 0) && (jVort->g > 0)) || ((iVort->g < 0) && (jVort->g < 0)) )
				{
					double g1sum = 1/(iVort->g + jVort->g);
					iVort->rx = (iVort->g*iVort->rx + jVort->g*jVort->rx)*g1sum;
					iVort->ry = (iVort->g*iVort->ry + jVort->g*jVort->ry)*g1sum;
				}
				else
				{
					if ( fabs(iVort->g) < fabs(jVort->g) )
					{
						iVort->rx = jVort->rx;
						iVort->ry = jVort->ry;
					}
				}
				iVort->g += jVort->g;
				jVort->g = 0; Node->VortexLList->Remove(j); lsize--;
				break;
			}
			if ( drabs2 < n1 ) { n2=n1; n1=drabs2; ln2=ln1; ln1=ljVort; } else
			if ( drabs2 < n2 ) { n2=drabs2; ln2=ljVort; }
			ljVort++;
		}

		if (ln1 && ln2)
		if ( ((iVort->g<0)&&((**ln1).g>0)&&((**ln2).g>0)) || ((iVort->g>0)&&((**ln1).g<0)&&((**ln2).g<0)) )
		{
			res++;

			if ( fabs(iVort->g) < fabs((**ln1).g) )
			{
				iVort->rx = (**ln1).rx;
				iVort->ry = (**ln1).ry;
			}
			iVort->g += (**ln1).g;
			(**ln1).g = 0; Node->VortexLList->Remove((void**)ln1); lsize--;
		}

		liVort++;
	}

	return res;
}}

int MergeFast()
{
	MergeFast_MergedV = 0;

	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link

	for ( int i=0; i<bnlsize; i++ )
	{
		MergeFast_MergedV+= MergeVortexList_NodeOnly(*lBNode);

		lBNode++;
	}

	return 0;
}

int MergedFastV()
{
	return MergeFast_MergedV;
}
