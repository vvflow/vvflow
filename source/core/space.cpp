#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "space.h"

Space::Space(bool CreateVortexes,
				bool CreateHeat,
				TVec (*sInfSpeed)(double time))
{
	VortexList = CreateVortexes ? (new vector<TObj>()) : NULL;
	HeatList = CreateHeat ? (new vector<TObj>()) : NULL;
	BodyList = new vector<TBody*>();

	InfSpeed_link = sInfSpeed;
	Time = dt = 0;
}

/********************************** SAVE/LOAD *********************************/

int Space::LoadVorticityFromFile(const char* filename)
{
	if ( !VortexList ) return -1;

	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return -1; } 

	TObj obj(0, 0, 0);
	while ( fscanf(fin, "%lf %lf %lf", &obj.rx, &obj.ry, &obj.g)==3 )
	{
		VortexList->push_back(obj);
	}

	fclose(fin);
	return 0;
}

int Space::LoadHeatFromFile(const char* filename)
{
	if ( !HeatList ) return -1;

	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; }

	TObj obj(0, 0, 0);
	while ( fscanf(fin, "%lf %lf %lf", &obj.rx, &obj.ry, &obj.g)==3 )
	{
		HeatList->push_back(obj);
	}

	fclose(fin);
	return 0;
}

int Space::LoadBody(const char* filename)
{
	TBody *body = new TBody();
	size_t N=0; const_for(BodyList, llbody) N+= (**llbody).List->size();
	int res = body->LoadFromFile(filename, N);
	BodyList->push_back(body);
	return res;
}

/******************************** Print ***************************************/

int Space::Print_byos(vector<TObj> *list, std::ostream& os, bool bin)
{
	if (!list) return -1;
	if (!os) return -1;

	const_for (list, obj)
	{
		if (bin) os.write((const char *)(obj), 3*sizeof(double));
		else os << *obj << endl;
	}
	if (!bin) os << endl;
	return 0;
}

int Space::Print_bymask(vector<TObj> *list, const char* format, ios::openmode mode)
{
	int res;
	fstream fout;
	char fname[64];

	sprintf(fname, format, int(Time/dt));

	fout.open(fname, mode);
	if (mode & ios::binary) { fout.seekp(1024, ios::beg); }
	res = Print_byos(list, fout, (mode & ios::binary));
	fout.close();

	return res;
}

int Space::PrintBody(const char* filename)
{
	ios::openmode mode = ios::out;
	const_for(BodyList, llbody)
	{
		Print_bymask((**llbody).List, filename, mode);
		mode = ios::out | ios::app;
	}
	return 0;
}

int Space::PrintVorticity(const char* filename)
{
	return Print_bymask(VortexList, filename);
}

int Space::PrintVorticity_bin(const char* filename)
{
	return Print_bymask(VortexList, filename, ios::out | ios::binary);
}

int Space::PrintHeat(const char* filename)
{
	return Print_bymask(HeatList, filename);
}

void Space::PrintHeader(const char* format, const char* data, streamsize size)
{
	fstream fout;
	char fname[64]; sprintf(fname, format, int(Time/dt));

	fout.open(fname, ios::out | ios::binary);
	fout.write(data, min(size, 1024));
	fout.close();
}

double Space::integral()
{
	if (!VortexList) return 0;
	double sum = 0;

	const_for (VortexList, obj)
	{
		sum += obj->g * obj->abs2();
	}

	return sum;
}

double Space::gsum()
{
	if (!VortexList) return 0;
	double sum = 0;

	const_for (VortexList, obj)
	{
		sum += obj->g;
	}

	return sum;
}

double Space::gmax()
{
	if (!VortexList) return 0;
	double max = 0;

	const_for (VortexList, obj)
	{
		max = ( fabs(obj->g) > fabs(max) ) ? obj->g : max;
	}

	return max;
}

TVec Space::HydroDynamicMomentum()
{
	TVec res(0, 0);
	if (!VortexList) return res;

	const_for (VortexList, obj)
	{
		res += obj->g * TVec(*obj);
	}
	return res;
}
