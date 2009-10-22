#include <math.h>
#include "mergefast.h"

/********************* HEADER ****************************/

namespace {

Space *MergeFast_S;
double MergeFast_SqEps;
int MergeFast_MergedV;

int MergeVortexList(TNode *Node);
int MergeHeatList(TNode *Node);

} //end of namespace

/********************* SOURCE *****************************/

int InitMergeFast(Space *sS, double sSqEps)
{
	MergeFast_S = sS;
	MergeFast_SqEps = sSqEps;
}

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
						Vort->g+= NVort->g;
						double g1sum = 1/(Vort->g + NVort->g);
						Vort->rx = (Vort->g*Vort->rx + NVort->g*NVort->rx)*g1sum;
						Vort->ry = (Vort->g*Vort->ry + NVort->g*NVort->ry)*g1sum;
						NNode->VortexLList->Remove(j); j--; lsize--;
					}
					else
					{
						Vort->g += NVort->g;
						if ( fabs(Vort->g) < fabs(NVort->g) )
						{
							Vort->rx = NVort->rx;
							Vort->ry = NVort->ry;
						}
						NNode->VortexLList->Remove(j); j--; lsize--;
					}
				}
				lVort++;
			}
			lNVort++;
		}
		lNNode++;
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
		MergeFast_MergedV+= MergeVortexList(*lBNode);

		lBNode++;
	}

	return 0;
}

int MergedFastV()
{
	return MergeFast_MergedV;
}
