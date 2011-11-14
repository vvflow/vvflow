#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include "core.h"

using namespace std;

bool PointIsInvalid(Space *S, TVec p)
{
	const_for(S->BodyList, llbody)
	{
		if ((**llbody).PointIsInvalid(p)) return true;
	}
	return false;
}

FILE *fout;
double Rd2;

double Psi(Space* S, TVec p)
{
	double psi=0;

	const_for(S->BodyList, llbody)
	{
		#define b (**llbody)

		const_for(b.List, lobj)
		{
			psi += log((p-*lobj).abs2() + Rd2) * lobj->g;
		}
		//psi*= b.RotSpeed(S->Time);
		#undef b
	}

	TNode* Node = FindNode(p);
	if (!Node) return 0;
	const_for (Node->FarNodes, llfnode)
	{
		TObj obj = (**llfnode).CMp;
		psi+= log((p-obj).abs2() + Rd2)*obj.g;
		obj = (**llfnode).CMm;
		psi+= log((p-obj).abs2() + Rd2)*obj.g;
	}
	const_for (Node->NearNodes, llnnode)
	{
		#define vlist (**llnnode).VortexLList
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			if (!*llobj) continue;
			psi+= log((p-**llobj).abs2() + Rd2)*(**llobj).g;
		}
	}

	return -psi*0.5/C_2PI + p*rotl(S->InfSpeed());
}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		cout << "Error! Please use: \nstreamlines_exe file.vb precission xmin xmax ymin ymax\n";
		return -1;
	}

	Space *S = new Space(true, false, NULL);
	int N; double *header = S->Load(argv[1], &N);
	TBody* body = S->BodyList->at(0);

	double dl = body->AverageSegmentLength(); Rd2 = dl*dl/25;
	InitTree(S, 8, dl*20, 0.3);

	/**************************** LOAD ARGUMENTS ******************************/
	double xmin, xmax, ymin, ymax, prec;
	{
	int i=2;
	xmin = atof(argv[++i]);
	xmax = atof(argv[++i]);
	ymin = atof(argv[++i]);
	ymax = atof(argv[++i]);
	prec = atof(argv[2]);
	}
	/******************************************/

	BuildTree();

	int total = int((xmax-xmin)/prec + 1)*int((ymax-ymin)/prec + 1);
	int now=0;

	char fname[128];
	sprintf(fname, "%s.map", argv[1]);
	FILE *fout = fopen(fname, "w");

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
			           0 : Psi(S, TVec(x, y));

			#pragma omp ordered
			{fprintf(fout, "%lg \t%lg \t%lg\n", x, y, t);}
			#pragma omp critical
			cerr << (++now*100)/total << "% \r" << flush;
		}
		fprintf(fout, "\n");
	}
	fclose(fout);

	return 0;
}
