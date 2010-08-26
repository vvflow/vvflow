#include "iostream"
#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include "math.h"
#ifndef M_PI_2
	#define M_PI_2 1.570796327
#endif
using namespace std;

#define CheckError if (!err) {if (feof(fin)) cout << "Reached unexpected end of file\n"; else cout << "Unknown error\n"; return -1; }

void NOK(double *xaxis, double *yaxis, double **data, int istart, int istop, int jstart, int jstop, double &a, double &b, double &c)
{
	//физический смысл этих переменных расписан в мануале
	double Sxx=0, Sxy=0, Sx=0, Szx=0;
	double        Syy=0, Sy=0, Szy=0;
	double               N =0, Sz =0;
	double xtmp, ytmp, ztmp; //временные переменные

	N = (istop-istart)*(jstop-jstart);
	for (int i=istart; i<istop; i++)
	{
		xtmp = xaxis[i];
		for (int j=jstart; j<jstop; j++)
		{
			ytmp = yaxis[j];
			ztmp = data[i][j];
			Sxx+= xtmp*xtmp;
			Sxy+= xtmp*ytmp;
			Syy+= ytmp*ytmp;
			Szx+= ztmp*xtmp;
			Szy+= ztmp*ytmp;
			Sx+= xtmp;
			Sy+= ytmp;
			Sz+= ztmp;
		}
	}

	//формулы ужасны. описание в мануале
	double A, B, D, E, F;
	A = (Sz*Sxx - Szx*Sx);
	B = (Syy*Sxx - Sxy*Sxy);
	D = (Szy*Sxx - Szx*Sxy);
	E = (Sy*Sxx - Sxy*Sx);
	F = (N*Sxx - Sx*Sx);
	c = (A*B - D*E) / (F*B - E*E);
	b = (D - c*E) / B;
	a = (Szx - b*Sxy - c*Sx) / Sxx;
}

int bound(int x, int xmin, int xmax)
{
	if (x<xmin) return xmin;
	else if (x>xmax) return xmax;
	else return x;
}

int main(int argc, char** argv)
{
	//заводим переменные

	int xgrid, ygrid, xspan, yspan;
	char* filename;

	if (argc != 6)
	{
		cout << "Enter input filename: ";
		filename = new char[64];
		scanf("%s", filename);
		cout << "Enter X grid size: ";
		cin >> xgrid;
		cout << "Enter Y grid size: ";
		cin >> ygrid;
		cout << "Enter X span: ";
		cin >> xspan;
		cout << "Enter Y span: ";
		cin >> yspan;
	} else 
	{
		filename = argv[1];
		sscanf(argv[2], "%d", &xgrid);
		sscanf(argv[3], "%d", &ygrid);
		sscanf(argv[4], "%d", &xspan);
		sscanf(argv[5], "%d", &yspan);
	}

	double **inputdata;
	double **outputdata;
	inputdata = (double**)malloc(xgrid*sizeof(double*));
	outputdata = (double**)malloc(xgrid*sizeof(double*));
	for (int i=0; i<xgrid; i++) 
	{
		inputdata[i] = (double*)malloc(ygrid*sizeof(double));
		outputdata[i] = (double*)malloc(ygrid*sizeof(double));
		for (int j=0; j<ygrid; j++) outputdata[i][j] = 0;
	}

	double xaxis[xgrid];
	double yaxis[ygrid];
	double xstep, ystep;
	char string[32000];
	void* err;

	//открываем файл для чтения
	FILE *fin, *fout;
	if (!strcmp(filename, "-"))
	{
		fin = stdin;
		fout = stdout;
	} else
	{
		fin = fopen(filename, "r");
		char output[strlen(filename)+10];
		sprintf(output, "%s.f3d", filename);
		fout = fopen(output, "w");
	}
	if (!fin) { cout << "Error opening input file." << endl; return -1;}
	if (!fout) { cout << "Error opening output file." << endl; return -1;}

	//Инициализация осей и массива данных
	for (int i=0; i<xgrid; i++)
	{
		for (int j=0; j<ygrid; j++)
		{
			//err = fgets(string, 32000, fin);
			//CheckError;
			fscanf(fin, "%lf %lf %lf", xaxis+i, yaxis+j, inputdata[i]+j);
		}
	}
	xstep = xaxis[1] - xaxis[0];
	ystep = yaxis[1] - yaxis[0];
	//cout << "grid " << xstep << " " << ystep <<" " << xgrid << " " << ygrid << " " << xspan << " " << yspan << endl;

	//считаем ур-е плоскости методом наименьших квадратов
	double a,b,c;
	double xscale = M_PI_2/(xspan*xstep);
	double yscale = M_PI_2/(yspan*ystep);
	for (int i=0; i<xgrid; i+=xspan)
	{
		for (int j=0; j<ygrid; j+=yspan)
		{
			NOK(xaxis, yaxis, inputdata, bound(i-xspan, 0, xgrid), bound(i+xspan+1, 0, xgrid), bound(j-yspan, 0, ygrid), bound(j+yspan+1, 0, ygrid), a, b, c);
			//cout << "NOK " << a << " " << b << " " << c << " ij " << i << " " << j << endl;
			for (int i1=bound(i-xspan, 0, xgrid); i1<bound(i+xspan+1, 0, xgrid); i1++)
			{
				for (int j1=bound(j-yspan, 0, ygrid); j1<bound(j+yspan+1, 0, ygrid); j1++)
				{
					double xcos = cos((xaxis[i]-xaxis[i1])*xscale);
					double ycos = cos((yaxis[j]-yaxis[j1])*yscale);
					//cout << "dbg " << i << " " << j << " " << i1 << " " << j1 << " " << xcos << " " << ycos << endl; 
					outputdata[i1][j1] += (a*xaxis[i1] + b*yaxis[j1] + c) * xcos*xcos * ycos*ycos;
				}
			}
		}
	}

	//печатаем
	for (int i=0; i<xgrid; i++)
	{
		for (int j=0; j<ygrid; j++)
		{
			fprintf(fout, "%lf\t%lf\t%lf\n", xaxis[i], yaxis[j], outputdata[i][j]);
		}
		//fprintf(fout, "\n");
	}

	fclose(fin);
	fclose(fout);
	return 0;
}
