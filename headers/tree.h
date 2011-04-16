#ifndef TREE_H_
#define TREE_H_
#include "core.h"

struct node
{
	double x, y, h, w;
	int i, j; //debug
	TVortex CMp, CMm;
	
	TList<TObject*> *VortexLList;
	TList<TObject*> *BodyLList;
	TList<TObject*> *HeatLList;

	TList<struct node*> *NearNodes;
	TList<struct node*> *FarNodes;

	struct node *Child1;
	struct node *Child2;
};
typedef struct node TNode; 

void InitTree(Space *sS, int sFarCriteria, double sMinNodeSize);
void BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat);
void DestroyTree();

double GetAverageNearNodesPercent();
double GetAverageNearNodesCount();
TList<TNode*>* GetTreeBottomNodes();

TNode* FindNode(double px, double py);
int GetMaxDepth();
int PrintBottomNodes(std::ostream& os, bool PrintDepth = false); // prints lines such "x y w h [i]" 
void PrintLevel(std::ostream& os, int level);

#endif /*TREE_H_*/
