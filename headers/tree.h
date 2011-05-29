#ifndef TREE_H_
#define TREE_H_
#include "core.h"

class node
{
	public:
		node();
		~node();

		double x, y, h, w;
		int i, j; //debug
		TObj CMp, CMm;

		typedef vector<TObj*> content;
		content *VortexLList;
		content *BodyLList;
		content *HeatLList;

		vector<node*> *NearNodes;
		vector<node*> *FarNodes;

		node *ch1;
		node *ch2;
	public:
		void DivideNode();
		void DistributeContent(content *parent, content **ch1, content **ch2);
		void Stretch();
		void Destroy();
		void CalculateCMass();
		void CalculateCMassFromScratch();
		void FindNearNodes(node *TopNode);
};
typedef node TNode;

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
