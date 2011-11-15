#include "stdio.h"
#include "iostream"
#include "malloc.h"

#include "core.h"
using namespace std;

class dot
{
	public:
	dot() {x=y=z=0;}
	dot(double _x, double _y, double _z) {x=_x; y=_y; z=_z;}
	friend bool operator == (dot &p1, dot &p2) { return (p1.x==p2.x)&&(p1.y==p2.y); }

	double x, y, z;
};

class seg
{
	public:
	seg() {}
	seg(dot _p1, dot _p2) {p1=_p1; p2=_p2;}
	dot p1, p2;
};

//double max(double a1, double a2) {return (a1>a2)?a1:a2;}
//double min(double a1, double a2) {return (a1<a2)?a1:a2;}
bool between(double c, double a1, double a2, double a3)
{
	if (!a1 || !a2 || !a3) return false;
	return (c<max(a1, max(a2, a3)))&&(c>min(a1, min(a2, a3)));
}

bool between(double c, dot p1, dot p2, dot& res)
{
	if ((c<=max(p1.z, p2.z))&&(c>=min(p1.z, p2.z)))
	{
		double tmp = (c-p1.z)/(p2.z-p1.z);
		res.x = p1.x + (p2.x-p1.x)*tmp;
		res.y = p1.y + (p2.y-p1.y)*tmp;
		return true;
	}
	return false;
}

int main(int argc, char **argv)
{
	if ( argc != 8)\
	{
		fprintf(stderr, "Error! Please use: \nisoline_exe file.map const prec xmin xmax ymin ymax\n");
		return -1;
	}

	double C = atof(argv[2]);
	double xmin, xmax, ymin, ymax, prec;
	{ int i=3;
		xmin = atof(argv[++i]);
		xmax = atof(argv[++i]);
		ymin = atof(argv[++i]);
		ymax = atof(argv[++i]);
		prec = atof(argv[3]); }
	int imax = (xmax-xmin)/prec + 1;
	int jmax = (ymax-ymin)/prec + 1;

	FILE* fin = fopen(argv[1], "r");
	if (!fin) { perror("Error"); return 0; }

	double xaxis[imax], yaxis[jmax], data[imax][jmax];

	double x, y, z; int k=0;
	while ( fscanf(fin, "%lf %lf %lf", &x, &y, &z)==3 )
	{
		//realloc(xaxis, imax*sizeof(double));
		xaxis[k/jmax] = x;
		yaxis[k%jmax] = y;
		data[k/jmax][k%jmax] = z;
		k++;
	}

	vector<seg> *seglist = new vector<seg>();
	for (int i=0; i<(imax-1); i++)
	{
		for (int j=0; j<(jmax-1); j++)
		{
			if (between(C, data[i][j], data[i][j+1], data[i+1][j+1]))
			{
				dot p1(xaxis[i], yaxis[j], data[i][j]);
				dot p2(xaxis[i], yaxis[j+1], data[i][j+1]);
				dot p3(xaxis[i+1], yaxis[j+1], data[i+1][j+1]);
				dot pn[2];
				int i=0;
				if (between(C, p1, p2, pn[i])) i++;
				if (between(C, p2, p3, pn[i])) i++;
				if (between(C, p3, p1, pn[i])) i++;

				seglist->push_back(seg(pn[0], pn[1]));
			}
			if (between(C, data[i][j], data[i+1][j], data[i+1][j+1]))
			{
				dot p1(xaxis[i], yaxis[j], data[i][j]);
				dot p2(xaxis[i+1], yaxis[j], data[i+1][j]);
				dot p3(xaxis[i+1], yaxis[j+1], data[i+1][j+1]);
				dot pn[2];
				int i=0;
				if (between(C, p1, p2, pn[i])) i++;
				if (between(C, p2, p3, pn[i])) i++;
				if (between(C, p3, p1, pn[i])) i++;

				seglist->push_back(seg(pn[0], pn[1]));
			}
		}
	}

	if (!seglist->size_safe()) {printf("0 0\n0 0\n"); return 0;}
	dot end = seglist->begin()->p1;
	#define print printf("%lg\t%lg\n", end.x, end.y)
	print;
	while (seglist->size())
	{
		bool updated = false;
		//FIXME this loop doesnt work with -O3
		const_for (seglist, lseg)
		{
			if (lseg->p1 == end)
			{
				end = lseg->p2;
				seglist->erase(lseg);
				updated = true;
				break;
			} else
			if (lseg->p2 == end)
			{
				end = lseg->p1;
				seglist->erase(lseg);
				updated = true;
				break;
			}
		}

		if (!updated)
		{
			end = seglist->begin()->p1;
			printf("\n");
		}

		print;
	}
	#undef print

	return 0;
}
