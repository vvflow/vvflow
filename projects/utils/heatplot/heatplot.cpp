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
#define expdef(x) fexp(x)
#define MARGIN 0.1
using namespace std;

void Epsilon(TList *List, double eps2min) //eps^2
{
	int i, lsize, j;
	TVortex *Vorti, *Vortj;
	double eps1;
	double drx, dry, drabs2;

	lsize = List->size;
	Vorti = List->Elements;
	if (lsize < 2) return;
	
	for ( i=0; i<lsize; i++ )
	{
		Vorti->vx = 1E10;
		Vortj = List->Elements;
		for ( j=0; j<lsize; j++ )
		{
			drx = Vortj->rx - Vorti->rx;
			dry = Vortj->ry - Vorti->ry;
			drabs2 = fabs(drx) + fabs(dry);
			if ( !drabs2 ) { Vortj++; continue; }
			if ( drabs2 <= Vorti->vx )
			{
				Vorti->vx = drabs2;
			}
			if ( drabs2 < eps2min)
			{
				Vorti->vx = eps2min;
			}
			Vortj++;
		}
		Vorti++;
	}
} 

void Epsilon(TList *List, double px, double py, double &res, double precision)
{
	int i, lsize;
	TVortex *Vort;
	double eps1, eps2=0;
	double drx, dry, drabs2;

	lsize = List->size;
	Vort = List->Elements;

	if (lsize < 2) res = precision;
	else
	{
		eps1 = 1E10; // = inf //Diffusive_DefaultEpsilon*Diffusive_DefaultEpsilon;
	
		for ( i=0; i<lsize; i++ )
		{
				drx = px - Vort->rx;
				dry = py - Vort->ry;
				drabs2 = drx*drx + dry*dry;
				if ( !drabs2 ) { Vort++; continue; }
				if ( drabs2 <= eps1 )
				{
					eps2 = eps1;
					eps1 = drabs2;
				} else
				if ( drabs2 < eps2 )
				{
					eps2 = drabs2;
				}
				Vort++;
		}
		res = sqrt(eps2)*2;
	}
	//cout << px << "\t" << py << "\t" << eps1 << "\t" << eps2 << endl;
	if (res < precision) res = precision;
}

TVortex* Nearest(TList *List, double px, double py)
{
	int i, lsize;
	TVortex *Vort;
	TVortex *res;
	double resr;
	double drx, dry, drabs2;

	lsize = List->size;
	Vort = List->Elements;

	if (lsize < 2) res = NULL;
	else
	{
		resr = 1E10; // = inf //Diffusive_DefaultEpsilon*Diffusive_DefaultEpsilon;
	
		for ( i=0; i<lsize; i++ )
		{
				drx = px - Vort->rx;
				dry = py - Vort->ry;
				drabs2 = drx*drx + dry*dry;
				if ( !drabs2 ) { Vort++; continue; }
				if ( drabs2 <= resr )
				{
					res = Vort;
					resr = drabs2;
				}
				Vort++;
		}
	}
	//cout << px << "\t" << py << "\t" << eps1 << "\t" << eps2 << endl;
	return res;
}

double Temperature(Space* S, double px, double py, double precision)
{
	double res=0;
	double drx, dry, drabs2, drx2, dry2;

	TList *vlist = S->HeatList;
	int lsize = S->HeatList->size;
	TVortex *Vort = S->HeatList->Elements;
	double g = Vort->g;
	TVortex* nearest = Nearest(vlist, px, py);
	double eps2 = nearest->vx;//vx isnt actually a vx, its eps stored in vx.

	// check if there r no particles beside	
	drx = px - nearest->rx;
	dry = py - nearest->ry;
	drx2 = drx*drx;
	dry2 = dry*dry;
	drabs2 = drx2 + dry2;
	if ( sqrt(drabs2)/eps2 > 10 ) return 0;
	//end of check

	double multiplier = M_PI*eps2/2;

	double v_1_eps2 = 1/eps2;

	for (int i=0; i<lsize; i++)
	{
		

		drx = px - Vort->rx;
		dry = py - Vort->ry;
		drx2 = drx*drx;
		dry2 = dry*dry;
		drabs2 = drx2 + dry2;

		double exparg = -drabs2*v_1_eps2; //vx isnt actually a vx, its eps stored in vx.
		if (exparg > -8) res+= expdef(exparg);
		Vort++;
	}

	//	cout << res << "\t";

	double h = sqrt(px*px+py*py)-1;
	double erfarg = h*sqrt(v_1_eps2);
	if (erfarg<3)
		res+= multiplier*(1-erf(erfarg));

		//cout << multiplier*(1-erf(erfarg)) << "\t" << v_1_eps2*M_1_PI << endl;

	res*= g*v_1_eps2*M_1_PI;
	return res;
}

void CutField(Space *S, double xmin, double xmax, double ymin, double ymax)
{
	double dx=xmax-xmin, dy=ymax-ymin;
	double xmin1=xmin-(dx*MARGIN), xmax1=xmax+(dx*MARGIN), ymin1=ymin-(dy*MARGIN), ymax1=ymax+(dy*MARGIN);
	
	if ( !S->HeatList ) return;

	TList *list = S->HeatList;
	int lsize = list->size;
	TVortex *Obj = list->Elements;
	for ( int i=0; i<lsize; i++)
	{
		if ( (Obj->rx < xmin1) || (Obj->rx > xmax1) || (Obj->ry < ymin1) || (Obj->ry > ymax1) )
		{
			list->Remove(Obj);
		}
		else { Obj++; }
	}
}

int main(int argc, char *argv[])
{
	if ( argc != 5) { cout << "Error! Please use: \nheatplot filename xmin xmax precision\n"; return -1; }

	Space *S = new Space(0, 0, 1);
	S->LoadHeatFromFile(argv[1]);

	double xmin, xmax, ymin, ymax, precision;
	sscanf(argv[2], "%lf", &xmin);
	sscanf(argv[3], "%lf", &xmax);
	sscanf(argv[4], "%lf", &precision); 
	ymax = (xmax - xmin) / 949*729/2; ymin = -ymax;
	CutField(S, xmin, xmax, ymin, ymax);
	Epsilon(S->HeatList, precision);


	int total= floor((xmax-xmin)/precision+1)*floor((ymax-ymin)/precision+1);
	int progress = 0, last=0, percent;

	//GNUPLOT part

	FILE *pipe = popen("gnuplot","w");
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



	for( double x=xmin; x<=xmax; x+= precision)
	{
		for( double y=ymin; y<=ymax; y+= precision)
		{
			if (x*x+y*y > 1)
			{
				double t= Temperature(S, x, y, precision);
				if ( t>1) t=1;
				fprintf(pipe, "%lf\t%lf\t%lf\n", x, y, t);
//				fout << x << "\t" << y << "\t" << Color(S, x, y, precision) << endl;
			}
			else
			{
				fprintf(pipe, "%lf\t%lf\t1\n", x, y);
//				fout << x << "\t" << y << "\t" << 0 << endl;
			}
			progress++;
			percent = 100*progress/total;
			if (percent > last) { printf("%02d\r", percent); flush(cout); last = percent; }
		}
		fprintf(pipe, "\n");
	}

	pclose(pipe);
	return 0;
}

