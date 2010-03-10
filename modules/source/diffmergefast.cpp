#include <math.h>
#include <cstdlib>
#include "diffmergefast.h"
#define expdef(x) exp(x)
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 		// = 2*PI

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffMergeFast_S;
double DiffMergeFast_Re;
double DiffMergeFast_Nyu;
double DiffMergeFast_DefaultEpsilon;
double DiffMergeFast_dfi;

double DiffMergeFast_MergeSqEps;
int DiffMergeFast_MergedV;

int MergeVortexes(TVortex **lv1, TVortex **lv2, TlList *LList2);

// Eps for vortexes
void EpsilonV(TNode *Node, TVortex **lv, double &res, bool merge=true);
void EpsilonV_faster(TNode *Node, TVortex **lv, double &res, bool merge=true);
//Eps for Heat
void EpsilonH(TNode *Node, double px, double py, double &res);
void EpsilonH_faster(TNode *Node, double px, double py, double &res);

void Division_vortex(TNode *Node, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD );
void Division_heat(TNode *Node, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD );

} //end of namespce

/********************* SOURCE *****************************/

int InitDiffMergeFast(Space *sS, double sRe, double sMergeSqEps)
{
	DiffMergeFast_S = sS;
	DiffMergeFast_Re = sRe;
	DiffMergeFast_Nyu = 1/sRe;
	DiffMergeFast_DefaultEpsilon = DiffMergeFast_Nyu * M_2PI;
	DiffMergeFast_MergeSqEps = sMergeSqEps;
	if (sS->BodyList) DiffMergeFast_dfi = M_2PI/sS->BodyList->size; else DiffMergeFast_dfi = 0;
	return 0;
}

namespace {
int MergeVortexes(TVortex **lv1, TVortex **lv2, TlList *LList2)
{
	DiffMergeFast_MergedV++;
	if (!lv1 || !lv2 || (lv1==lv2)) return -1;
	TVortex *v1, *v2;
	v1 = *lv1; v2 = *lv2;
	if (!v1 || !v2 || (v1==v2)) return -1;
	if ( ((v1->g > 0) && (v2->g > 0)) || ((v1 < 0) && (v2 < 0)) )
	{
		double g1sum = 1/(v1->g + v2->g);
		v1->rx = (v1->g*v1->rx + v2->g*v2->rx)*g1sum;
		v1->ry = (v1->g*v1->ry + v2->g*v2->ry)*g1sum;
	}
	else
	{
		if ( fabs(v1->g) < fabs(v2->g) )
		{
			v1->rx = v2->rx;
			v1->ry = v2->ry;
		}
	}
	v1->g+= v2->g; v2->g = 0;
	LList2->Remove((void**)lv2);
	return 0;
}}

int DiffMergedFastV()
{
	return DiffMergeFast_MergedV;
}

// EPSILON FUNCTIONS FOR VORTEXES

namespace {
void EpsilonV(TNode *Node, TVortex **lv, double &res, bool merge)
{
	double drx, dry, drabs2;
	double px=(**lv).rx, py=(**lv).ry; 
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	
	double res1, res2;
	res2 = res1 = 1E10;
	TVortex **lv1, **lv2;
	TNode *n1;
	lv1 = lv2 = NULL;
	n1 = NULL;
	for ( int i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }
		
		TVortex** lObj = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Obj;
		int lsize = NNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			drabs2 = drx*drx + dry*dry;
			if ( (res1 > drabs2) && drabs2 ) { res2 = res1; lv2 = lv1; res1 = drabs2; lv1 = lObj; n1 = NNode; } 
			else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; lv2 = lObj; }
			lObj++;
		}
		lNNode++;
	}

	res = sqrt(res2);
	if ( !lv || !lv1 ) { res=1E-10; return; }
	if ( !lv2 ) { res = sqrt(res1); return; }

	if (merge)
	{
		if ( (*lv<*lv1) && (res1 < (DiffMergeFast_MergeSqEps*( (*lv)->rx*(*lv)->rx + (*lv)->ry*(*lv)->ry + 3)*0.25 ) ) )
		{
			MergeVortexes(lv, lv1, n1->VortexLList);
			EpsilonV(Node, lv, res, false);
		} else if ( ( ((*lv)->g<0)&&((*lv1)->g>0)&&((*lv2)->g>0) ) || ( ((*lv)->g>0)&&((*lv1)->g<0)&&((*lv2)->g<0) ) )
		{
			MergeVortexes(lv, lv1, n1->VortexLList);
			EpsilonV(Node, lv, res, false);
		}
	}
}}

