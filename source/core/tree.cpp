#include "core.h"
#include "tree.h"
#include "math.h"
#include "iostream"
using namespace std;

const int Tree_MaxListSize = 16;

/********************* HEADER ****************************/
namespace {

Space *Tree_S;
int Tree_FarCriteria;
double Tree_MinNodeSize;
double Tree_MaxNodeSize;
//double Tree_MaxNodeSize;

TNode *Tree_RootNode;
vector<TNode*> *Tree_BottomNodes;
}

/********************* TNode class *****************************/

node::node()
{
	VortexLList = NULL;
	BodyLList = NULL;
	HeatLList = NULL;
	StreakLList = NULL;
	NearNodes = FarNodes = NULL;
	ch1 = ch2 = NULL;
	CMp.zero(); CMm.zero();
}

node::~node()
{
	if ( VortexLList ) delete VortexLList;
	if ( BodyLList ) delete BodyLList;
	if ( HeatLList ) delete HeatLList;
	if ( StreakLList ) delete StreakLList;
	if ( NearNodes ) delete NearNodes;
	if ( FarNodes ) delete FarNodes;
	if ( ch1 ) delete ch1;
	if ( ch2 ) delete ch2;
}

void TNode::DivideNode()
{
	if ( max(h,w) < Tree_MaxNodeSize )
	if ( min(h,w) < Tree_MinNodeSize )
	{
		Tree_BottomNodes->push_back(this);
		return;
	}

	long MaxListSize = max( max( max (
	                   VortexLList->size_safe(),
	                   BodyLList->size_safe()),
	                   HeatLList->size_safe()),
	                   StreakLList->size_safe());

	if ( max(h,w) < Tree_MaxNodeSize )
	if ( MaxListSize < Tree_MaxListSize ) //look for define
	{
		Tree_BottomNodes->push_back(this);
		return;
	}

	ch1 = new TNode();
	ch2 = new TNode();
	ch1->i = ch2->i = i+1; //DEBUG
	ch1->j = j*2; ch2->j = j*2+1; //DEBUG

	DistributeContent(VortexLList, &ch1->VortexLList, &ch2->VortexLList);
	DistributeContent(BodyLList, &ch1->BodyLList, &ch2->BodyLList);
	DistributeContent(HeatLList, &ch1->HeatLList, &ch2->HeatLList);
	DistributeContent(StreakLList, &ch1->StreakLList, &ch2->StreakLList);

	ch1->Stretch();
	ch2->Stretch();
	#define delete_content(content) delete content; content = NULL
	delete_content(VortexLList);
	delete_content(BodyLList);
	delete_content(HeatLList);
	delete_content(StreakLList);
	#undef delete_content

	ch1->DivideNode(); //recursion
	ch2->DivideNode(); //recursion
	return;
}

void TNode::Stretch()
{
	TVec tr(0, 0), bl(0, 0);

	if ( VortexLList )
	{
		tr = bl = **VortexLList->begin();
	} else
	if ( BodyLList )
	{
		tr = bl = **BodyLList->begin();
	} else
	if ( HeatLList )
	{
		tr = bl = **HeatLList->begin();
	} else
	if ( StreakLList )
	{
		tr = bl = **StreakLList->begin();
	}

	#define ResizeNode(List) \
		if ( List ) \
		{ \
			const_for (List, obj) \
			{ \
				TObj &o = **obj; \
				tr.rx = max(tr.rx, o.rx); \
				tr.ry = max(tr.ry, o.ry); \
				bl.rx = min(bl.rx, o.rx); \
				bl.ry = min(bl.ry, o.ry); \
			} \
		}

	ResizeNode(VortexLList);
	ResizeNode(BodyLList);
	ResizeNode(HeatLList);
	ResizeNode(StreakLList);
	#undef ResizeNode

	x = (bl+tr).rx * 0.5;
	y = (bl+tr).ry * 0.5;
	h = (tr-bl).ry;
	w = (tr-bl).rx;
}

