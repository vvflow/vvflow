#include "libVVHD/core.h"
#include "libVVHD/convectivefast.h"
#include "stdio.h"
#include "iostream"

using namespace std;

int main(int argc, char **argv)
{
	if ( argc < 10) { cout << "Error! Please use: \n\tvelgrid vortexfile bodyfile xmin xmax ymin ymax step infx infy\n"; return -1; }   
	Space *S = new Space(true, false);
	S->Body->LoadFromFile(argv[2]);
	S->LoadVorticityFromFile(argv[1]);
	InitTree(S, 10, S->Body->SurfaceLength()/S->Body->List->size*10);
	InitConvectiveFast(S, 5E-3);
	BuildTree(true, true, false);

	double xmin, xmax, ymin, ymax, step, infx, infy;
	sscanf(argv[3], "%lf", &xmin);
	sscanf(argv[4], "%lf", &xmax);
	sscanf(argv[5], "%lf", &ymin);
	sscanf(argv[6], "%lf", &ymax);
	sscanf(argv[7], "%lf", &step);
	sscanf(argv[8], "%lf", &infx);
	sscanf(argv[9], "%lf", &infy);

	for ( double x=xmin; x<=xmax; x+= step )
	{
		for ( double y=ymin; y<=ymax; y+= step )
		{
			if (S->Body->PointIsValid(x, y))
			{
				double ResX, ResY;
				SpeedSumFast(x, y, ResX, ResY);
				cout << x << " \t" << y << " \t" << ResX+infx << " \t" << ResY+infy << endl;
			} else
				cout << x << " \t" << y << " \t0 \t0\n";
		}
	}

	return 0;
}