namespace {
void EpsilonV_faster(TNode *Node, TVortex **lv, double &res, bool merge)
{
	double drx, dry, drabs2;
	double px=(**lv).rx, py=(**lv).ry; 
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	
	double res1, res2;
	res2 = res1 = 1E10;
	TVortex **lv1, **lv2;
	TNode *n1;
	lv1 = lv2 = NULL;
	n1 = NULL;

	for ( int i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }
		
		TVortex** lObj = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Obj;
		int lsize = NNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			drabs2 = fabs(drx) + fabs(dry);
			if ( (res1 > drabs2) && drabs2 ) { res2 = res1; lv2 = lv1; res1 = drabs2; lv1 = lObj; n1 = NNode; } 
			else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; lv2 = lObj; }
			lObj++;
		}
		lNNode++;
	}

	res = res2;
	if ( !lv || !lv1 ) { res=1E-10; return; }
	if ( !lv2 ) { res=sqrt(res1); return; }

	if (merge)
	{
		if ( (*lv<*lv1) && (res1 < (DiffMergeFast_MergeSqEps*( (*lv)->rx*(*lv)->rx + (*lv)->ry*(*lv)->ry + 3)*0.25 )) )
		{
			MergeVortexes(lv, lv1, n1->VortexLList);
			EpsilonV_faster(Node, lv, res, false);
		} else if ( ( ((*lv)->g<0)&&((*lv1)->g>0)&&((*lv2)->g>0) ) || ( ((*lv)->g>0)&&((*lv1)->g<0)&&((*lv2)->g<0) ) )
		{
			MergeVortexes(lv, lv1, n1->VortexLList);
			EpsilonV_faster(Node, lv, res, false);
		}
	}
}}

// EPSILON FUNCTIONS FOR HEAT

namespace {
void EpsilonH(TNode *Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	
	double res1, res2;
	res2 = res1 = 1E10;
	for ( int i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->HeatLList ) { lNNode++; continue; }
		
		TVortex** lObj = (TVortex**)NNode->HeatLList->Elements;
		TVortex* Obj;
		int lsize = NNode->HeatLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			drabs2 = drx*drx + dry*dry;
			if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2; } 
			else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
			lObj++;
		}
		lNNode++;
	}
	res = sqrt(res2);
}}

namespace {
void EpsilonH_faster(TNode *Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	
	double res1, res2;
	res2 = res1 = 1E10;
	for ( int i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->HeatLList ) { lNNode++; continue; }
		
		TVortex** lObj = (TVortex**)NNode->HeatLList->Elements;
		TVortex* Obj;
		int lsize = NNode->HeatLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			drabs2 = fabs(drx) + fabs(dry);
			if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2; } 
			else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
			lObj++;
		}
		lNNode++;
	}
	res = res2;
}}

// OTHER FUNCTIONS

namespace {
void Division_vortex(TNode *Node, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double px=v->rx, py=v->ry;
	double xx, dxx;

	ResPX = 0;
	ResPY = 0;
	ResD = 0.;

	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	for ( int i=0 ; i<nnlsize; i++)
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }

		TVortex** lObj = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Obj;
		int lsize = NNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { lObj++; continue; }

			drabs = sqrt(drx*drx + dry*dry);
			double exparg = -drabs*eps1;
			if ( exparg > -8 )
			{
				xx = Obj->g * expdef(exparg); // look for define
				dxx = xx/drabs;
				ResPX += drx * dxx;
				ResPY += dry * dxx;
				ResD += xx;
			}

			lObj++;
		}
		lNNode++;
	}

	if ( ( (ResD < 0) && (v->g > 0) ) || ( (ResD > 0) && (v->g < 0) ) ) { ResD = v->g; }
}}

