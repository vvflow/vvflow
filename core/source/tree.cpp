#include "tree.h"
#include "core.h"
#include "math.h"
#include "iostream"
using namespace std;

#define M_2PI 6.283185308 		// = 2*PI
#define Tree_MaxListSize 16

/********************* HEADER ****************************/
namespace {

Space *Tree_S;
int Tree_FarCriteria;
double Tree_MinNodeSize;

TNode *Tree_RootNode;
TlList *Tree_BottomNodes;

void DivideNode(TNode* ParentNode);

TNode* Tree_NodeStack;
TNode* CreateNode(bool CreateVortexes, bool CreateBody, bool CreateHeat);
void DeleteNode(TNode* Node);
void DeleteSubTree(TNode* Node);

void Stretch(TNode* Node);
void DistributeObjects(TlList* ParentList, double NodeC, TlList* &ch1, TlList* &ch2, char side);
void CalculateCMass(TNode* Node);
void CalculateCMassFromScratch(TNode* Node);
void FindNearNodes(TNode* BottomNode, TNode* TopNode);

} //end of namespace

/********************* SOURCE *****************************/

int InitTree(Space *sS, int sFarCriteria, double sMinNodeSize)
{
	Tree_S = sS;
	Tree_FarCriteria = sFarCriteria;
	Tree_MinNodeSize = sMinNodeSize;
	Tree_RootNode = NULL;
	Tree_NodeStack = NULL;
}

int BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat)
{
	Tree_RootNode = CreateNode(Tree_S->VortexList && IncludeVortexes && Tree_S->VortexList->size, 
								Tree_S->BodyList && IncludeBody && Tree_S->BodyList->size, 
								Tree_S->HeatList && IncludeHeat && Tree_S->HeatList->size);
	Tree_RootNode->i = Tree_RootNode->j = 0; //DEBUG

	//Filling root node
	#define FillRootNode(SList, RootLList) 						\
		if ( RootLList ) 										\
		{ 														\
			int i, lsize = SList->size; 						\
			TVortex* Obj = (TVortex*)SList->Elements; 			\
			for ( i=0; i<lsize; i++ ) 							\
			{ 													\
				RootLList->Add(Obj); 							\
				Obj++; 											\
			} 													\
		}

	FillRootNode(Tree_S->VortexList, Tree_RootNode->VortexLList)
	FillRootNode(Tree_S->BodyList, Tree_RootNode->BodyLList)
	FillRootNode(Tree_S->HeatList, Tree_RootNode->HeatLList)

	Tree_BottomNodes = new TlList();
	Stretch(Tree_RootNode);
	DivideNode(Tree_RootNode); //recursive

	CalculateCMass(Tree_RootNode);
	TNode** BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( int i=0; i<lsize; i++ )
	{
		(*BottomNode)->NearNodes = new TlList();
		(*BottomNode)->FarNodes = new TlList();
		FindNearNodes(*BottomNode, Tree_RootNode);
		BottomNode++;
	}
}

namespace {
void DivideNode(TNode* ParentNode)
{
	double NodecX, NodecY, NodeH, NodeW;  // center, height & width
	NodecX = ParentNode->x;
	NodecY = ParentNode->y;
	NodeH = ParentNode->h;
	NodeW = ParentNode->w;

	if ( (NodeH < Tree_MinNodeSize) || (NodeW < Tree_MinNodeSize) ) 
	{
		Tree_BottomNodes->Add(ParentNode);
		return;
	}

	int ParentVortexListSize = 0;
	int ParentBodyListSize = 0;
	int ParentHeatListSize = 0;
	int MaxListSize = 0;
	if ( ParentNode->VortexLList ) ParentVortexListSize = ParentNode->VortexLList->size;
	if ( ParentNode->BodyLList ) ParentBodyListSize = ParentNode->BodyLList->size;
	if ( ParentNode->HeatLList ) ParentHeatListSize = ParentNode->HeatLList->size;
	if ( MaxListSize < ParentVortexListSize ) MaxListSize = ParentVortexListSize;
	if ( MaxListSize < ParentBodyListSize ) MaxListSize = ParentBodyListSize;
	if ( MaxListSize < ParentHeatListSize ) MaxListSize = ParentHeatListSize;
	if ( MaxListSize < Tree_MaxListSize ) //look for define
	{
		Tree_BottomNodes->Add(ParentNode);
		return;
	}

	TNode *Ch1, *Ch2;
	ParentNode->Child1 = CreateNode(ParentVortexListSize, ParentBodyListSize, ParentHeatListSize);
	ParentNode->Child2 = CreateNode(ParentVortexListSize, ParentBodyListSize, ParentHeatListSize);
	Ch1=ParentNode->Child1; Ch2=ParentNode->Child2;
	Ch1->i = Ch2->i = ParentNode->i+1; //DEBUG
	Ch1->j = ParentNode->j << 1; Ch2->j = (ParentNode->j << 1) | 1; //DEBUG

	if ( NodeH < NodeW )
	{
		DistributeObjects(ParentNode->VortexLList, NodecX, Ch1->VortexLList, Ch2->VortexLList, 'x');
		DistributeObjects(ParentNode->BodyLList, NodecX, Ch1->BodyLList, Ch2->BodyLList, 'x');
		DistributeObjects(ParentNode->HeatLList, NodecX, Ch1->HeatLList, Ch2->HeatLList, 'x');
	}
	else
	{
		DistributeObjects(ParentNode->VortexLList, NodecY, Ch1->VortexLList, Ch2->VortexLList, 'y');
		DistributeObjects(ParentNode->BodyLList, NodecY, Ch1->BodyLList, Ch2->BodyLList, 'y');
		DistributeObjects(ParentNode->HeatLList, NodecY, Ch1->HeatLList, Ch2->HeatLList, 'y');
	}

	Stretch(Ch1);
	Stretch(Ch2);
	delete ParentNode->VortexLList; ParentNode->VortexLList = NULL;
	delete ParentNode->BodyLList; ParentNode->BodyLList = NULL;
	delete ParentNode->HeatLList; ParentNode->HeatLList = NULL;

	DivideNode(Ch1); //recursion
	DivideNode(Ch2); //recursion
	return;
}}

