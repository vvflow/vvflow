#include "libVVHD/core.h"
#include "libVVHD/convective.h"
#include "libVVHD/utils.h"
#include "libVVHD/list.h"
#include "stdio.h"
#include "math.h"
#include "iostream"
#include "fstream"

#define M_2PI 6.283185308

using namespace std;


struct s_boundary
{
	double xmin, xmax, ymin, ymax, step;
}; typedef struct s_boundary boundary;

int Inside(boundary* bnd, double x, double y)
{
	return (x>=bnd->xmin)&&(x<=bnd->xmax)&&(y>=bnd->ymin)&&(y<=bnd->ymax);
}

double NearestTrack(TlList *StreamLines, double x, double y)
{
	double nearest=1E10;
	double dx, dy, dr2;
	TList** StreamLineL = (TList**)StreamLines->Elements;
	TList* StreamLine;
	for (int i=0; i<StreamLines->size; i++)
	{
		StreamLine = *StreamLineL;
		TVortex* Obj = StreamLine->Elements;
		for (int j=0; j<StreamLine->size; j++)
		{
			dx = Obj->rx-x;
			dy = Obj->ry-y;
			dr2 = dx*dx+dy*dy;
			if (dr2 < nearest) nearest = dr2; 
			Obj++;
		}
		StreamLineL++;
	}
	return sqrt(nearest);
}

double NearestTrack_faster(TlList *StreamLines, double x, double y)
{
	double nearest=1E10;
	double dx, dy, dr2;
	TList** StreamLineL = (TList**)StreamLines->Elements;
	TList* StreamLine;
	for (int i=0; i<StreamLines->size; i++)
	{
		StreamLine = *StreamLineL;
		TVortex* Obj = StreamLine->Elements;
		for (int j=0; j<StreamLine->size; j++)
		{
			dx = fabs(Obj->rx-x);
			dy = fabs(Obj->ry-y);
			dr2 = dx+dy;
			if (dr2 < nearest) nearest = dr2; 
			Obj++;
		}
		StreamLineL++;
	}
	return nearest;
}

double NearestDot(TList *StreamLine, double x, double y)
{
	double nearest=1E10;
	double dx, dy, dr2;

	TVortex* Obj = StreamLine->Elements;
	for (int j=0; j<StreamLine->size; j++)
	{
		dx = Obj->rx-x;
		dy = Obj->ry-y;
		dr2 = dx*dx+dy*dy;
		if (dr2 < nearest) nearest = dr2; 
		Obj++;
	}

	return sqrt(nearest);
}

double NearestDot_faster(TList *StreamLine, double x, double y)
{
	double nearest=1E10;
	double dx, dy, dr2;

	TVortex* Obj = StreamLine->Elements;
	for (int j=0; j<StreamLine->size; j++)
	{
		dx = fabs(Obj->rx-x);
		dy = fabs(Obj->ry-y);
		dr2 = dx+dy;
		if (dr2 < nearest) nearest = dr2; 
		Obj++;
	}

	return nearest;
}

int CalculateStreamLine(Space *S, boundary* bnd, TList *StreamLine, double sx, double sy, double dt)
{
	TVortex dot;
	int interrupt = 0;
	double ResX1, ResY1, ResX2, ResY2;
	double vabs, drabs;
	int total = 1;

	InitVortex(dot, sx, sy, 0);
	StreamLine->Add(dot);
	while (Inside(bnd, dot.rx, dot.ry) && !interrupt)
	{
		SpeedSum(S->VortexList, dot.rx, dot.ry, ResX1, ResY1);
		SpeedSum(S->VortexList, dot.rx+(ResX1+1)*dt, dot.ry+(ResY1)*dt, ResX2, ResY2);
		dot.vx = (ResX1+ResX2+2)*0.5;
		dot.vy = (ResY1+ResY2)*0.5;
		vabs = fabs(dot.vx)+fabs(dot.vy);
		drabs = fabs(dot.rx-sx)+fabs(dot.ry-sy);
		if (vabs < 0.01) interrupt = 1;
		if ( (dot.g > 100) && (drabs < dt*0.5)) interrupt = 1;

		dot.rx+= dot.vx*dt;
		dot.ry+= dot.vy*dt;
		dot.g++;
		StreamLine->Add(dot);
	}
	total+= dot.g;
	/*
	InitVortex(dot, sx, sy, 0);
	while (Inside(bnd, dot.rx, dot.ry) && !interrupt)
	{
		SpeedSum(S->VortexList, dot.rx, dot.ry, ResX1, ResY1);
		SpeedSum(S->VortexList, dot.rx-(ResX1+1)*dt, dot.ry-ResY1*dt, ResX2, ResY2);
		dot.vx = (ResX1+ResX2+2)*0.5;
		dot.vy = (ResY1+ResY2)*0.5;
		if (fabs(dot.vx)+fabs(dot.vy)<0.01) interrupt = 1;
		dot.rx-= dot.vx*dt;
		dot.ry-= dot.vy*dt;
		dot.g--;
		StreamLine->Add(dot);
	}
	total-= dot.g;
*/
	return total;
}

