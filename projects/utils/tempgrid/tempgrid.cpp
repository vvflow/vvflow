#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>

#define M_2PI 6.283185308
using namespace std;

int CountSectors(char *filename, double *array[], double deltar, int countr, int countfi)
{
	FILE * fin;
	double x, y, c;
	double r;
	double fi, deltafi= 2*M_PI/countfi;
	int i,j;

	fin = fopen(filename,"r");
	while (!feof(fin))
	{
		fscanf(fin, "%lf %lf %lf\n", &x, &y, &c);
		r = sqrt(x*x+y*y);
		fi= atan2(y,x)+M_PI;
		i = floor(fi/deltafi);
		j = floor((r-1)/deltar);
		if(j>=countr) continue;
		array[i][j] +=c;
	} 
	fclose(fin);
	return 0;
}

int main(int argc, char *argv[])
{
	if ( argc != 5) { cout << "Error! Please use: \n\ttempgrid filename dr rcount ficount\n"; return -1; }
	double deltar;
	int countr, countfi;

	sscanf(argv[2], "%lf", &deltar);
	sscanf(argv[3], "%d", &countr);
	sscanf(argv[4], "%d", &countfi);

	double sectors[countfi][countr];
	double square[countfi][countr];
	double deltafi= M_2PI/countfi;

	fstream fout;
	char fname[256];
	sprintf(fname, "%s.dat", argv[1]);
	fout.open(fname, ios::out);
	fout << "TITLE = \"" << argv[1] << "\"\n";
	fout << "VARIABLES = \"x\", \"y\", \"p\"\n";
	fout << "ZONE T=\"Temperature\", I=" << countr << ", J=" << countfi << ", F=POINT\n";


	int i,j;
	//i -fi; j - r;
	for(i=0; i<countfi; i++)
	{
		for(j=0; j<countr; j++)
		{
			sectors[i][j] = 0;
			square[i][j] = ( (1+(j+1)*deltar)*(1+(j+1)*deltar) - (1+j*deltar)*(1+j*deltar) )*M_PI/countfi;		}
	}

	FILE * fin;
	double x, y;
	double r;
	double fi;

	fin = fopen(argv[1],"r");
	while (!feof(fin))
	{
		fscanf(fin, "%lf %lf\n", &x, &y);
		r = sqrt(x*x+y*y);
		fi= atan2(y,x)+M_PI;
		i = floor(fi/deltafi);
		j = floor((r-1)/deltar);
		if(j>=countr) continue;
		sectors[i][j]++;
	} 
	fclose(fin);
//	CountSectors(argv[1],sectors, deltar, countr, countfi);

	for(i=0; i<countfi; i++)
	{
		for(j=0; j<countr; j++)
		{
			fout << (1+(j+0.5)*deltar)*cos(deltafi*(i+0.5)-M_PI) << "\t" 
				 << (1+(j+0.5)*deltar)*sin(deltafi*(i+0.5)-M_PI) << "\t" 
				 << sectors[i][j]/square[i][j] << endl;
			
		}
		//cout << endl;
	}
	fout.close();

	return 0;
}
















/*#include "libVVHD/core.h"
#include "libVVHD/utils.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include <time.h>
#include "math.h"

#define M_2PI 6.283185308

using namespace std;

double Temperature(double x, double y, Space* S, double relax)
{
	if (!S->HeatList) return 0;
	double Res=0;
	double relax1 = -1/relax;
	double drx, dry, drabs;

	TVortex *obj = S->HeatList->Elements;
	int lsize = S->HeatList->size;
	for ( int i=0; i<lsize; i++ )
	{
		drx = obj->rx - x;
		dry = obj->ry - y;
		drabs = sqrt(drx*drx + dry*dry)*relax1;
		if (drabs > -15)
			Res+= obj->g*exp(drabs);
	}
	return Res;
}

int main(int argc, char **argv)
{
	if ( argc != 5) { cout << "Error! Please use: \n\ttempgrid filename rmax rstep sectorscount\n"; return -1; }   
	Space *S = new Space(0, 0, 1E6);
	S->LoadHeatFromStupidFile(argv[1]);
	double rmax, rstep, dfi;
	int sectcount;
	sscanf(argv[2], "%lf", &rmax);
	sscanf(argv[3], "%lf", &rstep);
	sscanf(argv[4], "%d", &sectcount);
	dfi = M_2PI/sectcount;
	fstream fout;
	char fname[256];
	sprintf(fname, "%s.dat", argv[1]);
	fout.open(fname, ios::out);
	fout << "TITLE = \"" << argv[1] << "\"\n";
	fout << "VARIABLES = \"x\", \"y\", \"p\"\n";
	fout << "ZONE T=\"Temperature\", I=" << sectcount << ", J=" << floor((rmax-1)/rstep) << ", F=POINT\n";

	for (double x=-3; x<=3; x+=0.1)
	{
		for (double y=-3; y<=3; y+=0.1)
		{
			if (x*x+y*y < 1) 
				fout << x << "\t" << y << "\t" << 0 << endl;
			else
				fout << x << "\t" << y << "\t" << Temperature(x, y, S, 1) << endl;
		}
	}



	double grid[(int)floor((rmax-1)/rstep)][sectcount];
	double squares[(int)floor((rmax-1)/rstep)][sectcount];

	int rnmax = floor((rmax-1)/rstep);
	double multiplier = dfi*0.5;
	double r1, r2;
	for ( int rn=0; rn<=rnmax; rn++ )
	{
		for ( int fin=0; fin<=sectcount; fin++ )
		{
			grid[rn][fin] = 0;
			r1 = rn*rstep+1;
			r2 = r1 + rstep;
			squares[rn][fin] = ( r2*r2 - r1*r1 )*multiplier;
		}
	}

	int lsize = S->HeatList->size;
	TVortex* Heat = S->HeatList->Elements;
	double r, fi;
	int rn, fin;
	for ( int i=0; i<lsize; i++ )
	{
		r = Heat->rx*Heat->rx + Heat->ry*Heat->ry;
		fi = atan2(Heat->ry, Heat->rx) + M_PI;
		rn = floor((r-1)/rstep);
		fin = floor(fi/sectcount);
		if (r<rmax) { grid[rn][fin]+= Heat->g; }
		Heat++;
	}

	for ( int rn=0, r=1+0.5*rstep; rn<=rnmax; rn++, r+=rstep )
	{
		for ( int fin=0; fin<=sectcount; fin++ )
		{
			fout <<
					r * cos (dfi * (fin+0.5) - M_PI) << "\t" <<
					r * sin (dfi * (fin+0.5) - M_PI) << "\t" <<
					grid[rn][fin]/squares[rn][fin] << endl;
		}
	}



	fout.close();

	return 0;
}*/

