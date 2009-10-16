#ifndef TREE_H_
#define TREE_H_
#include "core.h"

struct node
{
	double x, y, h, w;
	int i, j; //debug
	TVortex CMp, CMm;
	
	TlList *VortexLList;
	TlList *BodyLList;
	TlList *HeatLList;

	TlList *NearNodes;
	TlList *FarNodes;
};
typedef struct node TNode; 

int InitTree(Space *sS, int sTotalDepth, int sFarCriteria, double sMinNodeSize);
int BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat);
int DestroyTree();

double GetAverageNearNodesPercent();
double GetAverageNearNodesCount();
TlList* GetTreeBottomNodes();

TNode* FindNode(double px, double py); // doesnt work


#endif /*TREE_H_*/