int main(int argc, char **argv)
{
	if ( argc != 8) { cout << "Error! Please use: \n\tstreamlines filename xmin xmax ymin ymax step lines_density\n"; return -1; }   
	Space *S = new Space(1, 0, 0, NULL, NULL, NULL);
	S->LoadVorticityFromFile(argv[1]);
	InitConvective(S, 1E-4);
	TlList *StreamLines = new TlList;

	boundary bnd;
	double density;
	sscanf(argv[2], "%lf", &bnd.xmin);
	sscanf(argv[3], "%lf", &bnd.xmax);
	sscanf(argv[4], "%lf", &bnd.ymin);
	sscanf(argv[5], "%lf", &bnd.ymax);
	sscanf(argv[6], "%lf", &bnd.step);
	sscanf(argv[7], "%lf", &density);

	double G =0;
	TList *list = S->VortexList;
	int lsize = list->size;
	TVortex *Obj = list->Elements;
	for ( int i=0; i<lsize; i++)
	{
		G += Obj->g;
		Obj++;
	}
	TVortex BodyRotation; InitVortex(BodyRotation, 0, 0, -G);
	S->VortexList->Copy(&BodyRotation);

	for ( double x=bnd.xmin; x<=bnd.xmax; x+= bnd.step )
	{
		for ( double y=bnd.ymin; y<=bnd.ymax; y+= bnd.step )
		{
			if ( ((x*x+y*y)>1) && (NearestTrack_faster(StreamLines, x, y)>density) )
			{
				TList *tmpStreamLine = new TList;
				cout << x << " " << y << " " << CalculateStreamLine(S, &bnd, tmpStreamLine, x, y, bnd.step) << endl;
				StreamLines->Add(tmpStreamLine);
			}
		}
	}

	fstream fout;
	char fname[256];
	sprintf(fname, "%s.streamlines", argv[1]);
	fout.open(fname, ios::out);
	
//	#define fout cout

	{ //print
		TList** StreamLineL = (TList**)StreamLines->Elements;
		TList* StreamLine;
		for (int i=0; i<StreamLines->size; i++)
		{
			StreamLine = *StreamLineL;
			TVortex* Obj;

			Obj = StreamLine->Elements;
			for (int j=0; j<StreamLine->size; j++)
			{
				fout << Obj->rx << "\t" << Obj->ry << endl; 
				Obj++;
			} fout << endl;

/*			Obj = StreamLine->Elements;
			fout << "x\t";
			for (int j=0; j<StreamLine->size; j++)
			{
				fout << Obj->rx << "\t"; 
				Obj++;
			} fout << endl;

			Obj = StreamLine->Elements;
			fout << "y\t";
			for (int j=0; j<StreamLine->size; j++)
			{
				fout << Obj->ry << "\t"; 
				Obj++;
			} fout << endl;

			Obj = StreamLine->Elements;
			fout << "n\t";
			for (int j=0; j<StreamLine->size; j++)
			{
				fout << Obj->g << "\t"; 
				Obj++;
			} fout << endl;
*/
			StreamLineL++;
		}
	}

	fout.close();

	return 0;
}

