#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include "epsfast.h"

const double ExpArgRestriction = -8.;
double dl;
double EPS_MULT, BODY_TEMP;
using namespace std;

bool PointIsInvalid(Space *S, TVec p)
{
	const_for(S->BodyList, llbody)
	{
		if ((**llbody).PointIsInvalid(p)) return true;
	}
	return false;
}

double eps2h(const TNode &Node, TVec p)
{
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)

		auto *hlist = nnode.HeatLList;
		if (!hlist) {continue;}
		const_for (hlist, llobj)
		{
			if (!*llobj) { continue; }
			#define obj (**llobj)
			dr = p - obj;
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
			#undef obj
		}
		#undef nnode
	}

	if ( res1 == DBL_MAX ) return DBL_MIN;
	if ( res2 == DBL_MAX ) return sqrt(res1);

	return res2;
}

TObj* Nearest(TNode &Node, TVec p)
{
	TVec dr(0, 0);
	double resr = DBL_MAX;
	TObj *res = NULL;

	const_for(Node.NearNodes, llnnode)
	{
		TNode &nnode = **llnnode;

		auto list = nnode.HeatLList;
		if ( !list ) { continue; }
		const_for (list, llobj)
		{
			dr = p - **llobj;
			double drabs2 = dr.abs2();
			if ( !drabs2 ) continue;
			if ( drabs2 <= resr )
			{
				res = *llobj;
				resr = drabs2;
			}
		}
	}

	return res;
}

double h(TNode &Node, TVec p)
{
	double resh = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		TNode &nnode = **llnnode;

		auto blist = nnode.BodyLList;
		if ( !blist ) { continue; }
		const_for (blist, llobj)
		{
			resh = min(resh, (p-**llobj).abs());
		}
	}

	return resh;
}

double Temperature(Space* S, TVec p)
{
	double T=0;
	auto *hlist = S->HeatList;
	TNode* bnode = FindNode(p);

	//return  bnode->NearNodes->size_safe();
	TObj* nrst = Nearest(*bnode, p);

	//return nrst - S->HeatList->begin();

	const_for(bnode->NearNodes, llnnode)
	{
		#define nnode (**llnnode)

		auto *hlist = nnode.HeatLList;
		if (!hlist) {continue;}
		const_for (hlist, llobj)
		{
			double exparg = -(p-**llobj).abs2() * (**llobj).v.rx; // v.rx stores eps^(-2)
			T+= (exparg>-100) ? (**llobj).v.ry * exp(exparg) : 0; // v.ry stores g*eps(-2)
		}
		#undef nnode
	}

	T*= C_1_PI;

	double erfarg = h(*bnode, p)*nrst->v.rx; //bnode->CMp.g stores h
	T+= (erfarg<3) ? 0.5*(1-erf(erfarg)) : 0;

	return T;
}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		cerr << "Error! Please use: \nheatplot file.vb "
		     << "precision xmin xmax ymin ymax \n"
		     << "Also you can use enviroment variables:\n"
		     << "export HEATPLOT_EPS_MULT=2 to smooth picture\n"
		     << "export HEATPLOT_BODY_TEMP=1 to set body temperature\n";
		return -1;
	}

	char *mult_env = getenv("HEATPLOT_EPS_MULT");
	EPS_MULT = mult_env ? atof(mult_env) : 2;
	char *body_t_env = getenv("HEATPLOT_BODY_TEMP");
	BODY_TEMP = body_t_env ? atof(body_t_env) : 1;

	Space *S = new Space();
	S->Load(argv[1]);
	S->VortexList = NULL;

	dl = S->AverageSegmentLength();
	InitTree(S, 8, dl*20, DBL_MAX);
	BuildTree();

	#pragma omp parallel for
	const_for(GetTreeBottomNodes(), llbnode)
	{
		if (!(**llbnode).HeatLList) continue;
		const_for((**llbnode).HeatLList, llobj)
		{
			(**llobj).v.rx = 1./(sqr(EPS_MULT)*max(eps2h(**llbnode, **llobj), 0.6*dl));
			(**llobj).v.ry = (**llobj).v.rx*(**llobj).g;
		}
	}

	/************** CROP FIELD ****************/
	double prec = atof(argv[2]);
	double xmin = atof(argv[3]);
	double xmax = atof(argv[4]);
	double ymin = atof(argv[5]);
	double ymax = atof(argv[6]);
	/******************************************/

	int total = int((xmax-xmin)/prec + 1)*int((ymax-ymin)/prec + 1);
	int now=0;

	fstream fout;
	char fname[128];
	sprintf(fname, "%s.heat", argv[1]);
	fout.open(fname, ios::out);

	int imax = (xmax-xmin)/prec + 1;
	int jmax = (ymax-ymin)/prec + 1;
	for( int i=0; i<imax; i++)
	{
		double x = xmin + double(i)*prec;
		#pragma omp parallel for ordered schedule(dynamic, 1)
		for( int j=0; j<jmax; j++)
		{
			double y = ymin + double(j)*prec;
			double t = PointIsInvalid(S, TVec(x, y)) ?
			           BODY_TEMP :
			           Temperature(S, TVec(x, y));

			#pragma omp ordered
			{fout << x << "\t" << y << "\t" << t << endl;}
			//#pragma omp critical
			//cerr << (++now*100)/total << "% \r" << flush;
		}
	}
	fout.close();

	return 0;
}

