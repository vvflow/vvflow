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
		if ((**llbody).isPointInvalid(p)) return true;
	}
	return false;
}

FILE *fout;
double Rd2;
char RefFrame;

inline
TVec K(const TVec &obj, const TVec &p)
{
	TVec dr = p - obj;
	return dr/(dr.abs2() + Rd2);
}

double Pressure(Space* S, convectivefast *conv, TVec p, double precision)
{
	double _2PI_Cp=0;

	const_for(S->BodyList, llbody)
	{
		#define b (**llbody)
		//first addend
		const_for(b.List, latt)
		{
			TVec Vs = b.Speed_slae.r + b.Speed_slae.o * rotl(latt->r - (b.pos.r + b.dPos.r));
			double g = -Vs * latt->dl;
			double q = -rotl(Vs) * latt->dl;
			_2PI_Cp += (rotl(K(latt->r, p))*g + K(latt->r, p)*q) * Vs;
		}

		//second addend
		double gtmp = 0;
		const_for(b.List, latt)
		{
			TVec Vs = b.Speed_slae.r + b.Speed_slae.o * rotl(latt->r - (b.pos.r + b.dPos.r));
			gtmp+= latt->gsum;
			_2PI_Cp -= (latt->dl/S->dt * rotl(K(latt->r, p))) * gtmp;
		}
		#undef b
	}

	const_for(S->VortexList, lobj)
	{
		_2PI_Cp+= (lobj->v * rotl(K(lobj->r, p))) * lobj->g;
	}

	TVec LocalSpeed = conv->SpeedSumFast(p);
	double Cp_static = C_1_2PI * _2PI_Cp + 0.5*(S->InfSpeed().abs2() - LocalSpeed.abs2());
	switch (RefFrame)
	{
		case 's': return Cp_static;
		case 'o': return Cp_static + 0.5*(LocalSpeed.abs2());
		case 'f': return Cp_static + 0.5*((LocalSpeed - S->InfSpeed()).abs2());
		case 'b': return Cp_static + 0.5*((LocalSpeed - S->InfSpeed() + TVec(1, 0)).abs2());
		default: cerr << "Bad reference frame!!!" << endl; return -2;
	}
	return Cp_static; // not used
}

int main(int argc, char *argv[])
{
	if ( argc != 8)\
	{
		cerr << "Error! Please use: \npresplot file.vb precission xmin xmax ymin ymax s|o|b|f\n";
		return -1;
	}

	Space *S = new Space();
	S->Load(argv[1]);

	double dl = S->AverageSegmentLength();
	TSortedTree tr(S, 8, dl*20, 0.1);
	S->Tree = &tr;
	convectivefast conv(S);
	epsfast eps(S);
	diffusivefast diff(S);
	flowmove fm(S);

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
	RefFrame = argv[7][1]?0:argv[7][0];
	switch(RefFrame)
	{
		case 's': case 'o': case 'b': case 'f': break;
		default: cerr << "Bad reference frame" << endl; return -2;
	}

	fm.VortexShed();

	//дано: условие непротекания выполнено, неизвестные вихри найдены и рождены.
	//требуется: найти скорости вихрей (всех, включая присоединенные) и посчитать давление
	tr.build();
	eps.CalcEpsilonFast(false);
	conv.CalcBoundaryConvective();
	conv.CalcConvectiveFast();
	diff.CalcVortexDiffusiveFast();

	int total = int((xmax-xmin)/prec + 1)*int((ymax-ymin)/prec + 1);
	int now=0;

	fstream fout;
	char fname[128];
	sprintf(fname, "%s.%c.map", argv[1], RefFrame);
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
			           0 : Pressure(S, &conv, TVec(x, y), prec);

			#pragma omp ordered
			{fout << x << "\t" << y << "\t" << t << endl;}
			#pragma omp critical
			cerr << (++now*100)/total << "% \r" << flush;
		}
	}
	fout.close();

	return 0;
}
