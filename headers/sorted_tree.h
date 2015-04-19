#ifndef SORTEDTREE_H_
#define SORTEDTREE_H_

class snode; typedef snode TSortedNode;
class stree; typedef stree TSortedTree;

#include "core.h"
#include <stdio.h>
#include <float.h>
#include <vector>

using std::vector;

class range
{
    public:
        TObj *first, *last;
    public:
        range() { first = last = NULL;}
        range(TObj *f, TObj *l) { first = f; last = l; }
        void set(TObj *f, TObj *l) { first = f; last = l; }
        int size() { return last-first; }
};

class snode
{
    public:
        snode(stree *sParent);
        ~snode();

        double x, y, h, w;
        int i, j; //debug
        TObj CMp, CMm;

        typedef vector<TObj*> LList;
        LList bllist;
        range vRange;
        range hRange;
        range sRange;

        vector<snode*> *NearNodes;
        vector<snode*> *FarNodes;

        snode *ch1;
        snode *ch2;
    public:
        stree *parent;
        void DivideNode();
        void DistributeContent(range& parent, range *ch1, range *ch2);
        void DistributeContent(LList& parent, LList *ch1, LList *ch2);
        void Stretch();
        void Stretch(range &oRange, TVec &tr, TVec &bl);
        void Destroy();
        void CalculateCMass();
        void CalculateCMassFromScratch();
        void FindNearNodes(snode *TopNode);
};

class stree
{
    public:
        stree(Space *sS, int sFarCriteria, double sMinNodeSize, double sMaxNodeSize = DBL_MAX);
        //~tree();

        void build(bool IncludeVortexes = true, bool IncludeBody = true, bool IncludeHeat = true);
        void destroy();

        vector<snode*>& getBottomNodes();
        snode* findNode(TVec p);

        void printBottomNodes(FILE* f, bool PrintDepth = false); // prints lines such "x y w h [i]"

    private:
        Space *S;
        int farCriteria;
        double minNodeSize;
        double maxNodeSize;

        snode *rootNode;
        vector<snode*> bottomNodes;

        friend class snode;
};

#endif /*SORTEDTREE_H_*/
