#include "core.h"
#include "sorted_tree.h"
#include "math.h"
#include "iostream"
using namespace std;

const int Tree_MaxListSize = 16;
static vector<TObj> *vList;
static vector<TObj> *hList;
static vector<TObj> *sList;

/********************* TSortedNode class *****************************/

snode::snode(stree *sParent)
{
	parent = sParent;

	BodyLList = NULL;
	vRange.set(NULL, NULL);
	hRange.set(NULL, NULL);
	sRange.set(NULL, NULL);

	NearNodes = FarNodes = NULL;
	ch1 = ch2 = NULL;
	CMp.zero(); CMm.zero();
}

snode::~snode()
{
	if ( BodyLList ) delete BodyLList;
	if ( NearNodes ) delete NearNodes;
	if ( FarNodes ) delete FarNodes;
	if ( ch1 ) delete ch1;
	if ( ch2 ) delete ch2;
}

void TSortedNode::DivideNode()
{
	if ( max(h,w) < parent->maxNodeSize )
	if ( min(h,w) < parent->minNodeSize )
	{
		parent->bottomNodes->push_back(this);
		return;
	}

	long MaxListSize = max(
	                   BodyLList->size_safe(),
	                   vRange.size(),
	                   hRange.size(),
	                   sRange.size());

	if ( max(h,w) < parent->maxNodeSize )
	if ( MaxListSize < Tree_MaxListSize ) //look for define
	{
		parent->bottomNodes->push_back(this);
		return;
	}

	//fprintf(stderr, "%d %d will split: h=%g, w=%g, x=%g, y=%g, size=%d\n", i, j, h, w, x, y, MaxListSize);

	ch1 = new TSortedNode(parent);
	ch2 = new TSortedNode(parent);

	ch1->i = ch2->i = i+1; //DEBUG
	ch1->j = j*2; ch2->j = j*2+1; //DEBUG

	definePointerRangesAndSort(vList);
	definePointerRangesAndSort(hList);
	definePointerRangesAndSort(sList);

	DistributeContent(BodyLList, &ch1->BodyLList, &ch2->BodyLList);
	delete BodyLList; BodyLList = NULL;

	ch1->Stretch();
	ch2->Stretch();

	ch1->DivideNode(); //recursion
	ch2->DivideNode(); //recursion
	return;
}

void TSortedNode::definePointerRangesAndSort(vector<TObj> *list)
{
	TObj *first, *last;
	if (list == vList) { first = vRange.first; last = vRange.last; }
	else if (list == hList) { first = hRange.first; last = hRange.last; }
	else if (list == sList) { first = sRange.first; last = sRange.last; }

	//if (list == vList) fprintf(stderr, "%d %d; first = %x; last = %x\n", i, j, first, last);
	TObj *p1 = first, *p2 = last-1;
	while (p1 <= p2)
	{
		while ( (p1<last) && ((h<w) ? (p1->rx<x) : (p1->ry<y)) ) p1++;
		while ( (p2>=first) && ((h<w) ? (p2->rx>=x) : (p2->ry>=y)) ) p2--;
		if (p1 < p2)
		{ //swap objects
			//fprintf(stderr, "swap %x with %x : %c %g (%g, %g) (%g %g)\n", p1, p2, (h>w)?'h':'w', (h>w)?y:x, p1->rx, p1->ry, p2->rx, p2->ry);
			TObj tmp = *p1;
			*p1 = *p2;
			*p2 = tmp;
		}
	}

	if (list == vList) { ch1->vRange.set(first, p1); ch2->vRange.set(p1, last); }
	else if (list == hList) { ch1->hRange.set(first, p1); ch2->hRange.set(p1, last); }
	else if (list == sList) { ch1->sRange.set(first, p1); ch2->sRange.set(p1, last); }

	return;
}

