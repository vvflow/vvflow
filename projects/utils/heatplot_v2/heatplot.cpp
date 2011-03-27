#include "libVVHD/core.h"
#include "math.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>

#define M_2PI 6.283185308
#ifndef M_1_PI
	#define M_1_PI 0.318309886
#endif
#define expdef(x) exp(x)
#define MARGIN 0.1
using namespace std;

void Epsilon(TList<TObject> *List, double px, double py, double &res, double precision)
{
	double drx, dry, drabs2;
	double res1, res2;
	res2 = res1 = 1E10;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = drx*drx + dry*dry;
		if ( !drabs2 ) continue;
		if ( res1 > drabs2 ) { res2 = res1; res1 = drabs2;}
		else if ( res2 > drabs2 ) { res2 = drabs2; }
	}
	res = res1;
	//if (res < precision) res = precision;
}

TObject* Nearest(TList<TObject> *List, double px, double py)
{
	double drx, dry, drabs2;
	double resr = 1E10;
	TObject *res = NULL;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		drx = px - Obj->rx;
		dry = py - Obj->ry;
		drabs2 = drx*drx + dry*dry;
		if ( !drabs2 ) continue;
		if ( drabs2 <= resr )
		{
			res = Obj;
			resr = drabs2;
		}
	}

	return res;
}

double Temperature(Space* S, double px, double py, double precision)
{
	double res=0;
	double drx, dry, drabs2;

	TList<TObject> *List = S->HeatList;
	double g = List->First->g;
	TObject* nrst = Nearest(List, px, py);

	if (!nrst->vx)
		{ Epsilon(S->HeatList, nrst->rx, nrst->ry, nrst->vx, precision); }
	double eps2 = nrst->vx;//vx isnt actually a vx, its eps stored in vx.


	double multiplier = M_PI*eps2/2;
	double v_1_eps2 = 1/eps2;
	// check if there r no particles beside	
	#define dx (px-nrst->rx)
	#define dy (py-nrst->ry)
	//if ( sqrt(dx*dx+dy*dy)/eps2 > 10 ) return 0;
	#undef dx
	#undef dy
	//end of check
	{
		TObject *Obj = List->First;
		TObject *&LastObj = List->Last;
		for (; Obj<LastObj; Obj++)
		{
			drx = px - Obj->rx;
			dry = py - Obj->ry;
			drabs2 = drx*drx + dry*dry;

			double exparg = -drabs2*v_1_eps2;
			if (exparg > -8) res+= expdef(exparg);
		}
	}

	double h = sqrt(px*px+py*py)-1;
	double erfarg = h*sqrt(v_1_eps2);
	if (erfarg<3)
		res+= multiplier*(1-erf(erfarg));

	res*= g*v_1_eps2*M_1_PI;
	return res;
}

void CutField(Space *S, double xmin, double xmax, double ymin, double ymax)
{
	TList<TObject> *List = S->HeatList;
	if ( !List ) return;

	double dx=xmax-xmin, dy=ymax-ymin;
	double xmin1=xmin-(dx*MARGIN), xmax1=xmax+(dx*MARGIN), ymin1=ymin-(dy*MARGIN), ymax1=ymax+(dy*MARGIN);

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for ( ; Obj<LastObj; Obj++)
	{
		if ( (Obj->rx < xmin1) || (Obj->rx > xmax1) || (Obj->ry < ymin1) || (Obj->ry > ymax1) )
		{
			List->Remove(Obj);
			Obj--;
		}
	}
}

int main(int argc, char *argv[])
{
	if ( argc != 7) { cout << "Error! Please use: \nheatplot filename xmin xmax ymin ymax precision\n"; return -1; }

	Space *S = new Space(0, 0, 1, NULL, NULL, NULL);
	S->LoadHeatFromFile(argv[1]);
	cerr << "Before crop: " << S->HeatList->size;
	//S->LoadHeatFromStupidFile(argv[1], 0.15791367E-03);

	double xmin, xmax, ymin, ymax, precision;
	sscanf(argv[2], "%lf", &xmin);
	sscanf(argv[3], "%lf", &xmax);
	sscanf(argv[4], "%lf", &ymin);
	sscanf(argv[5], "%lf", &ymax);
	sscanf(argv[6], "%lf", &precision); 
	CutField(S, xmin, xmax, ymin, ymax);
	//Epsilon(S->HeatList, precision);
	cerr << "\t\tAfter crop: " << S->HeatList->size << endl;

	//GNUPLOT part

	/*FILE *pipe = fopen("gnuplot","w");
	fprintf(pipe, "unset key\n");
	fprintf(pipe, "set xrange [%lf:%lf]; set yrange [%lf:%lf]\n", xmin, xmax, ymin, ymax);

	fprintf(pipe, "set terminal png truecolor crop enhanced size 1024, 768\n");
	fprintf(pipe, "set output '%s.png'\n", argv[1]);
	fprintf(pipe, "set lmargin 0; set rmargin 0; set bmargin 0; set tmargin 0\n");

	fprintf(pipe, "set palette defined (0 \"#0000ff\", 1 \"red\", 2 \"#ffff88\", 3 \"white\")\n");
	fprintf(pipe, "unset surface; set view map; set pm3d;\n# unset colorbox\n");
	fprintf(pipe, "set zeroaxis; set border 0; set xtics axis nomirror; set ytics axis nomirror;\n");
	fprintf(pipe, " set tics scale 0.5; set format xy \"\"\n");

	fprintf(pipe, "splot \"-\"\n");


*/
	int total = int((xmax-xmin)/precision + 1)*int((ymax-ymin)/precision + 1);
	int now=0;
	//cerr << total << endl;

	for( double x=xmin; x<=xmax; x+= precision)
	{
		for( double y=ymin; y<=ymax; y+= precision)
		{
			if (x*x+y*y > 1)
			{
				double t= sqrt(Temperature(S, x, y, precision));
				printf("%lf\t%lf\t%lf\n", x, y, t);
			}
			else
			{
				printf("%lf\t%lf\t1\n", x, y);
			}
			now++;
			cerr << (now*100)/total << "%\r" << flush;
		}
		printf("\n");
	}

	return 0;
}

