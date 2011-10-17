#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>

#include "core.h"
#include "convectivefast.h"

using namespace std;


struct s_boundary
{
	double xmin, xmax, ymin, ymax, step;
}; typedef struct s_boundary boundary;

int Inside(boundary* bnd, Vector p)
{
	return (p.rx>=bnd->xmin)&&(p.rx<=bnd->xmax)&&(p.ry>=bnd->ymin)&&(p.ry<=bnd->ymax);
}

inline double min(double a, double b)
{
	return (a<b)?a:b;
}

inline double abs2(Vector v)
{
	return v.rx*v.rx + v.ry*v.ry;
}

inline double abs_faster(Vector v)
{
	return fabs(v.rx) + fabs(v.ry);
}

template <bool Faster>
double NearestTrack(TList<TList<TObject>*> *StreamLines, Vector p)
{
	double nearest=1E10;
	TList<TObject>** StreamLineL = (TList<TObject>**)StreamLines->First;
	TList<TObject>* StreamLine;
	for (int i=0; i<StreamLines->size; i++)
	{
		StreamLine = *StreamLineL;
		TObject* Obj = StreamLine->First;
		for (int j=0; j<StreamLine->size; j++)
		{
			nearest = min(nearest, Faster?abs_faster(*Obj-p):abs2(*Obj-p) );
			Obj++;
		}
		StreamLineL++;
	}
	return Faster?nearest:sqrt(nearest);
}

template <bool Faster>
double NearestDot(TList<TObject> *StreamLine, Vector p)
{
	double nearest=1E10;

	TObject* Obj = StreamLine->First;
	for (int j=0; j<StreamLine->size; j++)
	{
		nearest = min(nearest, Faster?abs_faster(*Obj-p):abs2(*Obj-p) ); 
		Obj++;
	}

	return Faster?nearest:sqrt(nearest);
}

int CalculateStreamLine(Space *S, boundary* bnd, TList<TObject> *StreamLine, Vector s, double dt)
{
	TObject dot(s.rx, s.ry, 0);
	int interrupt = 0;
	double ResX1, ResY1, ResX2, ResY2;
	Vector Res1, Res2;
	double vabs, drabs;
	int total = 1;

	StreamLine->Add(dot);
	while (Inside(bnd, dot) && !interrupt)
	{
		Res1 = SpeedSumFast(dot);
		Res2 = SpeedSumFast(dot+Res1*dt);
		dot.v = (Res1+Res2)*0.5 + Vector(1, 0);
		if (abs_faster(dot.v) < 0.01) interrupt = 1;
		if ( (dot.g > 100) && (abs_faster(dot-s) < dt*0.5)) interrupt = 1;

		dot += dot.v*dt;
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
	if ( argc != 9) { cout << "Error! Please use: \n\tstreamlines file.vort file.body xmin xmax ymin ymax step lines_density\n"; return -1; }   
	Space *S = new Space(true, false);
	S->LoadVorticityFromFile(argv[1]);
	S->Body->LoadFromFile(argv[2]);
	
	const double dl = S->Body->SurfaceLength()/S->Body->List->size;

	InitTree(S, 10, dl*4);
	InitConvectiveFast(S, dl*dl/16);
	BuildTree(1, 0, 0);
	TList<TList<TObject>*> *StreamLines = new TList<TList<TObject>*>;

	boundary bnd;
	double density;
	sscanf(argv[3], "%lf", &bnd.xmin);
	sscanf(argv[4], "%lf", &bnd.xmax);
	sscanf(argv[5], "%lf", &bnd.ymin);
	sscanf(argv[6], "%lf", &bnd.ymax);
	sscanf(argv[7], "%lf", &bnd.step);
	sscanf(argv[8], "%lf", &density);

	TObject BodyRotation(0, 0, -S->gsum());
	S->VortexList->Copy(&BodyRotation);

	for ( double x=bnd.xmin; x<=bnd.xmax; x+= bnd.step )
	{
		for ( double y=bnd.ymin; y<=bnd.ymax; y+= bnd.step )
		{
			if ( S->Body->PointIsValid(Vector(x, y)) && (NearestTrack <true> (StreamLines, Vector(x, y)) > density) )
			{
				TList<TObject> *tmpStreamLine = new TList<TObject>;
				cout << x << " " << y << " " << CalculateStreamLine(S, &bnd, tmpStreamLine, Vector(x, y), bnd.step) << endl;
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
		TList<TObject>** StreamLineL = (TList<TObject>**)StreamLines->First;
		TList<TObject>* StreamLine;
		for (int i=0; i<StreamLines->size; i++)
		{
			StreamLine = *StreamLineL;
			TObject* Obj;

			Obj = StreamLine->First;
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