void TNode::DistributeContent(content *parent, content **ch1, content **ch2)
{
	if (!parent) return;
	if (!*ch1) *ch1 = new content();
	if (!*ch2) *ch2 = new content();

	const_for (parent, llobj)
	{
		if ( (h<w) ? ((**llobj).rx<x) : ((**llobj).ry<y) ) 
			(**ch1).push_back(*llobj);
		else
			(**ch2).push_back(*llobj);
	}
	if ( !(**ch1).size() ) { delete *ch1; *ch1=NULL; }
	if ( !(**ch2).size() ) { delete *ch2; *ch2=NULL; }
}

void TNode::CalculateCMass()
{
	double sumg;

	if (!ch1)
	{
		CalculateCMassFromScratch();
		return;
	}

	ch1->CalculateCMass();
	ch2->CalculateCMass();

	#define CalculateCMassFromChilds(cm) \
		sumg = ch1->cm.g + ch2->cm.g; \
		if ( sumg ) \
		{ \
			cm = (ch1->cm * ch1->cm.g + ch2->cm * ch2->cm.g) / sumg; \
			cm.g = sumg; \
		} else { cm.zero(); }

	CalculateCMassFromChilds(CMp);
	CalculateCMassFromChilds(CMm);
	#undef CalculateCMassFromChilds
}

void TNode::CalculateCMassFromScratch()
{
	CMp.zero();
	CMm.zero();
	if ( !VortexLList ) return;

	const_for (VortexLList, llObj)
	{
		TObj &Obj = **llObj;
		if ( Obj.g > 0 )
		{
			CMp+= Obj * Obj.g;
			CMp.g+= Obj.g;
		}
		else
		{
			CMm+= Obj * Obj.g;
			CMm.g+= Obj.g;
		}
	}

	if ( CMp.g ) { CMp/= CMp.g; }
	if ( CMm.g ) { CMm/= CMm.g; }
}

void TNode::FindNearNodes(TNode* top)
{
	TVec dr;
	dr.rx = top->x - x;
	dr.ry = top->y - y;
	double HalfPerim = top->h + top->w + h + w;

	if ( dr.abs2() > Tree_FarCriteria*HalfPerim*HalfPerim )
	{
		//Far
		FarNodes->push_back(top);
		return;
	}
	if ( top->ch1 )
	{
		FindNearNodes(top->ch1);
		FindNearNodes(top->ch2);
		return;
	}
	NearNodes->push_back(top);
}


/************************** REST SOURCE ***************************************/

void InitTree(Space *sS, int sFarCriteria, double sMinNodeSize, double sMaxNodeSize)
{
	Tree_S = sS;
	Tree_FarCriteria = sFarCriteria;
	Tree_MinNodeSize = sMinNodeSize;
	Tree_MaxNodeSize = sMaxNodeSize;
	Tree_RootNode = NULL;
	Tree_BottomNodes = new vector<TNode*>();
}

namespace {
void FillRootNode(vector<TObj> *src, node::content **dst)
{
	if (!src->size_safe()) return;
	if (!(*dst))
		*dst = new node::content();

	const_for (src, lobj)
	{
		(**dst).push_back(lobj);
	}
}}

void BuildTree(bool includeV, bool includeB, bool includeH)
{
	Tree_RootNode = new TNode();
	Tree_RootNode->i = Tree_RootNode->j = 0; //DEBUG

	if (includeV) FillRootNode(Tree_S->VortexList, &Tree_RootNode->VortexLList);
	if (includeB)
	{
		const_for(Tree_S->BodyList, llbody)
		{
			FillRootNode((**llbody).List, &Tree_RootNode->BodyLList);
		}
	}
	if (includeH) FillRootNode(Tree_S->HeatList, &Tree_RootNode->HeatLList);
	FillRootNode(Tree_S->StreakList, &Tree_RootNode->StreakLList);

	Tree_BottomNodes->clear();

	Tree_RootNode->Stretch();
	Tree_RootNode->DivideNode(); //recursive

	Tree_RootNode->CalculateCMass();
	const_for (Tree_BottomNodes, llbnode)
	{
		TNode &bnode = **llbnode;
		bnode.NearNodes = new vector<TNode*>();
		bnode.FarNodes = new vector<TNode*>();
		/*if ( bnode.VortexLList && (bnode.VortexLList->size_safe() < 3) &&
		    !bnode.BodyLList && !bnode.HeatLList )
		{
			const_for(bnode.VortexLList, llobj) { (**llobj).g = 0; }
			delete bnode.VortexLList; bnode.VortexLList=NULL;
			Tree_BottomNodes->erase(llbnode);
			llbnode--;
			continue;
		}*/
		bnode.FindNearNodes(Tree_RootNode);
	}
}

