#ifndef TREE_H_
#define TREE_H_
#include "core.h"

struct node
{
	double x, y, h, w;
	int i, j; //debug
	TObj CMp, CMm;
	
	vector<TObj*> *VortexLList;
	vector<TObj*> *BodyLList;
	vector<TObj*> *HeatLList;

	vector<struct node*> *NearNodes;
	vector<struct node*> *FarNodes;

	struct node *Child1;
	struct node *Child2;
};
typedef struct node TNode; 

void InitTree(Space *sS, int sFarCriteria, double sMinNodeSize);
void BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat);
void DestroyTree();

double GetAverageNearNodesPercent();
double GetAverageNearNodesCount();
vector<TNode*>* GetTreeBottomNodes();

TNode* FindNode(TVec p);
int GetMaxDepth();
int PrintBottomNodes(std::ostream& os, bool PrintDepth = false); // prints lines such "x y w h [i]" 
void PrintLevel(std::ostream& os, int level);

#endif /*TREE_H_*/
