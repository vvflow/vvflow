#include "tree.h"
#include "core.h"
#include "math.h"
#include "iostream"
using namespace std;

#define Tree_MaxListSize 16

/********************* HEADER ****************************/
namespace {

Space *Tree_S;
int Tree_FarCriteria;
double Tree_MinNodeSize;

TNode *Tree_RootNode;
TList<TNode*> *Tree_BottomNodes;

void DivideNode(TNode* ParentNode);

TNode* Tree_NodeStack;
TNode* CreateNode(bool CreateVortexes, bool CreateBody, bool CreateHeat);
void DeleteNode(TNode* Node);
void DeleteSubTree(TNode* Node);

void Stretch(TNode* Node);
void DistributeObjects(TList<TObject*>* ParentList, double NodeC, TList<TObject*>* &ch1, TList<TObject*>* &ch2, char side);
void CalculateCMass(TNode* Node);
void CalculateCMassFromScratch(TNode* Node);
void FindNearNodes(TNode* BottomNode, TNode* TopNode);

} //end of namespace

/********************* SOURCE *****************************/

void InitTree(Space *sS, int sFarCriteria, double sMinNodeSize)
{
	Tree_S = sS;
	Tree_FarCriteria = sFarCriteria;
	Tree_MinNodeSize = sMinNodeSize;
	Tree_RootNode = NULL;
	Tree_NodeStack = NULL;
	Tree_BottomNodes = new TList<TNode*>();
}

inline
void FillRootNode(TList<TObject> *SList, TList<TObject*> *RootLList)
{
	if ( !RootLList ) return;

	TObject* Obj = SList->First;
	TObject* &Last = SList->Last;
	for ( ; Obj<Last; Obj++ )
	{
		RootLList->Add(Obj);
	}
}

void BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat)
{
	Tree_RootNode = CreateNode(Tree_S->VortexList && IncludeVortexes && Tree_S->VortexList->size, 
								Tree_S->BodyList && IncludeBody && Tree_S->BodyList->size, 
								Tree_S->HeatList && IncludeHeat && Tree_S->HeatList->size);
	Tree_RootNode->i = Tree_RootNode->j = 0; //DEBUG

	FillRootNode(Tree_S->VortexList, Tree_RootNode->VortexLList);
	FillRootNode(Tree_S->BodyList, Tree_RootNode->BodyLList);
	FillRootNode(Tree_S->HeatList, Tree_RootNode->HeatLList);

	Tree_BottomNodes->Clear();

	Stretch(Tree_RootNode);
	DivideNode(Tree_RootNode); //recursive

	CalculateCMass(Tree_RootNode);
	TNode** BottomNode = Tree_BottomNodes->First;
	TNode** &LastBN = Tree_BottomNodes->Last;
	for ( ; BottomNode<LastBN; BottomNode++ )
	{
		(*BottomNode)->NearNodes = new TList<TNode*>();
		(*BottomNode)->FarNodes = new TList<TNode*>();
		FindNearNodes(*BottomNode, Tree_RootNode);
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

	long ParentVortexListSize = 0;
	long ParentBodyListSize = 0;
	long ParentHeatListSize = 0;
	long MaxListSize = 0;
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
	
	if (CreateVortexes) NewNode->VortexLList = new TList<TObject*>(); else NewNode->VortexLList = NULL;
	if (CreateBody) NewNode->BodyLList = new TList<TObject*>(); else NewNode->BodyLList = NULL;
	if (CreateHeat) NewNode->HeatLList = new TList<TObject*>(); else NewNode->HeatLList = NULL;
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

void DestroyTree()
{
	if ( Tree_RootNode ) DeleteSubTree(Tree_RootNode);
	Tree_BottomNodes->Clear();
}

TNode* FindNode(double px, double py)
{
	return NULL;
}

namespace {
void Stretch(TNode* Node)
{
	double NodeL=0, NodeR=0, NodeT=0, NodeB=0; // left right top & bottom of the node
	if ( Node->VortexLList )
	{
		NodeL = NodeR = (*Node->VortexLList->First)->rx;
		NodeB = NodeT = (*Node->VortexLList->First)->ry;
	} else
	if ( Node->BodyLList )
	{
		NodeL = NodeR = (*Node->BodyLList->First)->rx;
		NodeB = NodeT = (*Node->BodyLList->First)->ry;
	} else
	if ( Node->HeatLList )
	{
		NodeL = NodeR = (*Node->HeatLList->First)->rx;
		NodeB = NodeT = (*Node->HeatLList->First)->ry;
	}

	#define ResizeNode(List) 							\
		if ( (List) ) 									\
		{ 												\
			TObject** Obj = (List)->First; 				\
			TObject** &Last = (List)->Last; 			\
			for ( ; Obj<Last; Obj++ ) 					\
			{ 											\
				TObject &o = **Obj; 					\
				if ( o.rx < NodeL ) NodeL = o.rx; 		\
				if ( o.rx > NodeR ) NodeR = o.rx; 		\
				if ( o.ry < NodeB ) NodeB = o.ry; 		\
				if ( o.ry > NodeT ) NodeT = o.ry; 		\
			} 											\
		}

	ResizeNode(Node->VortexLList);
	ResizeNode(Node->BodyLList);
	ResizeNode(Node->HeatLList);
	#undef ResizeNode

	Node->x = (NodeL + NodeR) * 0.5;
	Node->y = (NodeB + NodeT) * 0.5;
	Node->h = NodeT - NodeB;
	Node->w = NodeR - NodeL;
}}

namespace {
void DistributeObjects(TList<TObject*>* ParentList, double NodeC, TList<TObject*>* &ch1, TList<TObject*>* &ch2, char side)
{
	if (!ParentList) return;

	TObject** lObj = ParentList->First;
	TObject** &Last = ParentList->Last;
	if ( side == 'x' )
		for ( ; lObj<Last; lObj++ )
		{
			TObject *&Obj = *lObj;
			if ( Obj->rx < NodeC ) 
				ch1->Add(Obj);
			else
				ch2->Add(Obj);
		}
	else if ( side == 'y' )
		for ( ; lObj<Last; lObj++ )
		{
			TObject *&Obj = *lObj;
			if ( Obj->ry < NodeC ) 
				ch1->Add(Obj);
			else
				ch2->Add(Obj);
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
		} else { ZeroObject(Node->CMSign) }													\

	CalculateCMassFromChilds(CMp);
	CalculateCMassFromChilds(CMm);
	#undef CalculateCMassFromChilds
}}

namespace {
void CalculateCMassFromScratch(TNode* Node)
{
	if ( !Node ) return;
	ZeroObject(Node->CMp);
	ZeroObject(Node->CMm);
	if ( !Node->VortexLList ) return;

	TObject** lObj = Node->VortexLList->First;
	TObject** Last = Node->VortexLList->Last;
	for ( ; lObj<Last; lObj++ )
	{
		TObject *&Obj = *lObj;
		if ( Obj->g > 0 )
		{
			Node->CMp.rx+= Obj->rx * Obj->g;
			Node->CMp.ry+= Obj->ry * Obj->g;
			Node->CMp.g+= Obj->g;
		}
		else
		{
			Node->CMm.rx+= Obj->rx * Obj->g;
			Node->CMm.ry+= Obj->ry * Obj->g;
			Node->CMm.g+= Obj->g;
		}
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
	//FIXME
	long sum = 0;

	long lsize = Tree_BottomNodes->size; 
	TNode** BottomNode = Tree_BottomNodes->First;
	TNode** &Last = Tree_BottomNodes->Last;
	for ( ; BottomNode<Last; BottomNode++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
	}
	sum-= lsize;
	return (sum/((lsize-1.)*lsize));
}

double GetAverageNearNodesCount()
{
	//FIXME
	double sum = 0;

	TNode** BottomNode = Tree_BottomNodes->First;
	TNode** &Last = Tree_BottomNodes->Last;
	long lsize = Tree_BottomNodes->size;
	for ( ; BottomNode<Last; BottomNode++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
	}
	sum-= lsize;
	return (sum/lsize);
}

TList<TNode*>* GetTreeBottomNodes()
{
	return Tree_BottomNodes;
}

int GetMaxDepth()
{
	//FIXME
	int max = 0;
	if ( !Tree_BottomNodes ) return 0;
	TNode** BottomNode = Tree_BottomNodes->First;
	long lsize = Tree_BottomNodes->size;
	for ( long i=0; i<lsize; i++ )
	{
		if (max < (*BottomNode)->i) max = (*BottomNode)->i;
		BottomNode++;
	}
	return max;
}

int PrintBottomNodes(std::ostream& os, bool PrintDepth)
{
	//FIXME
	if ( !Tree_BottomNodes ) return -1;
	TNode** BottomNode = Tree_BottomNodes->First;
	long lsize = Tree_BottomNodes->size;
	for ( long i=0; i<lsize; i++ )
	{
		os << (*BottomNode)->x << "\t" << (*BottomNode)->y << "\t" << (*BottomNode)->w << "\t" << (*BottomNode)->h;
		if (PrintDepth) { os << "\t" << (*BottomNode)->i; }
		os << endl;
		BottomNode++;
	}
	return 0;
}

namespace {
void PrintNode(std::ostream& os, int level, TNode* Node)
{
	if (!Node) return;
	if (Node->i == level)
	{
		os << Node->x << "\t" << Node->y << "\t" << Node->w << "\t" << Node->h << endl;
	} else {
		PrintNode(os, level, Node->Child1);
		PrintNode(os, level, Node->Child2);
	}
}}

void PrintLevel(std::ostream& os, int level)
{
	PrintNode(os, level, Tree_RootNode);
}

