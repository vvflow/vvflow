#include "tree.h"

#include <math.h>
#include <iostream>
using namespace std;

const int Tree_MaxListSize = 16;

/********************* HEADER ****************************/
namespace {

Space *Tree_S;
int Tree_FarCriteria;
double Tree_MinNodeSize;

TNode *Tree_RootNode;
vector<TNode*> *Tree_BottomNodes;
TNode *dead_stack;

void find_near_nodes(TNode *bottom, TNode *top);
void DistributeObjects(vector<TObj*>* parent, TVec center, vector<TObj*>* ch1, vector<TObj*>* ch2, char side);
} //end of namespace

/********************* SOURCE *****************************/

void InitTree(Space *sS, int sFarCriteria, double sMinNodeSize)
{
	Tree_S = sS;
	Tree_FarCriteria = sFarCriteria;
	Tree_MinNodeSize = sMinNodeSize;
	Tree_RootNode = NULL;
	Tree_BottomNodes = new vector<TNode*>();
}

inline
void FillRootNode(vector<TObj> *slist, vector<TObj*> *rootlist)
{
	if ( !rootlist ) return;

	for (auto obj = slist->begin(); obj<slist->end(); obj++ )
	{
		rootlist->push_back(&*obj);
	}
}

void BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat)
{
	Tree_RootNode = new TNode(Tree_S->VortexList && IncludeVortexes && Tree_S->VortexList->size(), 
	                          Tree_S->Body->List && IncludeBody && Tree_S->Body->List->size(), 
	                          Tree_S->HeatList && IncludeHeat && Tree_S->HeatList->size());

	FillRootNode(Tree_S->VortexList, Tree_RootNode->VortexLList);
	FillRootNode(Tree_S->Body->List, Tree_RootNode->BodyLList);
	FillRootNode(Tree_S->HeatList, Tree_RootNode->HeatLList);

	Tree_BottomNodes->clear();

	Tree_RootNode->stretch();
	Tree_RootNode->divide(); //recursive

	Tree_RootNode->calc_cmass();
	for (auto bottom_node = Tree_BottomNodes->begin(); bottom_node<Tree_BottomNodes->end(); bottom_node++ )
	{
		(*bottom_node)->near_nodes = new vector<TNode*>();
		(*bottom_node)->far_nodes = new vector<TNode*>();
		//(*bottom_node)->find_near_nodes(Tree_RootNode); //recursive
		find_near_nodes(*bottom_node, Tree_RootNode);
	}
}

TNode::TNode(bool CreateVortexes, bool CreateBody, bool CreateHeat)
{
	VortexLList = CreateVortexes ? new vector<TObj*>() : NULL;
	BodyLList = CreateBody ? new vector<TObj*>() : NULL;
	HeatLList = CreateHeat ? new vector<TObj*>() : NULL;
	near_nodes = far_nodes = NULL;
	ch1 = ch2 = NULL;
}

TNode::~TNode()
{
	if ( ch1 ) { delete ch1; delete ch2; ch1 = ch2 = NULL; }
	if ( VortexLList ) delete VortexLList; VortexLList = NULL;
	if ( BodyLList ) delete BodyLList; BodyLList = NULL;
	if ( HeatLList ) delete HeatLList; HeatLList = NULL;
	if ( near_nodes ) delete near_nodes; near_nodes = NULL;
	if ( far_nodes ) delete far_nodes; far_nodes = NULL;
}

void TNode::del()
{
	if (ch1)
	{
		ch1->del();
		ch2->del();
	}

	if ( VortexLList ) delete VortexLList; VortexLList = NULL;
	if ( BodyLList ) delete BodyLList; BodyLList = NULL;
	if ( HeatLList ) delete HeatLList; HeatLList = NULL;
	if ( near_nodes ) delete near_nodes; near_nodes = NULL;
	if ( far_nodes ) delete far_nodes; far_nodes = NULL;

	ch1 = dead_stack;
	ch2 = NULL;
	dead_stack = this;
}

void DestroyTree()
{
	if ( Tree_RootNode ) delete Tree_RootNode;//->del();
	Tree_RootNode = NULL;
	Tree_BottomNodes->clear();
}

void TNode::stretch()
{
	TVec bl(0,0), tr(0,0);

	if ( VortexLList )
	{
		bl = tr = TVec(**VortexLList->begin());
	} else
	if ( BodyLList )
	{
		bl = tr = TVec(**BodyLList->begin());
	} else
	if ( HeatLList )
	{
		bl = tr = TVec(**HeatLList->begin());
	}

	#define ResizeNode(List) 							\
		if ( (List) ) 									\
		{ 												\
			for (auto obj = List->begin(); obj<List->end(); obj++ ) \
			{ 											\
				bl.rx = min((**obj).rx, bl.rx); 			\
				bl.ry = min((**obj).ry, bl.ry); 			\
				tr.rx = max((**obj).rx, tr.rx); 			\
				tr.ry = max((**obj).ry, tr.ry); 			\
			} 											\
		}

	ResizeNode(VortexLList);
	ResizeNode(BodyLList);
	ResizeNode(HeatLList);
	#undef ResizeNode

	center = (tr+bl) * 0.5;
	size = tr-bl;

}