namespace {
void Division_heat(TNode *Node, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	double drx, dry, drabs;
	double xx, dxx;

	ResPX = 0;
	ResPY = 0;
	ResD = 0.;

	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	for ( int i=0 ; i<nnlsize; i++)
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }

		TVortex** lObj = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Obj;
		int lsize = NNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = px - Obj->ry;
			if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { lObj++; continue; }

			drabs = sqrt(drx*drx + dry*dry);
			double exparg = -drabs*eps1;
			if ( exparg > -8 )
			{
				xx = Obj->g * expdef(exparg); // look for define
				dxx = xx/drabs;
				ResPX += drx * dxx;
				ResPY += dry * dxx;
				ResD += xx;
			}

			lObj++;
		}
		lNNode++;
	}
}}

int CalcVortexDiffMergeFast()
{
	if ( !DiffMergeFast_S->VortexList) return -1;
	double multiplier;
	double Four_Nyu = 4 *DiffMergeFast_Nyu;
	double M_2PINyu = DiffMergeFast_Nyu * M_2PI;
	TList *S_BodyList = DiffMergeFast_S->BodyList;

	DiffMergeFast_MergedV = 0;
	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link
	TNode* BNode; //bottom node link

	for ( int i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		if ( !BNode->VortexLList ) { lBNode++; continue; }

		TVortex** lObj = (TVortex**)BNode->VortexLList->Elements;
		TVortex* Obj;
		long int *lsize = &(BNode->VortexLList->size);
		for ( long int j=0; j<*lsize; j++ )
		{
			Obj = *lObj;

			double epsilon, eps1;
			EpsilonV_faster(BNode, lObj, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD;
			Division_vortex(BNode, Obj, eps1, ResPX, ResPY, ResD);
 
			if ( fabs(ResD) > 1E-6 )
			{
				multiplier = M_2PINyu/ResD*eps1;
				Obj->vx += ResPX * multiplier;
				Obj->vy += ResPY * multiplier;
			}

			if ( S_BodyList ) // diffusion from cylinder only
			{
				double rabs, exparg;
				rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);
				
				exparg = (1-rabs)*eps1;
				if (exparg > -8)
				{
					multiplier = Four_Nyu * eps1 * expdef(exparg) / rabs;
					Obj->vx += Obj->rx * multiplier; 
					Obj->vy += Obj->rx * multiplier;
				}
			}

			lObj++;
		}

		lBNode++;
	}

	return 0;
}

int CalcHeatDiffMergeFast()
{
	if ( !DiffMergeFast_S->HeatList) return -1;
	double multiplier1, multiplier2;
	double M_7PIdfi2 = 21.9911485752 * DiffMergeFast_dfi * DiffMergeFast_dfi;
	double M_2PINyu = DiffMergeFast_Nyu * M_2PI;
	TList *S_BodyList = DiffMergeFast_S->BodyList;

	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link
	TNode* BNode; //bottom node link

	for ( int i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		if ( !BNode->HeatLList ) { lBNode++; continue; }

		TVortex** lObj = (TVortex**)BNode->HeatLList->Elements;
		TVortex* Obj;
		long int lsize = BNode->HeatLList->size;
		for ( long int j=0; j<lsize; j++ )
		{
			Obj = *lObj;

			double epsilon, eps1;
			EpsilonH_faster(BNode, Obj->rx, Obj->ry, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD;
			Division_heat(BNode, Obj->rx, Obj->ry, eps1, ResPX, ResPY, ResD);
 
			if ( fabs(ResD) > 1E-6 )
			{
				multiplier1 = M_2PINyu/ResD*eps1;
				Obj->vx += ResPX * multiplier1;
				Obj->vy += ResPY * multiplier1;
			}

			if ( S_BodyList ) // diffusion from cylinder only
			{
				double rabs, exparg;
				rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);
				
				exparg = (1-rabs)*eps1; //look for define
				if ( exparg > -8 )
				{
					multiplier1 = M_7PIdfi2*expdef(exparg);
					multiplier2 = DiffMergeFast_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
					Obj->vx += Obj->rx * multiplier2; 
					Obj->vy += Obj->ry * multiplier2;
				}
			}

			lObj++;
		}

		lBNode++;
	}

	return 0;
}
