#include "core.h"
#include "sorted_tree.h"
#include "body.h"
#include "math.h"
#include "iostream"
using namespace std;

const int Tree_MaxListSize = 16;

/********************* TSortedNode class *****************************/

snode::snode(stree *sParent)
{
	parent = sParent;

	NearNodes = FarNodes = NULL;
	ch1 = ch2 = NULL;
	CMp = CMm = TObj(0., 0., 0.);
}

snode::~snode()
{
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
		parent->bottomNodes.push_back(this);
		return;
	}

	long MaxListSize = max(
	                   bllist.size(),
	                   vRange.size(),
	                   hRange.size(),
	                   sRange.size());

	if ( max(h,w) < parent->maxNodeSize )
	if ( MaxListSize < Tree_MaxListSize ) //look for define
	{
		parent->bottomNodes.push_back(this);
		return;
	}

	//fprintf(stderr, "%d %d will split: h=%g, w=%g, x=%g, y=%g, size=%d\n", i, j, h, w, x, y, MaxListSize);

	// FIXME нужен свой менеджер памяти, что бы не выделять ее каждый раз заново
	ch1 = new TSortedNode(parent);
	ch2 = new TSortedNode(parent);

	ch1->i = ch2->i = i+1; //DEBUG
	ch1->j = j*2; ch2->j = j*2+1; //DEBUG

	DistributeContent(vRange, &ch1->vRange, &ch2->vRange);
	DistributeContent(hRange, &ch1->hRange, &ch2->hRange);
	DistributeContent(sRange, &ch1->sRange, &ch2->sRange);
	DistributeContent(bllist, &ch1->bllist, &ch2->bllist);
	bllist.clear();

	ch1->Stretch();
	ch2->Stretch();

	ch1->DivideNode(); //recursion
	ch2->DivideNode(); //recursion
	return;
}

void TSortedNode::DistributeContent(range& parent, range *ch1, range *ch2)
{
	TObj *p1 = parent.first;
	TObj *p2 = parent.last-1;
	while (p1 <= p2)
	{
		while ( ((h<w) ? (p1->r.x<x)  : (p1->r.y<y))  && p1<parent.last )   p1++;
		while ( ((h<w) ? (p2->r.x>=x) : (p2->r.y>=y)) && p2>=parent.first ) p2--;
		if (p1 < p2) std::swap(*p1, *p2);
	}

	ch1->set(parent.first, p1);
	ch2->set(p1, parent.last);

	return;
}

void TSortedNode::Stretch()
{
	TVec tr(-DBL_MAX, -DBL_MAX), bl(DBL_MAX, DBL_MAX);

	for (auto& llobj: bllist)
	{
		tr.x = max(tr.x, llobj->r.x);
		tr.y = max(tr.y, llobj->r.y);
		bl.x = min(bl.x, llobj->r.x);
		bl.y = min(bl.y, llobj->r.y);
	}

	//fprintf(stderr, "%d %d stretch body: l=%g, r=%g, b=%g, t=%g\n", i, j, bl.rx, tr.rx, bl.ry, tr.ry);

	Stretch(vRange, tr, bl);
	Stretch(hRange, tr, bl);
	Stretch(sRange, tr, bl);

	//fprintf(stderr, "%d %d stretch objs: l=%g, r=%g, b=%g, t=%g\n", i, j, bl.rx, tr.rx, bl.ry, tr.ry);

	x = (bl+tr).x * 0.5;
	y = (bl+tr).y * 0.5;
	h = (tr-bl).y;
	w = (tr-bl).x;
}

void TSortedNode::Stretch(range &oRange, TVec &tr, TVec &bl)
{
	for (TObj *lobj = oRange.first; lobj < oRange.last; lobj++)
	{
		tr.x = max(tr.x, lobj->r.x);
		tr.y = max(tr.y, lobj->r.y);
		bl.x = min(bl.x, lobj->r.x);
		bl.y = min(bl.y, lobj->r.y);
	}
}

