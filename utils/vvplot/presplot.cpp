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
			TVec Vs = b.MotionSpeed_slae + b.RotationSpeed_slae * rotl(*latt - (b.Position + b.deltaPosition));
			double g = -Vs * latt->dl;
			double q = -rotl(Vs) * latt->dl;
			_2PI_Cp += (rotl(K(*latt, p))*g + K(*latt, p)*q) * Vs;
		}

		//second addend
		double gtmp = 0;
		const_for(b.List, latt)
		{
			TVec Vs = b.MotionSpeed_slae + b.RotationSpeed_slae * rotl(*latt - (b.Position + b.deltaPosition));
			gtmp+= latt->gsum;
			_2PI_Cp -= (latt->dl/S->dt * rotl(K(*latt, p))) * gtmp;
		}
		#undef b
	}

	const_for(S->VortexList, lobj)
	{
		_2PI_Cp+= (lobj->v * rotl(K(*lobj, p))) * lobj->g;
	}

	return C_1_2PI * _2PI_Cp + 0.5*(S->InfSpeed().abs2() - conv->SpeedSumFast(p).abs2());
}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		cerr << "Error! Please use: \npresplot file.vb precission xmin xmax ymin ymax\n";
		return -1;
	}

	Space *S = new Space();
	S->Load(argv[1]);

	double dl = S->AverageSegmentLength();
	tree tr(S, 8, dl*20, 0.1);
	S->Tree = &tr;
	convectivefast conv(S, dl*0.2);
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
