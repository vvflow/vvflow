#include "core.h"
#include "tree.h"
#include "math.h"
#include "iostream"
using namespace std;

const int Tree_MaxListSize = 16;
static vector<TObj> *vList;
static vector<TObj> *hList;
static vector<TObj> *sList;

/********************* TNode class *****************************/

node::node(tree *sParent)
{
	parent = sParent;
	firstVort = lastVort = NULL;
	//VortexLList = NULL;
	BodyLList = NULL;
	firstHeat = lastStreak = NULL;
	//HeatLList = NULL;
	firstStreak = lastStreak = NULL;
	//StreakLList = NULL;
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
	if ( max(h,w) < parent->maxNodeSize )
	if ( min(h,w) < parent->minNodeSize )
	{
		parent->bottomNodes->push_back(this);
		return;
	}

	long MaxListSize = max( max( max (
	                   lastVort-firstVort,
	                   BodyLList->size_safe()),
	                   lastHeat-firstHeat),
	                   lastStreak-firstStreak);

	if ( max(h,w) < parent->maxNodeSize )
	if ( MaxListSize < Tree_MaxListSize ) //look for define
	{
		parent->bottomNodes->push_back(this);
		return;
	}

	ch1 = new TNode(parent);
	ch2 = new TNode(parent);

	ch1->i = ch2->i = i+1; //DEBUG
	ch1->j = j*2; ch2->j = j*2+1; //DEBUG

	definePointerRanges(vList);
	resortList(vList);

	ch1->firstHeat = firstHeat;
	ch2->lastHeat = lastHeat;
	ch1->firstStreak = firstStreak;
	ch2->lastStreak = lastStreak;
	


	DistributeContent(BodyLList, &ch1->BodyLList, &ch2->BodyLList);
	ResortList(HeatLList, &ch1->HeatLList, &ch2->HeatLList);
	ResortList(StreakLList, &ch1->StreakLList, &ch2->StreakLList);

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

void TNode::definePointerRanges(vector<TObj> *list)
{
	TObj *first, *center, *last;
	if (list == vList) { first = center = vRange.first; last = vRange.last; }
	else if (list == hList) { first = center = hRange.first; last = hRange.last; }
	else if (list == sList) { first = center = sRange.first; last = sRange.last; }

	for (TObj* ptr = first; ptr < last; ptr++)
	{
		if ( (h<w) ? (ptr->rx<x) : (ptr->ry<y) )
		{
			center++;
		}
	}

	if (list == vList) { ch1->vRange.set(first, center); ch2->vRange.set(center, last); }
	else if (list == hList) { ch1->hRange.set(first, center); ch2->hRange.set(center, last); }
	else if (list == sList) { ch1->sRange.set(first, center); ch2->sRange.set(center, last); }

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

void TNode::resortList(vector<TObj> *list)
{
	TObj *first, *center, *last;
	if (list == vList)      { first = vRange.first; center= vRange.last; last = vRange.last; }
	else if (list == hList) { first = hRange.first; center= hRange.last; last = hRange.last; }
	else if (list == sList) { first = sRange.first; center= hRange.last; last = sRange.last; }

	for (TObj *p1 = first, *p2 = last; p1<center && p2>=center; p1++, p2--)
	{
		//fixme как организовать сортировку?
		if (p1
	}
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

	if ( dr.abs2() > parent->farCriteria*sqr(HalfPerim) )
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


/************************** TTree SOURCE **************************************/

tree::tree(Space *sS, int sFarCriteria, double sMinNodeSize, double sMaxNodeSize)
{
	S = sS;
	vList = S->VortexList;
	hList = S->HeatList;
	sList = S->StreakList;
	farCriteria = sFarCriteria;
	minNodeSize = sMinNodeSize;
	maxNodeSize = sMaxNodeSize;
	rootNode = NULL;
	bottomNodes = new vector<TNode*>();
}

void tree::build(bool includeV, bool includeB, bool includeH)
{
	if (rootNode) { fprintf(stderr, "Tree is already built\n"); return; }
	rootNode = new TNode(this);
	rootNode->i = rootNode->j = 0; //DEBUG

	if (includeV) fillRootNode(S->VortexList, &rootNode->VortexLList);
	if (includeB)
	{
		const_for(S->BodyList, llbody)
		{
			fillRootNode((**llbody).List, &rootNode->BodyLList);
		}
	}
	if (includeH) fillRootNode(S->HeatList, &rootNode->HeatLList);
	fillRootNode(S->StreakList, &rootNode->StreakLList);

	bottomNodes->clear();

	rootNode->Stretch();
	rootNode->DivideNode(); //recursive

	rootNode->CalculateCMass();
	const_for (bottomNodes, llbnode)
	{
		TNode &bnode = **llbnode;
		bnode.NearNodes = new vector<TNode*>();
		bnode.FarNodes = new vector<TNode*>();
		bnode.FindNearNodes(rootNode);
	}
}

void tree::destroy()
{
	if ( rootNode ) delete rootNode; rootNode = 0;
	bottomNodes->clear();
}

vector<TNode*>* tree::getBottomNodes()
{
	if (!bottomNodes) {fprintf(stderr, "PANIC! Tree isn't built\n");}
	return bottomNodes;
}

TNode* tree::findNode(TVec p)
{
	TNode *Node = rootNode;

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

void tree::printBottomNodes(FILE* f, bool PrintDepth)
{
	if ( !bottomNodes ) return;
	const_for (bottomNodes, llBN)
	{
		fprintf(f, "%g\t%g\t%g\t%g\t", (**llBN).x, (**llBN).y, (**llBN).w, (**llBN).h);
//		fprintf(f, "%g\t%g\t%g\t", (**llBN).CMp.rx, (**llBN).CMp.ry, (**llBN).CMp.g);
//		fprintf(f, "%g\t%g\t%g"  , (**llBN).CMm.rx, (**llBN).CMm.ry, (**llBN).CMm.g);
		if(PrintDepth) fprintf(f, "\t%d", (**llBN).i);
		fprintf(f, "\n");
	}
}

template <class T>
void tree::fillRootNode(vector<T> *src, node::content **dst)
{
	if (!src->size_safe()) return;
	if (!(*dst))
		*dst = new node::content();

	const_for (src, lobj)
	{
		(**dst).push_back(lobj);
	}
}