void TSortedNode::DistributeContent(LList &parent, LList *ch1, LList *ch2)
{
	for (auto llobj: parent)
	{
		if ( (h<w) ? (llobj->r.x<x) : (llobj->r.y<y) ) 
			ch1->push_back(llobj);
		else
			ch2->push_back(llobj);
	}
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
			cm.r = (ch1->cm.r * ch1->cm.g + ch2->cm.r * ch2->cm.g) / sumg; \
			cm.g = sumg; \
		} else { cm = TObj(x, y, 0); }

	CalculateCMassFromChilds(CMp);
	CalculateCMassFromChilds(CMm);
	#undef CalculateCMassFromChilds
}

void TSortedNode::CalculateCMassFromScratch()
{
	CMp = CMm = TObj();

	for (TObj *lobj = vRange.first; lobj < vRange.last; lobj++)
	{
		if ( lobj->g > 0 )
		{
			CMp.r+= lobj->r * lobj->g;
			CMp.g+= lobj->g;
		}
		else
		{
			CMm.r+= lobj->r * lobj->g;
			CMm.g+= lobj->g;
		}
	}

	if ( CMp.g ) { CMp.r/= CMp.g; } else {CMp.r = TVec(x, y);}
	if ( CMm.g ) { CMm.r/= CMm.g; } else {CMm.r = TVec(x, y);}
}

void TSortedNode::FindNearNodes(TSortedNode* top)
{
	TVec dr = TVec(top->x, top->y) - TVec(x, y);
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
	farCriteria = sFarCriteria;
	minNodeSize = sMinNodeSize;
	maxNodeSize = sMaxNodeSize;
	rootNode = NULL;
}

void stree::build(bool includeV, bool includeB, bool includeH)
{
	if (rootNode) { fprintf(stderr, "Tree is already built\n"); return; }
	rootNode = new TSortedNode(this);
	rootNode->i = rootNode->j = 0; //DEBUG

	if (includeB)
	{
		for (auto& lbody: S->BodyList)
		{
			for (auto& lobj: lbody->alist)
			{
				rootNode->bllist.push_back(&lobj);
			}
		}
	}

	if (includeV && S->VortexList.size()) rootNode->vRange.set(&*S->VortexList.begin(), &*S->VortexList.end());
	if (includeH && S->HeatList.size())   rootNode->hRange.set(&*S->HeatList.begin(), &*S->HeatList.end());
	if (S->StreakList.size())             rootNode->sRange.set(&*S->StreakList.begin(), &*S->StreakList.end());

	bottomNodes.clear();

	rootNode->Stretch();
	rootNode->DivideNode(); //recursive

	rootNode->CalculateCMass();
	for (auto& lbnode: bottomNodes)
	{
		lbnode->NearNodes = new vector<TSortedNode*>();
		lbnode->FarNodes = new vector<TSortedNode*>();
		lbnode->FindNearNodes(rootNode);
	}
}

void stree::destroy()
{
	if ( rootNode ) delete rootNode; rootNode = 0;
	bottomNodes.clear();
}

vector<TSortedNode*>& stree::getBottomNodes()
{
	if (bottomNodes.empty()) {fprintf(stderr, "PANIC in stree::getBottomNodes()! Tree isn't built\n");}
	return bottomNodes;
}

TSortedNode* stree::findNode(TVec p)
{
	TSortedNode *Node = rootNode;

	while (Node->ch1)
	{
		if (Node->h < Node->w)
		{
			Node = (p.x < Node->x) ? Node->ch1 : Node->ch2 ;
		}
		else
		{
			Node = (p.y < Node->y) ? Node->ch1 : Node->ch2 ;
		}
	}
	return Node;
}

void stree::printBottomNodes(FILE* f, bool PrintDepth)
{
	for (auto& lbn: bottomNodes)
	{
		fprintf(f, "%g\t%g\t%g\t%g\t", lbn->x, lbn->y, lbn->w, lbn->h);
//		fprintf(f, "%g\t%g\t%g\t", lbn->CMp.rx, lbn->CMp.ry, lbn->CMp.g);
//		fprintf(f, "%g\t%g\t%g"  , lbn->CMm.rx, lbn->CMm.ry, lbn->CMm.g);
		if(PrintDepth) fprintf(f, "\t%d", lbn->i);
		fprintf(f, "\n");
	}
}

