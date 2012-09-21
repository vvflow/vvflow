#ifndef TREE_H_
#define TREE_H_

class node; typedef node TNode;
class tree; typedef tree TTree;

#include "core.h"
#include <stdio.h>
#include <float.h>

class node
{
	public:
		node(tree *sParent);
		~node();

		double x, y, h, w;
		int i, j; //debug
		TObj CMp, CMm;

		typedef vector<TObj*> content;
		content *VortexLList;
		content *BodyLList;
		content *HeatLList;
		content *StreakLList;

		vector<node*> *NearNodes;
		vector<node*> *FarNodes;

		node *ch1;
		node *ch2;
	public:
		tree *parent;
		void DivideNode();
		void DistributeContent(content *parent, content **ch1, content **ch2);
		void Stretch();
		void Destroy();
		void CalculateCMass();
		void CalculateCMassFromScratch();
		void FindNearNodes(node *TopNode);
};

class tree
{
	public:
		tree(Space *sS, int sFarCriteria, double sMinNodeSize, double sMaxNodeSize = DBL_MAX);
		//~tree();

		void build(bool IncludeVortexes = true, bool IncludeBody = true, bool IncludeHeat = true);
		void destroy();

		vector<TNode*>* getBottomNodes();
		TNode* findNode(TVec p);

		void printBottomNodes(FILE* f, bool PrintDepth = false); // prints lines such "x y w h [i]"

	private:
		Space *S;
		int farCriteria;
		double minNodeSize;
		double maxNodeSize;

		TNode *rootNode;
		vector<TNode*> *bottomNodes;
		template <class T>
		void fillRootNode(vector<T> *src, node::content **dst);

		friend class node;
};

#endif /*TREE_H_*/
