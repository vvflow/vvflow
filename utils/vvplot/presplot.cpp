#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include "core.h"
#include "convectivefast.h"
#include "epsfast.h"
#include "diffusivefast.h"
#include "flowmove.h"

const double S1Restriction = 1E-6;
const double ExpArgRestriction = -8.;
typedef void* pointer;
using namespace std;
#define dbg(a) { cerr << #a << "... "; a; cerr << "done\n"; }
//#define dbg(a) a

bool PointIsInvalid(Space *S, TVec p)
{
	const_for(S->BodyList, llbody)
	{
		if ((**llbody).PointIsInvalid(p)) return true;
	}
	return false;
}

void RotateAll(Space* S)
{
	const_for(S->BodyList, llbody)
	{
		(**llbody).Rotate((**llbody).RotSpeed(S->Time)*S->dt);
	}
}

FILE *fout;
double Rd2;

inline
TVec K(const TVec &obj, const TVec &p)
{
	TVec dr = p - obj;
	return dr/(dr.abs2() + Rd2);
}

double Pressure(Space* S, TVec p, double precision)
{
	double Cp=0;

	const_for(S->BodyList, llbody)
	{
		#define b (**llbody)
		double gtmp = 0;
		const_for(b.List, lobj)
		{
			gtmp+= b.att(lobj)->gsum;
			Cp -= (b.att(lobj)->dl * rotl(K(*lobj, p))) * gtmp;
		}

		double alpha = b.RotSpeed(S->Time)*S->dt;
		const_for(b.AttachList, latt)
		{
			Cp += ((latt->g*rotl(K(*latt, p)) + latt->q*K(*latt, p)) *
			      rotl(*latt-b.RotAxis)) * alpha;
		}
		#undef b
	}
	Cp/= S->dt;

	const_for(S->VortexList, lobj)
	{
		Cp+= (lobj->v*rotl(p-*lobj))*lobj->g / ((p-*lobj).abs2() + Rd2);
	}

	Cp *= C_1_2PI;
	Cp += 0.5*(S->InfSpeed().abs2() - (SpeedSumFast(p)).abs2());
	return Cp*2/S->InfSpeed().abs2();
}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		cerr << "Error! Please use: \npresplot file.vb precission xmin xmax ymin ymax\n";
		return -1;
	}

	Space *S = new Space(true, false, NULL);
	int N; double *header = S->Load(argv[1], &N);
	TBody* body = S->BodyList->at(0);

	double dl = body->AverageSegmentLength(); Rd2 = dl*dl/25;
	InitConvectiveFast(S, dl*dl/25);
	InitEpsilonFast(S, 0);
	InitDiffusiveFast(S, S->Re);
	flowmove fm(S, S->dt);
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
		dbg(fm.VortexShed());
	/******************************************/

	const_for(S->BodyList, llbody) { (**llbody).zero_variables(); }
	for (int i=0; i<1; i++)
	{
		S->ZeroSpeed();
		dbg(BuildTree());
		dbg(CalcEpsilonFast(false));
		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(CalcVortexDiffusiveFast());
		dbg(DestroyTree());

		RotateAll(S);
		dbg(fm.MoveAndClean(true, false));
		S->Time += S->dt;

		dbg(BuildTree());
		dbg(CalcCirculationFast(false));
		dbg(fm.VortexShed());
	}

	//требуется: выполнить условие непротекания, найти скорости вихрей (всех, включая присоединенные)
	int total = int((xmax-xmin)/prec + 1)*int((ymax-ymin)/prec + 1);
	int now=0;

	fstream fout;
	char fname[128];
	sprintf(fname, "%s.map", argv[1]);
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
			           0 : Pressure(S, TVec(x, y), prec);

			#pragma omp ordered
			{fout << x << "\t" << y << "\t" << t << endl;}
			#pragma omp critical
			cerr << (++now*100)/total << "% \r" << flush;
		}
	}
	fout.close();

	return 0;
}
