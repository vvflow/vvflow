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
const int imax=10;

TVec RotSpeed(TVec p, TVec axis, double speed)
{
	return rotl(p-axis)*speed;
}

FILE *fout;
double Rd2;
double Pressure(Space* S, TVec p, double precision)
{
	double Cp=0;

	const_for(S->BodyList, llbody)
	{
//		TBody &b = **llbody;
		#define b (**llbody)
		double gtmp = 0;
		const_for(b.List, lobj)
		{
			gtmp+= b.att(lobj)->gsum;
			Cp -= b.att(lobj)->dl * rotl(p-*lobj)*gtmp/((p-*lobj).abs2() + Rd2);
		}
		#undef b
	}
	Cp/= imax*S->dt;

	const_for(S->VortexList, lobj)
	{
//		Cp+= (lobj->v*rotl(p-*lobj))*lobj->g / ((p-*lobj).abs2() + Rd2);
	}

	Cp *= C_1_2PI;

	//Cp += 0.5*(S->InfSpeed().abs2() - (SpeedSumFast(p)+S->InfSpeed()).abs2());
/*	#pragma omp critical
	{
		TVec tmp = SpeedSumFast(p)+S->InfSpeed();
		fprintf(fout, "%lf\t %lf\t %lf\t %lf\n", p.rx, p.ry, tmp.rx, tmp.ry);
	}
*/

	return Cp*2/S->InfSpeed().abs2();
}

TVec InfSpeed(double t){return TVec(0.1, 0);}

int main(int argc, char *argv[])
{
	if ( argc != 7)\
	{
		cout << "Error! Please use: \npresplot file.vb precission\n";
//		     << "Also you can use enviroment variables:\n"
//		     << "export HEATPLOT_EPS_MULT=2 to smooth picture\n"
//		     << "export HEATPLOT_BODY_TEMP=1 to set body temperature\n";
		return -1;
	}

//	char *mult_env = getenv("PREPLOT_EPS_MULT");
//	double mult = mult_env ? atof(mult_env) : 2;
	Space *S = new Space(true, false, InfSpeed);
	int N; double *header = S->Load(argv[1], &N);
	if (!S->BodyList->size_safe()) return -1;
	TBody* body = S->BodyList->at(0); 

	double dl = body->AverageSegmentLength(); Rd2 = dl*dl/25;
	InitConvectiveFast(S, dl*dl/25);
	InitEpsilonFast(S, 0);
	InitDiffusiveFast(S, 200);
	flowmove fm(S, 0.005/imax);
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

	body->zero_variables();
	for (int i=0; i<imax; i++)
	{
		dbg(BuildTree());
		dbg(CalcEpsilonFast(false));
//		dbg(CalcBoundaryConvective());
		dbg(CalcConvectiveFast());
		dbg(CalcVortexDiffusiveFast());
		dbg(DestroyTree());

		dbg(fm.MoveAndClean(true, false));
		cerr << "Removed " << fm.CleanedV() << endl;
		S->Time += S->dt;

		dbg(BuildTree());
		dbg(CalcCirculationFast(false));
		dbg(fm.VortexShed());
	}

	S->Save("copy%d", header, N);
	fout = S->OpenFile("vel%d");
	const_for(body->List, lobj)
	{
		fprintf(fout, "%lf\t %lf\t %lf\t %lf\n", lobj->rx, lobj->ry, lobj->g, body->att(lobj)->gsum);
	}
	fclose(fout);

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
			double t = body->PointIsInvalid(TVec(x, y)) ? 
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