void TSortedNode::Stretch()
{
	TVec tr(-DBL_MAX, -DBL_MAX), bl(DBL_MAX, DBL_MAX);

	if (BodyLList)
	const_for(BodyLList, llobj)
	{
		tr.rx = max(tr.rx, (**llobj).rx);
		tr.ry = max(tr.ry, (**llobj).ry);
		bl.rx = min(bl.rx, (**llobj).rx);
		bl.ry = min(bl.ry, (**llobj).ry);
	}

	//fprintf(stderr, "%d %d stretch body: l=%g, r=%g, b=%g, t=%g\n", i, j, bl.rx, tr.rx, bl.ry, tr.ry);

	Stretch(vRange, tr, bl);
	Stretch(hRange, tr, bl);
	Stretch(sRange, tr, bl);

	//fprintf(stderr, "%d %d stretch objs: l=%g, r=%g, b=%g, t=%g\n", i, j, bl.rx, tr.rx, bl.ry, tr.ry);

	x = (bl+tr).rx * 0.5;
	y = (bl+tr).ry * 0.5;
	h = (tr-bl).ry;
	w = (tr-bl).rx;
}

void TSortedNode::Stretch(range &oRange, TVec &tr, TVec &bl)
{
	for (TObj *lobj = oRange.first; lobj < oRange.last; lobj++)
	{
		tr.rx = max(tr.rx, lobj->rx);
		tr.ry = max(tr.ry, lobj->ry);
		bl.rx = min(bl.rx, lobj->rx);
		bl.ry = min(bl.ry, lobj->ry);
	}
}

void TSortedNode::DistributeContent(blList *parent, blList **ch1, blList **ch2)
{
	if (!parent) return;
	if (!*ch1) *ch1 = new blList();
	if (!*ch2) *ch2 = new blList();

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

void TSortedNode::CalculateCMass()
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

void TSortedNode::CalculateCMassFromScratch()
{
	CMp.zero();
	CMm.zero();

	for (TObj *lobj = vRange.first; lobj < vRange.last; lobj++)
	{
		if ( lobj->g > 0 )
		{
			CMp+= TVec(*lobj) * lobj->g;
			CMp.g+= lobj->g;
		}
		else
		{
			CMm+= TVec(*lobj) * lobj->g;
			CMm.g+= lobj->g;
		}
	}

	if ( CMp.g ) { CMp/= CMp.g; }
	if ( CMm.g ) { CMm/= CMm.g; }
}

void TSortedNode::FindNearNodes(TSortedNode* top)
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

stree::stree(Space *sS, int sFarCriteria, double sMinNodeSize, double sMaxNodeSize)
{
	S = sS;
	vList = S->VortexList;
	hList = S->HeatList;
	sList = S->StreakList;
	farCriteria = sFarCriteria;
	minNodeSize = sMinNodeSize;
	maxNodeSize = sMaxNodeSize;
	rootNode = NULL;
	bottomNodes = new vector<TSortedNode*>();
}

void stree::build(bool includeV, bool includeB, bool includeH)
{
	if (rootNode) { fprintf(stderr, "Tree is already built\n"); return; }
	rootNode = new TSortedNode(this);
	rootNode->i = rootNode->j = 0; //DEBUG

	rootNode->BodyLList = new snode::blList();
	if (includeB)
	{
		const_for(S->BodyList, llbody)
		{
			const_for ((**llbody).List, lobj)
			{
				rootNode->BodyLList->push_back(lobj);
			}
		}
	}

	if (includeV) rootNode->vRange.set(vList->begin(), vList->end()); else rootNode->vRange.set(NULL, NULL);
	if (includeH) rootNode->hRange.set(hList->begin(), hList->end()); else rootNode->hRange.set(NULL, NULL);
	rootNode->sRange.set(sList->begin(), sList->end());

	bottomNodes->clear();

	rootNode->Stretch();
	rootNode->DivideNode(); //recursive

	rootNode->CalculateCMass();
	const_for (bottomNodes, llbnode)
	{
		TSortedNode &bnode = **llbnode;
		bnode.NearNodes = new vector<TSortedNode*>();
		bnode.FarNodes = new vector<TSortedNode*>();
		bnode.FindNearNodes(rootNode);
	}
}

void stree::destroy()
{
	if ( rootNode ) delete rootNode; rootNode = 0;
	bottomNodes->clear();
}

vector<TSortedNode*>* stree::getBottomNodes()
{
	if (!bottomNodes) {fprintf(stderr, "PANIC in stree::getBottomNodes()! Tree isn't built\n");}
	return bottomNodes;
}

TSortedNode* stree::findNode(TVec p)
{
	TSortedNode *Node = rootNode;

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

void stree::printBottomNodes(FILE* f, bool PrintDepth)
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