vector<TNode*>* GetTreeBottomNodes()
{
	if (!Tree_BottomNodes) {fprintf(stderr, "PANIC! Tree isn't built\n");}
	return Tree_BottomNodes;
}

void DestroyTree()
{
	if ( Tree_RootNode ) delete Tree_RootNode; Tree_RootNode = 0;
	Tree_BottomNodes->clear();
}

TNode* FindNode(TVec p)
{
	TNode *Node = Tree_RootNode;

	while (Node->ch1)
	{
		if (Node->h < Node->w)
		{
			Node = (p.rx < Node->x) ? Node->ch1 : Node->ch2 ;
		}
		else
		{
			Node = (p.ry < Node->y) ? Node->ch1 : Node->ch2 ;
		}
	}
	return Node;
}

/************************** GARBAGE ******************************************/

double GetAverageNearNodesPercent()
{
	//FIXME
	/*long sum = 0;

	long lsize = Tree_BottomNodes->size; 
	TNode** BottomNode = Tree_BottomNodes->First;
	TNode** &Last = Tree_BottomNodes->Last;
	for ( ; BottomNode<Last; BottomNode++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
	}
	sum-= lsize; */
	return 0;//(sum/((lsize-1.)*lsize));
}

double GetAverageNearNodesCount()
{
	//FIXME
	/*double sum = 0;

	TNode** BottomNode = Tree_BottomNodes->First;
	TNode** &Last = Tree_BottomNodes->Last;
	long lsize = Tree_BottomNodes->size;
	for ( ; BottomNode<Last; BottomNode++ )
	{
		sum+= (*BottomNode)->NearNodes->size;
	}
	sum-= lsize;*/
	return 0;//(sum/lsize);
}

int GetMaxDepth()
{
	/*//FIXME
	int max = 0;
	if ( !Tree_BottomNodes ) return 0;
	TNode** BottomNode = Tree_BottomNodes->First;
	long lsize = Tree_BottomNodes->size;
	for ( long i=0; i<lsize; i++ )
	{
		if (max < (*BottomNode)->i) max = (*BottomNode)->i;
		BottomNode++;
	}*/
	return 0;// max;
}

int PrintBottomNodes(FILE* f, bool PrintDepth)
{
	if ( !Tree_BottomNodes ) return -1;
	const_for (Tree_BottomNodes, llBN)
	{
		fprintf(f, "%g\t%g\t%g\t%g\t", (**llBN).x, (**llBN).y, (**llBN).w, (**llBN).h);
//		fprintf(f, "%g\t%g\t%g\t", (**llBN).CMp.rx, (**llBN).CMp.ry, (**llBN).CMp.g);
//		fprintf(f, "%g\t%g\t%g"  , (**llBN).CMm.rx, (**llBN).CMm.ry, (**llBN).CMm.g);
		if(PrintDepth) fprintf(f, "\t%d", (**llBN).i);
		fprintf(f, "\n");
	}
	return 0;
}

namespace {
void PrintNode(std::ostream& os, int level, TNode* Node)
{
	/*if (!Node) return;
	if (Node->i == level)
	{
		os << Node->x << "\t" << Node->y << "\t" << Node->w << "\t" << Node->h << endl;
	} else {
		PrintNode(os, level, Node->Child1);
		PrintNode(os, level, Node->Child2);
	}*/
}}

void PrintLevel(std::ostream& os, int level)
{
	/*PrintNode(os, level, Tree_RootNode);*/
}

