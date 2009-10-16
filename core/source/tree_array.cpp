#include "tree.h"
#include "core.h"
#include "math.h"
#include "iostream"
using namespace std;

#define M_2PI 6.283185308 		// = 2*PI

/********************* HEADER ****************************/
namespace {

Space *Tree_S;
int Tree_TotalDepth;
int Tree_CurrentDepth;
int Tree_FarCriteria;
double Tree_MinNodeSize;

TNode ***Tree_Elements;
TlList *Tree_BottomNodes;

void DeleteNode(TNode* Node);
void Stretch(TNode* Node);
TNode* CreateNode(int VortexListSize, int BodyListSize, int HeatListSize);
void DistributeObjects(TlList* ParentList, double NodeC, TlList* &ch1, TlList* &ch2, char side);
void CalculateCMass(int i, int j);
void CalculateCMass(TNode* Node);
void FindNearNodes(TNode* BottomNode, int i, int j);

} //end of namespace

/********************* SOURCE *****************************/

int InitTree(Space *sS, int sTotalDepth, int sFarCriteria, double sMinNodeSize)
{
	Tree_S = sS;
	Tree_TotalDepth = sTotalDepth;
	Tree_FarCriteria = sFarCriteria;
	Tree_MinNodeSize = sMinNodeSize;

	Tree_Elements = (TNode***)new Pointer[Tree_TotalDepth];
	int i , j, LineLen = 1;
	TNode*** LineLink = Tree_Elements;
	TNode** NodeLink;
	for ( i=0; i<=Tree_TotalDepth; i++ )
	{
		*LineLink = (TNode**)new Pointer[LineLen];
		NodeLink = *LineLink;
		for ( j=0; j<LineLen; j++ )
		{
			*NodeLink = NULL;
			NodeLink++;
		}
		LineLink++;
		LineLen = LineLen << 1; // LineLen= pow(2, i); 
	}
}

int BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat)
{
	int i, j, maxj;
	double NodecX, NodecY, NodeH, NodeW;  // center, height & width
	TNode* &RootNode = **Tree_Elements;
	RootNode = new TNode;

	//Filling root node
	#define FillRootNode(Include, SList, RootLList) 						\
		if ( (Include) && (SList) ) 										\
		{ 																	\
			if ( (SList)->size ) 											\
			{ 																\
				int i, lsize = (SList)->size; 								\
				TlList* &RootList = (RootLList); 							\
				RootList = new TlList(lsize); 								\
				TVortex* Obj = (TVortex*)(SList)->Elements; 				\
				for ( i=0; i<lsize; i++ ) 									\
				{ 															\
					RootList->Add(Obj); 									\
					Obj++; 													\
				} 															\
			} else { (RootLList) = NULL; } 									\
		} else { (RootLList) = NULL; } 										\

	FillRootNode(IncludeVortexes, Tree_S->VortexList, RootNode->VortexLList)
	FillRootNode(IncludeBody, Tree_S->BodyList, RootNode->BodyLList)
	FillRootNode(IncludeHeat, Tree_S->HeatList, RootNode->HeatLList)
	RootNode->NearNodes = NULL;
	RootNode->FarNodes = NULL;
	RootNode->i = RootNode->j = 0; //DEBUG

	int sumsize=0;
	if ( RootNode->VortexLList ) sumsize+= RootNode->VortexLList->size;
	if ( RootNode->BodyLList ) sumsize+= RootNode->BodyLList->size;
	if ( RootNode->HeatLList ) sumsize+= RootNode->HeatLList->size;
	if ( !sumsize ) 
		Tree_CurrentDepth = 0;
	else
		Tree_CurrentDepth = floor(log(sumsize)/log(2))-1;

	if( Tree_CurrentDepth > Tree_TotalDepth ) Tree_CurrentDepth = Tree_TotalDepth;
	Tree_BottomNodes = new TlList(1 << Tree_CurrentDepth); //!!!!!!!!!!!!!!!! 2^Tree_CurrentDepth;

	Stretch(RootNode);

	maxj = 1;
	for ( i=0; i<Tree_CurrentDepth; i++ )
	{
		for ( j=0; j<maxj; j++ )
		{
			TNode* &ParentNode = Tree_Elements[i][j];
			if ( !ParentNode ) continue;
			/*NodecX = 0.5*(ParentNode->r + ParentNode->l);
			NodecY = 0.5*(ParentNode->t + ParentNode->b);
			NodeW = ParentNode->r - ParentNode->l;
			NodeH = ParentNode->t - ParentNode->b;*/
			NodecX = ParentNode->x;
			NodecY = ParentNode->y;
			NodeH = ParentNode->h;
			NodeW = ParentNode->w;

			if ( (NodeH < Tree_MinNodeSize) || (NodeW < Tree_MinNodeSize) ) 
			{
				Tree_BottomNodes->Add(ParentNode);
				continue;
			}
			TNode* &Child1 = Tree_Elements[i+1][(j << 1)];
			TNode* &Child2 = Tree_Elements[i+1][(j << 1) | 1];

			int ParentVortexListSize = 0;
			int ParentBodyListSize = 0;
			int ParentHeatListSize = 0;
			if ( ParentNode->VortexLList ) ParentVortexListSize = ParentNode->VortexLList->size;
			if ( ParentNode->BodyLList ) ParentBodyListSize = ParentNode->BodyLList->size;
			if ( ParentNode->HeatLList ) ParentHeatListSize = ParentNode->HeatLList->size;
			Child1 = CreateNode(ParentVortexListSize, ParentBodyListSize, ParentHeatListSize);
			Child2 = CreateNode(ParentVortexListSize, ParentBodyListSize, ParentHeatListSize);
			Child1->i = Child2->i = i+1; //DEBUG
			Child1->j = j << 1; Child2->j = (j << 1) | 1; //DEBUG

			if ( NodeH < NodeW )
			{
				DistributeObjects(ParentNode->VortexLList, NodecX, Child1->VortexLList, Child2->VortexLList, 'x');
				DistributeObjects(ParentNode->BodyLList, NodecX, Child1->BodyLList, Child2->BodyLList, 'x');
				DistributeObjects(ParentNode->HeatLList, NodecX, Child1->HeatLList, Child2->HeatLList, 'x');
			}
			else
			{
				DistributeObjects(ParentNode->VortexLList, NodecY, Child1->VortexLList, Child2->VortexLList, 'y');
				DistributeObjects(ParentNode->BodyLList, NodecY, Child1->BodyLList, Child2->BodyLList, 'y');
				DistributeObjects(ParentNode->HeatLList, NodecY, Child1->HeatLList, Child2->HeatLList, 'y');
			}

			Stretch(Child1);
			Stretch(Child2);
			delete ParentNode->VortexLList; ParentNode->VortexLList = NULL;
			delete ParentNode->BodyLList; ParentNode->BodyLList = NULL;
			delete ParentNode->HeatLList; ParentNode->HeatLList = NULL;
		}
		maxj= maxj << 1; // maxj = 2^i
	}

	TNode** BottomNode = Tree_Elements[Tree_CurrentDepth];
	for ( j=0; j<maxj; j++ ) // here maxj=2^Tree_CurrentDepth
	{
		if ( *BottomNode )
		{
			Tree_BottomNodes->Add(*BottomNode);
		}
		BottomNode++;
	}

	CalculateCMass(0,0);
	BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( i=0; i<lsize; i++ )
	{
		(*BottomNode)->NearNodes = new TlList(lsize);
		(*BottomNode)->FarNodes = new TlList(lsize);
		FindNearNodes(*BottomNode, 0, 0);
		BottomNode++;
	}
}

namespace {
void DeleteNode(TNode* Node)
{
	
	if ( !Node ) return;
	if ( Node->VortexLList ) delete Node->VortexLList;
	if ( Node->BodyLList ) delete Node->BodyLList;
	if ( Node->HeatLList ) 
	{
		delete Node->HeatLList; 
	}
	if ( Node->NearNodes ) delete Node->NearNodes;
	if ( Node->FarNodes ) delete Node->FarNodes;
	delete Node;

}}