namespace {
TNode* CreateNode(bool CreateVortexes, bool CreateBody, bool CreateHeat)
{
	TNode *NewNode;
	if (Tree_NodeStack)
	{
		NewNode = Tree_NodeStack;
		Tree_NodeStack = Tree_NodeStack->Child1;
	} else
	{
		NewNode = new TNode;
	}
	
	if (CreateVortexes) NewNode->VortexLList = new TlList(); else NewNode->VortexLList = NULL;
	if (CreateBody) NewNode->BodyLList = new TlList(); else NewNode->BodyLList = NULL;
	if (CreateHeat) NewNode->HeatLList = new TlList(); else NewNode->HeatLList = NULL;
	NewNode->NearNodes = NewNode->FarNodes = NULL;
	NewNode->Child1 = NewNode->Child2 = NULL;
	return NewNode;
}}

namespace {
void DeleteNode(TNode* Node)
{
	if ( Node->VortexLList ) delete Node->VortexLList;
	if ( Node->BodyLList ) delete Node->BodyLList;
	if ( Node->HeatLList ) delete Node->HeatLList;
	if ( Node->NearNodes ) delete Node->NearNodes;
	if ( Node->FarNodes ) delete Node->FarNodes;
	Node->Child1 = Tree_NodeStack;
	Tree_NodeStack = Node;
	return;
}}

namespace {
void DeleteSubTree(TNode* Node)
{
	if (Node->Child1) DeleteSubTree(Node->Child1);
	if (Node->Child2) DeleteSubTree(Node->Child2);
	DeleteNode(Node);
}}

int DestroyTree()
{
	if ( Tree_RootNode ) DeleteSubTree(Tree_RootNode);
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
void DistributeObjects(TlList* ParentList, double NodeC, TlList* &ch1, TlList* &ch2, char side)
{
	int i;
	TVortex** lObj;
	TVortex* obj;
	if (!ParentList) return;

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
}}

namespace {
void CalculateCMass(TNode* Node)
{
	double sumg, sumg1;

	if ( !Node ) return;
	CalculateCMass(Node->Child1);
	CalculateCMass(Node->Child2);

	TNode* Ch1 = Node->Child1;
	TNode* Ch2 = Node->Child2;

	if ( !Ch1 ) { CalculateCMassFromScratch(Node); return; }

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
}}

namespace {
void CalculateCMassFromScratch(TNode* Node)
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
void FindNearNodes(TNode* BottomNode, TNode* TopNode)
{
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
	if ( TopNode->Child1 )
	{
		FindNearNodes(BottomNode, TopNode->Child1);
		FindNearNodes(BottomNode, TopNode->Child2);
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
		BottomNode++;
	}
	sum-= lsize;
	return (sum/((lsize-1.)*lsize));
}

double GetAverageNearNodesCount()
{
	double sum = 0;

	TNode** BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( int i=0; i<lsize; i++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
		BottomNode++;
	}
	sum-= lsize;
	return (sum/lsize);
}

TlList* GetTreeBottomNodes()
{
	return Tree_BottomNodes;
}

int GetMaxDepth()
{
	int max = 0;
	if ( !Tree_BottomNodes ) return 0;
	TNode** BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( int i=0; i<lsize; i++ )
	{
		if (max < (*BottomNode)->i) max = (*BottomNode)->i;
		BottomNode++;
	}
	return max;
}

int PrintBottomNodes(std::ostream& os, bool PrintDepth)
{
	if ( !Tree_BottomNodes ) return -1;
	TNode** BottomNode = (TNode**)Tree_BottomNodes->Elements;
	int lsize = Tree_BottomNodes->size;
	for ( int i=0; i<lsize; i++ )
	{
		os << (*BottomNode)->x << "\t" << (*BottomNode)->y << "\t" << (*BottomNode)->w << "\t" << (*BottomNode)->h;
		if (PrintDepth) { os << "\t" << (*BottomNode)->i; }
		os << endl;
		BottomNode++;
	}
	return 0;
}

namespace {
int PrintNode(std::ostream& os, int level, TNode* Node)
{
	if (!Node) return 0;
	if (Node->i == level)
	{
		os << Node->x << "\t" << Node->y << "\t" << Node->w << "\t" << Node->h << endl;
	} else {
		PrintNode(os, level, Node->Child1);
		PrintNode(os, level, Node->Child2);
	}
	return 0;
}}

int PrintLevel(std::ostream& os, int level)
{
	PrintNode(os, level, Tree_RootNode);
}
