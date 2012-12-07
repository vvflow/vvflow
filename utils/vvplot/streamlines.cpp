#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include <time.h>

#include "core.h"

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

FILE *fout;
double Rd2;
TVec RefFrame_Speed;
inline double atan(TVec p) {return atan(p.ry/p.rx);}

double Psi(Space* S, TVec p)
{
	double psi1(0), psi2(0), psi3(0), psi4(0);

	const_for(S->BodyList, llbody)
	{
		#define b (**llbody)

		double psi_g_tmp=0, psi_q_tmp=0;

		if ((b.MotionSpeed_slae.abs() > 1e-5) || (b.RotationSpeed_slae > 1e-5))
		const_for(b.List, latt)
		{
			TVec Vs = b.MotionSpeed_slae + b.RotationSpeed_slae * rotl(*latt - (b.Position + b.deltaPosition));
			double g = -Vs * latt->dl;
			double q = -rotl(Vs) * latt->dl;
			psi_g_tmp+= log((p-*latt).abs2() + Rd2) * g;
			psi_q_tmp+= atan(p-*latt) * q;
		}
		psi1-= (psi_g_tmp*0.5 + psi_q_tmp) * C_1_2PI;

		const_for(b.List, latt)
		{
			psi2 += log((p-*latt).abs2() + Rd2) * latt->g;
		}

		#undef b
	}
	psi2*= -0.5*C_1_2PI;

	TNode* Node = S->Tree->findNode(p);
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

	return psi1 + psi2 + psi3 + p*rotl(RefFrame_Speed);
}

int main(int argc, char *argv[])
{
	if ( argc != 8 )
	{
		cerr << "Error! Please use: \nstreamlines_exe file.vb precission xmin xmax ymin ymax o|b|f\n";
		return -1;
	}

	Space *S = new Space();
	S->Load(argv[1]);
	printer my_printer;

	double dl = S->AverageSegmentLength(); Rd2 = dl*dl/25;
	S->Tree = new tree(S, 8, dl*20, 0.3);

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
	char RF = argv[7][1]?0:argv[7][0];
	switch (RF)
	{
		case 'f': RefFrame_Speed = TVec(0, 0); break;
		case 'b': cerr << "Not implemented yet\n"; return -1; RefFrame_Speed = TVec(1, 0); break;
		case 'o': RefFrame_Speed = S->InfSpeed(); break;
		default: cerr << "Bad reference frame" << endl; return -2;
	}
	/******************************************/

	S->Tree->build();

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
			my_printer.go(++now*100/total);
		}
		fprintf(fout, "\n");
	}
	fclose(fout);

	return 0;
}
