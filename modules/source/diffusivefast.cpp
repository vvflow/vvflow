#include <math.h>
#include <cstdlib>
#include "diffusivefast.h"
#define expdef(x) exp(x)
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 		// = 2*PI

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffusiveFast_S;
double DiffusiveFast_Re;
double DiffusiveFast_Nyu;
double DiffusiveFast_DefaultEpsilon;
double DiffusiveFast_dfi;

// Eps for vortexes
void EpsilonV(TNode *Node, double px, double py, double &res);
void EpsilonV_faster(TNode *Node, double px, double py, double &res);
void EpsilonV_fastest(TNode *Node, double &res);
//Eps for Heat
void EpsilonH(TNode *Node, double px, double py, double &res);
void EpsilonH_faster(TNode *Node, double px, double py, double &res);
void EpsilonH_fastest(TNode *Node, double &res);

void Division_vortex(TNode *Node, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD );
void Division_heat(TNode *Node, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD );
} //end of namespce

/********************* SOURCE *****************************/

int InitDiffusiveFast(Space *sS, double sRe)
{
	DiffusiveFast_S = sS;
	DiffusiveFast_Re = sRe;
	DiffusiveFast_Nyu = 1/sRe;
	DiffusiveFast_DefaultEpsilon = DiffusiveFast_Nyu * M_2PI;
	if (sS->BodyList) DiffusiveFast_dfi = M_2PI/sS->BodyList->size; else DiffusiveFast_dfi = 0;
	return 0;
}

// EPSILON FUNCTIONS FOR VORTEXES

namespace {
void EpsilonV(TNode *Node, double px, double py, double &res)
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
			if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2; } 
			else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
			lObj++;
		}
		lNNode++;
	}

	if (res2 > 1E9)
	{
		TNode **lFNode = (TNode**)Node->FarNodes->Elements;
		TNode *FNode;
		int fnlsize = Node->FarNodes->size;

		for ( int i=0; i<fnlsize; i++)
		{
			FNode = *lFNode;
			if ( !FNode->VortexLList ) { lFNode++; continue; }
			drx = px - FNode->x;
			dry = py - FNode->y;
			drabs2 = drx*drx + dry*dry;
			if ( (res2+FNode->h+FNode->w) > drabs2 ) { lFNode++; continue; }

			TVortex** lObj = (TVortex**)FNode->VortexLList->Elements;
			TVortex* Obj;
			int lsize = FNode->VortexLList->size;

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

			lFNode++;
		}
	}
	res = sqrt(res2);
}}

namespace {
void EpsilonV_faster(TNode *Node, double px, double py, double &res)
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
			if ( (res1 > drabs2) && drabs2 ) { res2 = res1; res1 = drabs2; } 
			else if ( (res2 > drabs2) && drabs2 ) { res2 = drabs2; }
			lObj++;
		}
		lNNode++;
	}

	if (res2 > 1E9)
	{
		TNode **lFNode = (TNode**)Node->FarNodes->Elements;
		TNode *FNode;
		int fnlsize = Node->FarNodes->size;

		for ( int i=0; i<fnlsize; i++)
		{
			FNode = *lFNode;
			if ( !FNode->VortexLList ) { lFNode++; continue; }
			drx = px - FNode->x;
			dry = py - FNode->y;
			drabs2 = drx*drx + dry*dry;
			if ( (res2+FNode->h+FNode->w) > drabs2 ) { lFNode++; continue; }

			TVortex** lObj = (TVortex**)FNode->VortexLList->Elements;
			TVortex* Obj;
			int lsize = FNode->VortexLList->size;

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

			lFNode++;
		}
	}
	res = res2;
}}

namespace {
void EpsilonV_fastest(TNode *Node, double &res)
{
	double S=0;
	int i, n=0, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( NNode->VortexLList ) { S+= NNode->h*NNode->w; n+= NNode->VortexLList->size; }
		lNNode++;
	}
	if (n) 
		res = 2*sqrt(S/n); 
	else 
		; //count from far nodes
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
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

	if (res2 > 1E9)
	{
		TNode **lFNode = (TNode**)Node->FarNodes->Elements;
		TNode *FNode;
		int fnlsize = Node->FarNodes->size;

		for ( int i=0; i<fnlsize; i++)
		{
			FNode = *lFNode;
			if ( !FNode->HeatLList ) { lFNode++; continue; }
			drx = px - FNode->x;
			dry = py - FNode->y;
			drabs2 = drx*drx + dry*dry;
			if ( (res2+FNode->h+FNode->w) > drabs2 ) { lFNode++; continue; }

			TVortex** lObj = (TVortex**)FNode->HeatLList->Elements;
			TVortex* Obj;
			int lsize = FNode->HeatLList->size;

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

			lFNode++;
		}
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

	if (res2 > 1E9)
	{
		TNode **lFNode = (TNode**)Node->FarNodes->Elements;
		TNode *FNode;
		int fnlsize = Node->FarNodes->size;

		for ( int i=0; i<fnlsize; i++)
		{
			FNode = *lFNode;
			if ( !FNode->HeatLList ) { lFNode++; continue; }
			drx = px - FNode->x;
			dry = py - FNode->y;
			drabs2 = drx*drx + dry*dry;
			if ( (res2+FNode->h+FNode->w) > drabs2 ) { lFNode++; continue; }

			TVortex** lObj = (TVortex**)FNode->HeatLList->Elements;
			TVortex* Obj;
			int lsize = FNode->HeatLList->size;

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

			lFNode++;
		}
	}
	res = res2;
}}

