#include "iostream"
#include "fstream"
#include <vector>
#include <algorithm>
#include <string.h>

#include "math.h"

using namespace std;

struct Point
{
	double x, y;
}; typedef struct Point TPoint;

bool operator<(const TPoint& a, const TPoint& b)
{
	return (a.x < b.x);
}

void LeastSquares(vector<TPoint> &Array, double xmin, double xmax, double &k, double &b)
{
	double Sxx=0, Sx=0, Sxy=0;
	double        N =0, Sy =0;
	TPoint tmp;

	for (int i=0; i<Array.size(); i++)
	{
		tmp= Array[i];
		if ((tmp.x >=  xmin) && (tmp.x <= xmax))
		{
			Sxx+= tmp.x * tmp.x;
			Sxy+= tmp.x * tmp.y;
			Sx+= tmp.x;
			Sy+= tmp.y;
			N++;
		}
	}

	double bottom = (N*Sxx-Sx*Sx);
	if (fabs(bottom) < 1E-8) {k=0; b = (N ? Sy/N : 0); return;}
	k = (N*Sxy-Sx*Sy)/bottom;
	b = (Sy*Sxx-Sx*Sxy)/bottom;
}

int main(int argc, char** argv)
{
	/****** OPTIONS ******/
	char* filename;
	bool SteadyX = 0;
	double xmin, xmax, dx;
	int xspan;

	/****** SCANNING *****/
	if (argc != 3)
	{
		cout << "Enter input filename: ";
		filename = new char[256];
		scanf("%s", filename);
		cout << "Enter X span (No. of data points in a segment): ";
		cin >> xspan;
	} else 
	{
		filename = argv[1];
		sscanf(argv[2], "%d", &xspan);
	}

	/****** OPENING FILE *****/
	FILE *fin, *fout;
	if (!strcmp(filename, "-"))
	{
		fin = stdin;
		fout = stdout;
	} else
	{
		fin = fopen(filename, "r");
		char output[strlen(filename)+10];
		sprintf(output, "%s.f2d", filename);
		fout = fopen(output, "w");
	}
	if (!fin) { cout << "Error opening input file." << endl; return -1;}
	if (!fout) { cout << "Error opening output file." << endl; return -1;}


	/***** READING FILE *****/
	vector<TPoint> Array, Copy;
	TPoint a;

	char line[255], *err;
	while ( fgets(line, 255, fin) )
	{
		if (sscanf(line, "%lf\t%lf\n", &a.x, &a.y) == 2)
			Array.push_back(a);
	}
	fclose(fin);

	xmin = min_element(Array.begin(), Array.end())->x;
	xmax = max_element(Array.begin(), Array.end())->x;
	if (xspan)
		dx = (xmax-xmin)/double(Array.size()-1)*xspan;
	else 
		dx = (xmax-xmin)/double(Array.size()-1)*0.5;

	cout << xmin << " " << xmax << " " << dx << endl;

	/***** SPARE ARRAY *****/
	if (SteadyX)
	{

	} else
	{
		Copy = Array;
		for (int i=0; i<Array.size(); i++)
		{ Copy[i].y = 0; }
	}

	/***** MAIN *****/

	double PI_2dx = M_PI/2/dx;
	for (double x=xmin; x<xmax+dx; x+= dx)
	{
		double k, b;
		double xmin1=x-dx, xmax1=x+dx;
		LeastSquares(Array, xmin1, xmax1, k, b);

		for (int i=0; i<Copy.size(); i++)
		{
			TPoint &tmp= Copy[i];
			if ((tmp.x >  xmin1) && (tmp.x < xmax1))
			{
				double costmp = cos((tmp.x - x)*PI_2dx);
				tmp.y += (k*tmp.x+b)*costmp*costmp;
			}
		}
	}

	/***** OUTPUT *****/

	for (int i=0; i<Copy.size(); i++)
	{
		fprintf(fout, "%f\t%f\n", Copy[i].x, Copy[i].y);
//		fout << Copy[i].x << "\t" << Copy[i].y << endl;
	}
	return 0;
}

