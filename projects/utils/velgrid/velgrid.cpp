#include "libVVHD/core.h"
#include "libVVHD/convective.h"
#include "libVVHD/utils.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include <time.h>

#define M_2PI 6.283185308

using namespace std;

int main(int argc, char **argv)
{
	if ( argc < 7) { cout << "Error! Please use: \n\tvelgrid filename xmin xmax ymin ymax step\n"; return -1; }   
	Space *S = new Space(1, 0, 0, NULL, NULL, NULL);
	S->LoadVorticityFromFile(argv[1]);
	cout << "Load done\n";
	InitConvective(S, 5E-3);
	double xmin, xmax, ymin, ymax, step;
	double A, ch=1, t=0;
	sscanf(argv[2], "%lf", &xmin);
	sscanf(argv[3], "%lf", &xmax);
	sscanf(argv[4], "%lf", &ymin);
	sscanf(argv[5], "%lf", &ymax);
	sscanf(argv[6], "%lf", &step);
/*	if ( argc > 7 )
	{
		sscanf(argv[7], "%lf", &A);
		if ( argc == 10 )
		{
			sscanf(argv[8], "%lf", &ch);
			sscanf(argv[9], "%lf", &t);
		} else { ch=0; t=0; }
		TVortex BodyRotation; InitVortex(BodyRotation, 0, 0, A*M_2PI*cos(M_2PI*ch*t));
		S->VortexList->Copy(&BodyRotation);
	}
*/
	double G =0;
	TList<TObject> *list = S->VortexList;
	int lsize = list->size;
	TVortex *Obj = list->First;
	for ( int i=0; i<lsize; i++)
	{
		G += Obj->g;
		Obj++;
	}
	TVortex BodyRotation; InitVortex(BodyRotation, 0, 0, -G);
	S->VortexList->Copy(&BodyRotation);


	fstream fout;
	char fname[256];
	sprintf(fname, "%s.dat", argv[1]);
	fout.open(fname, ios::out);
	fout << "TITLE = \"" << argv[1] << "\"\n";
	fout << "VARIABLES = \"x\", \"y\", \"u\", \"v\"\n";
	fout << "ZONE T=\"Body\", I=" << floor((ymax-ymin)/step) << ", J=" << floor((xmax-xmin)/step) << ", F=POINT\n";

	double n=0;
	double nmax1=1/(floor((xmax-xmin)/step)*floor((ymax-ymin)/step));
	cout.precision(2);
	for ( double x=xmin; x<xmax; x+= step )
	{
		for ( double y=ymin; y<ymax; y+= step )
		{
			//if ( (x*x+y*y) > 1 )
			//{
				double ResX, ResY;
				SpeedSum(S->VortexList, x, y, ResX, ResY);
				fout << x << " " << y << " " << ResX << " " << ResY << endl;
			//}
			//else
			//{ fout << x << " " << y << " 0 0" << endl; }

			if ( !((int)n%10000) ) 
			{
				printf(" %.1f %\r", n*nmax1);
				flush(cout);
			}
			n+=100;
		}
	}
	cout << " 100  % " << endl;
	fout.close();

	return 0;
}

