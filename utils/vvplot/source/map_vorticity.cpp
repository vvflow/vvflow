#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include "hdf5.h"


#include "libvvplot_api.h"
#include "flowmove.h"
#include "epsfast.h"

static const double ExpArgRestriction = -8.;
static double dl;
static double EPS_MULT;
using namespace std;

double eps2h(const TSortedNode &Node, TVec p)
{
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			dr = p - lobj->r;
			double drabs2 = dr.abs2();
			if ( drabs2 ) {
			if ( res1 > drabs2 )
			{
				res2 = res1;
				res1 = drabs2;
			}
			else if ( res2 > drabs2 )
			{
				res2 = drabs2;
			}}
		}
		#undef nnode
	}

	if ( res1 == DBL_MAX ) return DBL_MIN;
	if ( res2 == DBL_MAX ) return DBL_MIN;//res1;

	return res2;
}

TObj* Nearest(TSortedNode &Node, TVec p)
{
	TVec dr(0, 0);
	double resr = DBL_MAX;
	TObj *res = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			dr = p - lobj->r;
			double drabs2 = dr.abs2();
			if ( !drabs2 ) continue;
			if ( drabs2 <= resr )
			{
				res = lobj;
				resr = drabs2;
			}
		}
		#undef nnode
	}

	return res;
}

double h2(TSortedNode &Node, TVec p)
{
	double resh2 = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		auto blist = nnode.BodyLList;
		if ( !blist ) { continue; }
		const_for (blist, llobj)
		{
			resh2 = min(resh2, (p-(**llobj).r).abs2());
		}
		#undef nnode
	}

	return resh2;
}

double Vorticity(Space* S, TVec p)
{
	double T=0;
	auto *hlist = S->VortexList;
	TSortedNode* bnode = S->Tree->findNode(p);

	//return  bnode->NearNodes->size_safe();
	TObj* nrst = Nearest(*bnode, p);

	//return nrst - S->HeatList->begin();

	const_for(bnode->NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.vRange.first; lobj < nnode.vRange.last; lobj++)
		{
			double exparg = -(p-lobj->r).abs2() * lobj->v.x; // v.rx stores eps^(-2)
			T+= (exparg>-10) ? lobj->v.y * exp(exparg) : 0; // v.ry stores g*eps(-2)
		}
		#undef nnode
	}

	T*= C_1_PI;

	double erfarg = h2(*bnode, p)/sqr(dl*EPS_MULT);
	T+= (erfarg<3) ? 0.5*(1-erf(erfarg)) : 0;

	return T;
}

extern "C" {
int map_vorticity(hid_t fid, double xmin, double xmax, double ymin, double ymax, double spacing)
{
	char *mult_env = getenv("VV_EPS_MULT");
	EPS_MULT = mult_env ? atof(mult_env) : 2;

	/**************************************************************************
	*** MAIN JOB **************************************************************
	**************************************************************************/

	Space *S = new Space();
	S->Load(fid);
	flowmove fm(S);
	fm.VortexShed();
	S->HeatList = NULL;

	dl = S->AverageSegmentLength();
	S->Tree = new TSortedTree(S, 8, dl*20, DBL_MAX);
	S->Tree->build();

	#pragma omp parallel for
	const_for(S->Tree->getBottomNodes(), llbnode)
	{
		for (TObj *lobj = (**llbnode).vRange.first; lobj < (**llbnode).vRange.last; lobj++)
		{
			lobj->v.x = 1./(sqr(EPS_MULT)*max(eps2h(**llbnode, lobj->r), sqr(0.6*dl)));
			lobj->v.y = lobj->v.x * lobj->g;
		}
	}

	// Calculate field ********************************************************
	hsize_t dims[2] = {(xmax-xmin)/spacing + 1, (ymax-ymin)/spacing + 1};
	float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);

	for (int xi=0; xi<dims[0]; xi++)
	{
		double x = xmin + double(xi)*spacing;
		#pragma omp parallel for ordered schedule(dynamic, 100)
		for( int yj=0; yj<dims[1]; yj++)
		{
			double y = ymin + double(yj)*spacing;
			TVec xy(x,y);
			mem[xi*dims[1]+yj] = S->PointIsInBody(xy) ? 0 : Vorticity(S, xy);
		}
	}

	/**************************************************************************
	*** SAVE RESULTS **********************************************************
	**************************************************************************/

	map_save(fid, "map_vorticity", mem, dims, xmin, xmax, ymin, ymax, spacing);
	free(mem);

	return 0;
}}
