#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include "core.h"
#include "convectivefast.h"

const double S1Restriction = 1E-6;
const double ExpArgRestriction = -8.;
typedef void* pointer;
using namespace std;
#define dbg(a) { cerr << #a << "... "; a; cerr << "done\n"; }


TVec RotSpeed(TVec p, TVec axis, double speed)
{
	return rotl(p-axis)*speed;
}

double Rd2;
TBody *naca, *spoiler;
double Pressure(Space* S, TVec p, double precision)
{
	double Cp=0; 

	const_for(S->BodyList, llbody)
	{
		TBody &b = **llbody;
		const_for(b.List, lobj)
		{
			double gtmp = 0;
			for (TObj* lk = b.List->begin(); lk<lobj; lk++) {gtmp+=lk->g;}
			Cp -= ((TVec(*b.next(lobj))-TVec(*lobj)) * rotl(p-*lobj))*gtmp/((p-*lobj).abs2() + Rd2);
		}
	}
	Cp/= S->dt;

	Cp += 0.5*(S->InfSpeed().abs2() - SpeedSumFast(p).abs2());

	const_for(S->VortexList, lobj)
	{
		Cp+= (lobj->v*rotl(p-*lobj))*lobj->g / ((p-*lobj).abs2() + Rd2);
		cout << lobj->v << endl;
	}
	const_for(spoiler->AttachList, latt)
	{
		Cp+= rotl(TVec(*latt) - spoiler->Motion(0).v) * spoiler->Motion(0).g *
		     rotl(p-*latt)*latt->g / ((p-*latt).abs2() + Rd2) ;
	}
	

	return Cp*2;
}

TVec InfSpeed(double t){return TVec(1, 0);}
const double spoiler_time = 4;
const double spoiler_angle = C_PI/2;
TObj rot(double t)
{
	if ((t>= 5) && (t< 5+spoiler_time ))
	{
		return TObj(0,0,spoiler_angle/spoiler_time);
	}
	if ((t>=  15) && (t< 15+spoiler_time ))
	{
		return TObj(0,0,-spoiler_angle/spoiler_time);
	}
	return TObj(0,0,0);
}

int main(int argc, char *argv[])
{
	if ( argc != 4)\
	{
		cout << "Error! Please use: \npresplot vortex.file info.file"
		     << "precision\n";
//		     << "Also you can use enviroment variables:\n"
//		     << "export HEATPLOT_EPS_MULT=2 to smooth picture\n"
//		     << "export HEATPLOT_BODY_TEMP=1 to set body temperature\n";
		return -1;
	}

//	char *mult_env = getenv("PREPLOT_EPS_MULT");
//	double mult = mult_env ? atof(mult_env) : 2;
	Space *S = new Space(true, false, InfSpeed);
	S->dt = 2E-3;
	S->LoadBody("naca0012_body"); naca = S->BodyList->at(0);
	S->LoadBody("naca0012_spoiler"); spoiler = S->BodyList->at(1);
	S->LoadVorticityFromFile(argv[1]);
	double dl = naca->AverageSegmentLength(); Rd2 = sqr(dl)*0.25;
	cout << dl << endl;
	InitConvectiveFast(S, Rd2);
	InitTree(S, 8, dl*20, 0.3);

	/**************************** LOAD INFO ***********************************/
	double time, angle;
	{
	FILE* fin = fopen(argv[2], "r");
	char line[128];
	fgets(line, 127, fin); //1
	fgets(line, 127, fin); sscanf(line, "%lf", &time); //2
	fgets(line, 127, fin); //3
	fgets(line, 127, fin); //4
	fgets(line, 127, fin); sscanf(line, "%lf", &angle); //5
	fclose(fin);
	}
	S->Time = time;
	spoiler->SetMotion(rot, TVec(0.700122, 0.0355247));
	spoiler->UpdateAttach();
	spoiler->Transform(TObj(0,0,angle));

	/**************************** LOAD ARGUMENTS ******************************/
	double xmin, xmax, ymin, ymax, prec;
	{
	int i=2;
	xmin = -0.1; //atof(argv[++i]);
	xmax = 1.4; //atof(argv[++i]);
	ymin = -0.5; //atof(argv[++i]);
	ymax = 0.5; //atof(argv[++i]);
	prec = atof(argv[3]);
	}
	/******************************************/

	dbg(BuildTree());
//	dbg(CalcCirculationFast(false));
	dbg(CalcConvectiveFast());
//	dbg(CalcBoundaryConvective());
//	dbg(DestroyTree());
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
			double t = (naca->PointIsInvalid(TVec(x, y)) || spoiler->PointIsInvalid(TVec(x, y))) ? 
			           0 : Pressure(S, TVec(x, y), prec);

			#pragma omp ordered
			{fout << x << "\t" << y << "\t" << t << endl;}
			cerr << (++now*100)/total << "% \r" << flush;
		}
	}
	fout.close();

	return 0;
}