int DestroyTree()
{
	int i, j, LineLen = 1;
	TNode*** LineLink = Tree_Elements;
	TNode** NodeLink;
	for ( i=0; i<Tree_TotalDepth; i++ )
	{
		NodeLink = *LineLink;
		for ( j=0; j<LineLen; j++ )
		{
			DeleteNode(*NodeLink);
			*NodeLink = NULL;
			NodeLink++;

		}
		LineLink++;
		LineLen= LineLen << 1; // LineLen= pow(2, i); 
	}
	if ( Tree_BottomNodes ) delete Tree_BottomNodes;
}

TNode* FindNode(double px, double py)
{
	return NULL;
}

namespace {
void Stretch(TNode* Node)
{
	double NodeL, NodeR, NodeT, NodeB; // left right top & bottom of the node
	if ( Node->VortexLList )
	{
		NodeL = NodeR = (*(TVortex**)Node->VortexLList->Elements)->rx;
		NodeB = NodeT = (*(TVortex**)Node->VortexLList->Elements)->ry;
	} else
	if ( Node->BodyLList )
	{
		NodeL = NodeR = (*(TVortex**)Node->BodyLList->Elements)->rx;
		NodeB = NodeT = (*(TVortex**)Node->BodyLList->Elements)->ry;
	} else
	if ( Node->HeatLList )
	{
		NodeL = NodeR = (*(TVortex**)Node->HeatLList->Elements)->rx;
		NodeB = NodeT = (*(TVortex**)Node->HeatLList->Elements)->ry;
	}

	#define ResizeNode(List) 											\
		if ( (List) ) 													\
		{ 																\
			int i, lsize = (List)->size; 								\
			TVortex** Obj = (TVortex**)(List)->Elements; 				\
			for ( i=0; i<lsize; i++ ) 									\
			{ 															\
				if ( (*Obj)->rx < NodeL ) NodeL = (*Obj)->rx; 			\
				if ( (*Obj)->rx > NodeR ) NodeR = (*Obj)->rx; 			\
				if ( (*Obj)->ry < NodeB ) NodeB = (*Obj)->ry; 			\
				if ( (*Obj)->ry > NodeT ) NodeT = (*Obj)->ry; 			\
				Obj++; 													\
			} 															\
		} 																

	ResizeNode(Node->VortexLList);
	ResizeNode(Node->BodyLList);
	ResizeNode(Node->HeatLList);

	Node->x = (NodeL + NodeR) * 0.5;
	Node->y = (NodeB + NodeT) * 0.5;
	Node->h = NodeT - NodeB;
	Node->w = NodeR - NodeL;
}}

namespace {
TNode* CreateNode(int VortexListSize, int BodyListSize, int HeatListSize)
{
	TNode *NewNode = new TNode;
	if (VortexListSize) NewNode->VortexLList = new TlList(VortexListSize); else NewNode->VortexLList = NULL;
	if (BodyListSize) NewNode->BodyLList = new TlList(BodyListSize); else NewNode->BodyLList = NULL;
	if (HeatListSize) 
	{
		NewNode->HeatLList = new TlList(HeatListSize);
	} else 
	{
		NewNode->HeatLList = NULL;
	}
	NewNode->NearNodes = NewNode->FarNodes = NULL;
	return NewNode;
}}

namespace {
void DistributeObjects(TlList* ParentList, double NodeC, TlList* &ch1, TlList* &ch2, char side)
{
	int i;
	TVortex** lObj;
	TVortex* obj;
	if (ParentList)
	{
		int lsize = ParentList->size;
		lObj = (TVortex**)ParentList->Elements;
		if ( side == 'x' )
			for ( i=0; i<lsize; i++ )
			{
				obj = *lObj;
				if ( obj->rx < NodeC ) 
					ch1->Add(obj);
				else
					ch2->Add(obj);
				lObj++;
			}
		else if ( side == 'y' )
			for ( i=0; i<lsize; i++ )
			{
				obj = *lObj;
				if ( obj->ry < NodeC ) 
					ch1->Add(obj);
				else
					ch2->Add(obj);
				lObj++;
			}
		if ( !ch1->size ) { delete ch1; ch1=NULL; }
		if ( !ch2->size ) { delete ch2; ch2=NULL; }
	}
}}

