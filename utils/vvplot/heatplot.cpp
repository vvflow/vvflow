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

class printer
{
	public:
		printer() {last=0;}
		void go(int percent)
		{
			if (clock()-last < CLOCKS_PER_SEC) return;
			fprintf(stderr, "%3d%%\r", percent);
			fflush(stderr);
			last = clock();
		}
	private:
		clock_t last;
};

bool PointIsInvalid(Space *S, TVec p)
{
	const_for(S->BodyList, llbody)
	{
		if ((**llbody).isPointInvalid(p)) return true;
	}
	return false;
}

double eps2h(const TSortedNode &Node, TVec p)
{
	TVec dr;
	double res1, res2;
	res2 = res1 = DBL_MAX;

	const_for(Node.NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.hRange.first; lobj < nnode.hRange.last; lobj++)
		{
			dr = p - *lobj;
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
		for (TObj *lobj = nnode.hRange.first; lobj < nnode.hRange.last; lobj++)
		{
			dr = p - *lobj;
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
			resh2 = min(resh2, (p-**llobj).abs2());
		}
		#undef nnode
	}

	return resh2;
}

double Temperature(Space* S, TVec p)
{
	double T=0;
	auto *hlist = S->HeatList;
	TSortedNode* bnode = S->Tree->findNode(p);

	//return  bnode->NearNodes->size_safe();
	TObj* nrst = Nearest(*bnode, p);

	//return nrst - S->HeatList->begin();

	const_for(bnode->NearNodes, llnnode)
	{
		#define nnode (**llnnode)
		for (TObj *lobj = nnode.hRange.first; lobj < nnode.hRange.last; lobj++)
		{
			double exparg = -(p-*lobj).abs2() * (*lobj).v.rx; // v.rx stores eps^(-2)
			T+= (exparg>-10) ? (*lobj).v.ry * exp(exparg) : 0; // v.ry stores g*eps(-2)
		}
		#undef nnode
	}

	T*= C_1_PI;

	double erfarg = h2(*bnode, p)/sqr(dl*EPS_MULT);
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
		     << "export VV_EPS_MULT=2 to smooth picture\n"
		     << "export VV_BODY_TEMP=1 to set body temperature\n";
		return -1;
	}

	char *mult_env = getenv("VV_EPS_MULT");
	EPS_MULT = mult_env ? atof(mult_env) : 2;
	char *body_t_env = getenv("VV_BODY_TEMP");
	BODY_TEMP = body_t_env ? atof(body_t_env) : 1;

	Space *S = new Space();
	S->Load(argv[1]);
	printer my_printer;
	S->VortexList = NULL;

	dl = S->AverageSegmentLength();
	S->Tree = new TSortedTree(S, 8, dl*20, DBL_MAX);
	S->Tree->build();

	#pragma omp parallel for
	const_for(S->Tree->getBottomNodes(), llbnode)
	{
		for (TObj *lobj = (**llbnode).hRange.first; lobj < (**llbnode).hRange.last; lobj++)
		{
			(*lobj).v.rx = 1./(sqr(EPS_MULT)*max(eps2h(**llbnode, *lobj), sqr(0.6*dl)));
			(*lobj).v.ry = (*lobj).v.rx*(*lobj).g;
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
			#pragma omp critical
			my_printer.go(++now*100/total);
		}
	}
	fout.close();

	return 0;
}

