#include "stdio.h"
#include "stdlib.h"
#include "iostream"
#include "malloc.h"
#include <time.h>

//#include "core.h"
using namespace std;

#define Panic {cerr << "Bad file!\n"; return -3;}

struct vec { double x, y; };
typedef struct vec Vec;

double max(double a1, double a2) {return (a1>a2)?a1:a2;}
double min(double a1, double a2) {return (a1<a2)?a1:a2;}

void ProcessRect(double x1, double x2, double y1, double y2, double z11,
				double z12, double z21, double z22, double C)
{
	Vec vecs[4];
	int N=0;

	if ( (z11<C && C<z12) || (z12<C && C<z11) )
	{
		vecs[N].x = x1;
		vecs[N].y = y1 + (C-z11)*(y2-y1)/(z12-z11);
		N++;
	}

	if ( (z22<C && C<z21) || (z21<C && C<z22) )
	{
		vecs[N].x = x2;
		vecs[N].y = y1 + (C-z21)*(y2-y1)/(z22-z21);
		N++;
	}

	if ( (z12<C && C<z22) || (z22<C && C<z12) )
	{
		vecs[N].x = x1 + (C-z12)*(x2-x1)/(z22-z12);
		vecs[N].y = y2;
		N++;
	}

	if ( (z21<C && C<z11) || (z11<C && C<z21) )
	{
		vecs[N].x = x1 + (C-z11)*(x2-x1)/(z21-z11);
		vecs[N].y = y1;
		N++;
	}

	if (N==1 || N==3) {cerr << "Fanatactic!\n"; exit(-4); }
	if (N==2)
	{
		//#pragma omp critical(print)
		{
			printf("%lg\t%lg\n", vecs[0].x, vecs[0].y);
			printf("%lg\t%lg\n", vecs[1].x, vecs[1].y);
			printf("\n");
		}
	}
	if (N==4)
	{
		//#pragma omp critical(print)
		{
			printf("%lg\t%lg\n", vecs[2].x, vecs[2].y);
			printf("%lg\t%lg\n", vecs[3].x, vecs[3].y);
			printf("\n");
		}
	}
}

int main(int argc, char **argv)
{
	time_t begining = clock();
	if ( argc < 3 )
	{
		fprintf(stderr, "Error! Please use: \nisoline_exe file.map const\n");
		return -1;
	}

	double C = atof(argv[2]);
	FILE* fin = fopen(argv[1], "r");
	if (!fin) { perror("Error"); return 0; }

	double *xaxis, *yaxis, *data;
	xaxis = (double*) malloc(sizeof(double));
	yaxis = (double*) malloc(sizeof(double));
	data = (double*) malloc(sizeof(double));

	double x, y, z;
	int imax=0, jmax=0;

	if (fscanf(fin, "%lf %lf %lf", &x, &y, &z) != 3) Panic;
	xaxis[0] = x;
	yaxis[0] = y;
	data[0] = z;

	while(true)
	{
		if (fscanf(fin, "%lf %lf %lf", &x, &y, &z) != 3) Panic;
		if (xaxis[0] != x) break;

		jmax++;
		yaxis = (double*)realloc(yaxis, (jmax+1)*sizeof(double));
		data = (double*)realloc(data, (jmax+1)*sizeof(double));
		yaxis[jmax] = y;
		data[jmax] = z;
	}

	bool go = true;
	while(go)
	{
		for (int j=0; j<=jmax; j++)
		{
			if (yaxis[j] != y) Panic;
			if (!j)
			{
				imax++;
				xaxis = (double*) realloc(xaxis, (imax+1)*sizeof(double));
				data = (double*) realloc(data, (jmax+1)*(imax+1)*sizeof(double));
				xaxis[imax] = x;
			}

			data[(jmax+1)*imax + j] = z;

			if (fscanf(fin, "%lf %lf %lf", &x, &y, &z) != 3)
				if (j==jmax) {go=false;}
				else Panic;
		}
	}

	/*for(int i=0; i<=imax; i++)
	for(int j=0; j<=jmax; j++)
	{
		cout << xaxis[i] << "\t" << yaxis[j] << "\t" << data[i*(jmax+1)+j] << endl;
	}*/

	//return 0;
	double steptime = double(clock()-begining)/CLOCKS_PER_SEC;

	begining = clock();
//	seglist = new vector<seg>();
	for (int c=2; c<argc; c++)
	{
		double C = atof(argv[c]);
		for (int i=0; i<=(imax-1); i++)
		{
			//#pragma omp parallel for
			for (int j=0; j<=(jmax-1); j++)
			{
				ProcessRect(xaxis[i], xaxis[i+1], yaxis[j], yaxis[j+1],
							data[i*(jmax+1)+j], data[i*(jmax+1)+j+1],
							data[(i+1)*(jmax+1)+j], data[(i+1)*(jmax+1)+j+1], C);
			}
		}
	}

	printf("0\t0\n0\t0\n");

/*
	cerr << seglist->size_safe() << endl;
	if (!seglist->size_safe()) {printf("0 0\n0 0\n"); return 0;}
	Vec end = seglist->begin()->v1;
	#define print printf("%lg\t%lg\n", end.x, end.y)
	print;
	while (seglist->size())
	{
		bool updated = false;
		//FIXME this loop doesnt work with -O3
		const_for (seglist, lseg)
		{
			if (lseg->v1.x == end.x && lseg->v1.y == end.y)
			{
				end = lseg->v2;
				seglist->erase(lseg);
				updated = true;
				break;
			} else
			if (lseg->v2.x == end.x && lseg->v2.y == end.y)
			{
				end = lseg->v1;
				seglist->erase(lseg);
				updated = true;
				break;
			}
		}

		if (!updated)
		{
			end = seglist->begin()->v1;
			printf("\n");
		}

		print;
	}
	#undef print
	*/
	steptime = double(clock()-begining)/CLOCKS_PER_SEC;
	//cerr << "Calc: " << steptime << endl;

	return 0;
}