void TNode::divide()
{
	if ( (size.ry < Tree_MinNodeSize) || (size.rx < Tree_MinNodeSize) ) 
	{
		Tree_BottomNodes->push_back(this);
		return;
	}

	long ParentVortexListSize = VortexLList ? VortexLList->size() : 0;
	long ParentBodyListSize   = BodyLList ? BodyLList->size() : 0;
	long ParentHeatListSize   = HeatLList ? HeatLList->size() : 0;
	long MaxListSize = max(max(ParentVortexListSize, ParentBodyListSize), ParentHeatListSize);
	if ( MaxListSize < Tree_MaxListSize ) //look for define
	{
		Tree_BottomNodes->push_back(this);
		return;
	}

	ch1 = new TNode(ParentVortexListSize, ParentBodyListSize, ParentHeatListSize);
	ch2 = new TNode(ParentVortexListSize, ParentBodyListSize, ParentHeatListSize);

	if ( size.ry < size.rx )
	{
		DistributeObjects(VortexLList, center, ch1->VortexLList, ch2->VortexLList, 'x');
		DistributeObjects(BodyLList, center, ch1->BodyLList, ch2->BodyLList, 'x');
		DistributeObjects(HeatLList, center, ch1->HeatLList, ch2->HeatLList, 'x');
	}
	else
	{
		DistributeObjects(VortexLList, center, ch1->VortexLList, ch2->VortexLList, 'y');
		DistributeObjects(BodyLList, center, ch1->BodyLList, ch2->BodyLList, 'y');
		DistributeObjects(HeatLList, center, ch1->HeatLList, ch2->HeatLList, 'y');
	}

	ch1->stretch();
	ch2->stretch();
	delete VortexLList; VortexLList = NULL;
	delete BodyLList; BodyLList = NULL;
	delete HeatLList; HeatLList = NULL;

	ch1->divide(); //recursion
	ch2->divide(); //recursion
	return;
}

TNode* FindNode(TVec p)
{
	TNode *node = Tree_RootNode;

	while (node->ch1)
	{
		if (node->size.ry < node->size.rx)
		{
			node = (p.rx < node->center.rx) ? node->ch1 : node->ch2 ;
		}
		else
		{
			node = (p.ry < node->center.ry) ? node->ch1 : node->ch2 ;
		}
	}
	return node;
}

namespace {
void DistributeObjects(vector<TObj*>* parent, TVec center, vector<TObj*>* ch1, vector<TObj*>* ch2, char side)
{
	if (!parent) return;

	if ( side == 'x' )
		for (auto obj = parent->begin() ; obj<parent->end(); obj++ )
		{
			if ( (**obj).rx < center.rx ) 
				ch1->push_back(*obj);
			else
				ch2->push_back(*obj);
		}
	else if ( side == 'y' )
		for (auto obj = parent->begin() ; obj<parent->end(); obj++ )
		{
			if ( (**obj).ry < center.ry ) 
				ch1->push_back(*obj);
			else
				ch2->push_back(*obj);
		}
	if ( !ch1->size() ) { delete ch1; ch1=NULL; }
	if ( !ch2->size() ) { delete ch2; ch2=NULL; }
}}

void TNode::calc_cmass()
{
	if (ch1)
	{
		ch1->calc_cmass();
		ch2->calc_cmass();

		CMp = (ch1->CMp * ch1->CMp.g + ch2->CMp * ch2->CMp.g)/max(ch1->CMp.g+ch2->CMp.g, 1E-20);
		CMm = (ch1->CMm * ch1->CMm.g + ch2->CMm * ch2->CMm.g)/min(ch1->CMm.g+ch2->CMm.g, -1E-20);
		CMp.g = ch1->CMp.g + ch2->CMp.g;
		CMm.g = ch1->CMm.g + ch2->CMm.g;
	} else
	{
		calc_cmass_from_scratch();
	}
}

void TNode::calc_cmass_from_scratch()
{
	CMp.zero();
	CMm.zero();
	if ( !VortexLList ) return;

	auto list = VortexLList;
	for (auto obj = list->begin(); obj<list->end(); obj++ )
	{
		if ( (**obj).g > 0 )
		{
			CMp += (**obj) * (**obj).g;
			CMp.g += (**obj).g;
		}
		else
		{
			CMm += (**obj) * (**obj).g;
			CMm.g += (**obj).g;
		}
	}
	if (CMp.g) CMp/= CMp.g;
	if (CMm.g) CMm/= CMm.g;
}

void TNode::find_near_nodes(TNode *top)
{
	double HalfPerim = top->size.rx + top->size.rx + top->size.ry + top->size.ry;

	if ( abs2(top->center - center) > Tree_FarCriteria*HalfPerim*HalfPerim )
	{
		//Far
		far_nodes->push_back(top);
		return;
	}
	if ( top->ch1 )
	{
		find_near_nodes(top->ch1);
		find_near_nodes(top->ch2);
		return;
	}
	near_nodes->push_back(top);
}

namespace {
void find_near_nodes(TNode *bottom, TNode *top)
{
	TVec dr = top->center - bottom->center;
	double HalfPerim = top->size.rx + top->size.rx + top->size.ry + top->size.ry;

	if ( dr.abs2() > Tree_FarCriteria*HalfPerim*HalfPerim )
	{
		//Far
		bottom->far_nodes->push_back(top);
		return;
	}
	if ( top->ch1 )
	{
		find_near_nodes(bottom, top->ch1);
		find_near_nodes(bottom, top->ch2);
		return;
	}
	bottom->near_nodes->push_back(top);
}}

vector<TNode*>* GetTreeBottomNodes()
{
	return Tree_BottomNodes;
}

/*
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
*/