namespace {
void EpsilonH_fastest(TNode *Node, double &res)
{
	double S=0;
	int i, n=0, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( NNode->HeatLList ) { S+= NNode->h*NNode->w; n+= NNode->HeatLList->size; }
		lNNode++;
	}
	if (n) 
		res = 2*sqrt(S/n); 
	else 
		; //count from far nodes
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
}}

// OTHER FUNCTIONS

namespace {
void Division_vortex(TNode *Node, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	int i, j;
	double drx, dry, drabs;
	double px=v->rx, py=v->ry;
	double xx, dxx;

	ResPX = 0;
	ResPY = 0;
	ResD = 0.;

	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	for ( i=0 ; i<nnlsize; i++)
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }

		TVortex** lObj = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Obj;
		int lsize = NNode->VortexLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			if ( (fabs(drx) < 1E-6) || (fabs(dry) < 1E-6) ) { Obj++; continue; }
			drabs = sqrt(drx*drx + dry*dry);

			double exparg = -drabs*eps1;
			if ( exparg > -8 )
			{
				xx = Obj->g * expdef(-drabs*eps1); // look for define
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
void Division_heat(TNode *Node, TVortex* v, double eps1, double &ResPX, double &ResPY, double &ResD )
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
		if ( !NNode->HeatLList ) { lNNode++; continue; }

		TVortex** lObj = (TVortex**)NNode->HeatLList->Elements;
		TVortex* Obj;
		int lsize = NNode->HeatLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			if ( (fabs(drx) < 1E-6) && (fabs(dry) < 1E-6) ) { Obj++; continue; }
			drabs = sqrt(drx*drx + dry*dry);

			double exparg = -drabs*eps1;
			if ( exparg > -8 )
			{
				xx = Obj->g * expdef(-drabs*eps1); // look for define
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

int CalcVortexDiffusiveFast()
{
	if ( !DiffusiveFast_S->VortexList) return -1;

	double multiplier;
	double M_2PINyu = DiffusiveFast_Nyu * M_2PI;
	double Four_Nyu = 4 * DiffusiveFast_Nyu;
	TList* S_BodyList = DiffusiveFast_S->BodyList;

	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link
	TNode* BNode; //bottom node link

	for ( int i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		if ( !BNode->VortexLList ) { lBNode++; continue; }

/*		double epsilon, eps1;
		EpsilonV_fastest(BNode, epsilon);
		eps1 = 1/epsilon;
*/
		TVortex** lObj = (TVortex**)BNode->VortexLList->Elements;
		TVortex* Obj;
		int lsize = BNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;

			double epsilon, eps1;
			EpsilonV_faster(BNode, Obj->rx, Obj->ry, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD, ResVx, ResVy, ResAbs2;
			Division_vortex(BNode, Obj, eps1, ResPX, ResPY, ResD);

			if ( fabs(ResD) > 1E-6 )
			{
				multiplier = M_2PINyu/ResD*eps1;
				ResVx = ResPX * multiplier;
				ResVy = ResPY * multiplier;
				ResAbs2 = ResVx*ResVx+ResVy*ResVy;
				if (ResAbs2 > 10000) { multiplier*=100/sqrt(ResAbs2); }
				Obj->vx += ResPX * multiplier;
				Obj->vy += ResPY * multiplier;
//cout << "d " << Obj->rx << " " << Obj->ry << " " << Obj->g << " " << ResPX * multiplier << " " << ResPY * multiplier << " " << epsilon << endl;
			}

			if ( S_BodyList ) // diffusion from cylinder only
			{
				double rabs, r1abs, exparg;
				rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);
				
				exparg = (1-rabs)*eps1;
				if (exparg > -8)
				{
					multiplier = Four_Nyu * eps1 * expdef(exparg)/rabs;
					Obj->vx += Obj->rx * multiplier;
					Obj->vy += Obj->ry * multiplier;
//cout << "db " << Obj->rx << " " << Obj->ry << " " << Obj->g << " " << Obj->rx * multiplier << " " << Obj->ry * multiplier << " " << epsilon << endl;
				}
			}

			lObj++;
		}

		lBNode++;
	}

	return 0;
}

int CalcHeatDiffusiveFast()
{
	if ( !DiffusiveFast_S->HeatList) return -1;

	double multiplier1, multiplier2;
	double M_2PINyu = DiffusiveFast_Nyu * M_2PI;
	double M_7PIdfi2 = 21.9911485752 * DiffusiveFast_dfi * DiffusiveFast_dfi;
	TList* S_BodyList = DiffusiveFast_S->BodyList;

	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link
	TNode* BNode; //bottom node link

	for ( int i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		if ( !BNode->HeatLList ) { lBNode++; continue; }

/*		double epsilon, eps1;
		EpsilonV_fastest(BNode, epsilon);
		eps1 = 1/epsilon;
*/
		TVortex** lObj = (TVortex**)BNode->HeatLList->Elements;
		TVortex* Obj;
		int lsize = BNode->HeatLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Obj = *lObj;

			double epsilon, eps1;
			EpsilonH_faster(BNode, Obj->rx, Obj->ry, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD;
			Division_heat(BNode, Obj, eps1, ResPX, ResPY, ResD);

			if ( fabs(ResD) > 1E-12 )
			{
				multiplier1 = M_2PINyu/ResD*eps1;
				Obj->vx += ResPX * multiplier1;
				Obj->vy += ResPY * multiplier1;
			}

			if ( S_BodyList ) // diffusion from cylinder only
			{
				double rabs, exparg;
				rabs = sqrt(Obj->rx*Obj->rx + Obj->ry*Obj->ry);

				exparg = (1-rabs)*eps1;
				if (exparg > -8)
				{
					multiplier1 = M_7PIdfi2*expdef(exparg);
					multiplier2 = DiffusiveFast_Nyu*eps1*multiplier1/(ResD+multiplier1)/rabs;
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

