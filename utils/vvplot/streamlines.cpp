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
inline double atan(TVec p) {return atan(p.ry/p.rx);}

double Psi(Space* S, TVec p)
{
	double psi1(0), psi2(0), psi3(0), psi4(0);

	const_for(S->BodyList, llbody)
	{
		#define b (**llbody)

		double psi_g_tmp=0, psi_q_tmp=0;
		if (b.RotSpeed(S->Time))
		const_for(b.AttachList, latt)
		{
			psi_g_tmp+= log((p-*latt).abs2() + Rd2) * latt->g;
			psi_q_tmp+= atan(p-*latt) * latt->q;
		}
		psi1-= (psi_g_tmp*0.5 + psi_q_tmp) * b.RotSpeed(S->Time) * C_1_2PI;

		const_for(b.List, lobj)
		{
			psi2 += log((p-*lobj).abs2() + Rd2) * lobj->g;
		}
		#undef b
	}
	psi2*= -0.5*C_1_2PI;

	TNode* Node = FindNode(p);
	if (!Node) return 0;
	const_for (Node->FarNodes, llfnode)
	{
		TObj obj = (**llfnode).CMp;
		psi3+= log((p-obj).abs2() + Rd2)*obj.g;
		obj = (**llfnode).CMm;
		psi3+= log((p-obj).abs2() + Rd2)*obj.g;
	}
	const_for (Node->NearNodes, llnnode)
	{
		#define vlist (**llnnode).VortexLList
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			if (!*llobj) continue;
			psi3+= log((p-**llobj).abs2() + Rd2)*(**llobj).g;
		}
	}
	psi3*= -0.5*C_1_2PI;

	return psi1 + psi2 + psi3 + p*rotl(S->InfSpeed());
}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		cerr << "Error! Please use: \nstreamlines_exe file.vb precission xmin xmax ymin ymax\n";
		return -1;
	}

	Space *S = new Space();
	S->Load(argv[1]);

	double dl = S->AverageSegmentLength(); Rd2 = dl*dl/25;
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
	sprintf(fname, "%s.psi", argv[1]);
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
			double t = Psi(S, TVec(x, y));

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