namespace {
void CalculateCMass(int i, int j)
{
	double sumg, sumg1;
	TNode* &Node = Tree_Elements[i][j];
	if ( i < Tree_CurrentDepth )
	{
		if ( !Node ) return;
		CalculateCMass(i+1, (j << 1));
		CalculateCMass(i+1, (j << 1) | 1);

		TNode* &Ch1 = Tree_Elements[i+1][(j << 1)];
		TNode* &Ch2 = Tree_Elements[i+1][(j << 1) | 1];

		if ( !Ch1 ) { CalculateCMass(Node); return; }

		#define CalculateCMassFromChilds(CMSign) 											\
			sumg = Ch1->CMSign.g + Ch2->CMSign.g; 											\
			if ( sumg ) 																	\
			{ 																				\
				sumg1 = 1/sumg; 															\
				Node->CMSign.rx = (Ch1->CMSign.rx * Ch1->CMSign.g + Ch2->CMSign.rx * Ch2->CMSign.g) * sumg1; \
				Node->CMSign.ry = (Ch1->CMSign.ry * Ch1->CMSign.g + Ch2->CMSign.ry * Ch2->CMSign.g) * sumg1; \
				Node->CMSign.g = sumg; 														\
			} else { ZeroVortex(Node->CMSign) }													\

		CalculateCMassFromChilds(CMp);
		CalculateCMassFromChilds(CMm);

		return;
	}
	CalculateCMass(Node);
}}

namespace {
void CalculateCMass(TNode* Node)
{
	if ( !Node ) return;
	ZeroVortex(Node->CMp);
	ZeroVortex(Node->CMm);
	if ( !Node->VortexLList ) return;
	int i, lsize = Node->VortexLList->size;
	TVortex** lObj = (TVortex**)Node->VortexLList->Elements;
	TVortex* obj;

	for ( i=0; i<lsize; i++ )
	{
		obj = *lObj;
		if ( obj->g > 0 )
		{
			Node->CMp.rx+= obj->rx * obj->g;
			Node->CMp.ry+= obj->ry * obj->g;
			Node->CMp.g+= obj->g;
		}
		else
		{
			Node->CMm.rx+= obj->rx * obj->g;
			Node->CMm.ry+= obj->ry * obj->g;
			Node->CMm.g+= obj->g;
		}

		lObj++;
	}
	if ( Node->CMp.g )
	{
		double CMp1g = 1/Node->CMp.g;
		Node->CMp.rx *= CMp1g;
		Node->CMp.ry *= CMp1g;
	}
	if ( Node->CMm.g )
	{
		double CMm1g = 1/Node->CMm.g;
		Node->CMm.rx *= CMm1g;
		Node->CMm.ry *= CMm1g;
	} 
}}

namespace {
void FindNearNodes(TNode* BottomNode, int i, int j)
{
	TNode* &TopNode = Tree_Elements[i][j];
	double dx, dy;
	dx = TopNode->x - BottomNode->x;
	dy = TopNode->y - BottomNode->y;
	double SqDistance = dx*dx+dy*dy;
	double HalfPerim = TopNode->h + TopNode->w + BottomNode->h + BottomNode->w;

	if ( SqDistance > Tree_FarCriteria*HalfPerim*HalfPerim )
	{
		//Far
		BottomNode->FarNodes->Add(TopNode);
		return;
	}
	if ( i < Tree_CurrentDepth ) if ( Tree_Elements[i+1][j << 1] )
	{
		FindNearNodes(BottomNode, i+1, (j << 1));
		FindNearNodes(BottomNode, i+1, (j << 1) | 1);
		return;
	}
	BottomNode->NearNodes->Add(TopNode);
}}

double GetAverageNearNodesPercent()
{
	int sum = 0;

	TNode** BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( int i=0; i<lsize; i++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
	}
	sum-= lsize;
	return (sum/((lsize-1.)*lsize));
}

double GetAverageNearNodesCount()
{
	int sum = 0;

	TNode** BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( int i=0; i<lsize; i++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
	}
	sum-= lsize;
	return (sum/lsize);
}

TlList* GetTreeBottomNodes()
{
	return Tree_BottomNodes;
}
