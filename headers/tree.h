#ifndef TREE_H_
#define TREE_H_

#include <vector>
#include <algorithm>
using namespace std;

#include "core.h"

class TNode
{
	public:
		TNode();
		TNode(bool CreateVortexes, bool CreateBody, bool CreateHeat);
		~TNode();
		void del();

		void stretch();
		void divide();
		void calc_cmass();
		void calc_cmass_from_scratch();
		void find_near_nodes(TNode *top);

		TVec center, size;
		TObj CMp, CMm; //center of mass

		vector<TObj*> *VortexLList;
		vector<TObj*> *BodyLList;
		vector<TObj*> *HeatLList;

		vector<TNode*> *near_nodes;
		vector<TNode*> *far_nodes;

		TNode *ch1;
		TNode *ch2;
};

void InitTree(Space *sS, int sFarCriteria, double sMinNodeSize);
void BuildTree(bool IncludeVortexes, bool IncludeBody, bool IncludeHeat);
void DestroyTree();

TNode* FindNode(TVec p);
vector<TNode*>* GetTreeBottomNodes();

/*double GetAverageNearNodesPercent();
double GetAverageNearNodesCount();

int GetMaxDepth();
int PrintBottomNodes(std::ostream& os, bool PrintDepth = false); // prints lines such "x y w h [i]" 
void PrintLevel(std::ostream& os, int level);
*/
#endif /*TREE_H_*/
