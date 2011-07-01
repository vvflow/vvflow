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

int Space::LoadHeatFromStupidFile(const char* filename, double g)
{
	if ( !HeatList ) return -1;

	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; }

	TObj obj(0, 0, g);
	while ( fscanf(fin, "%lf %lf", &obj.rx, &obj.ry)==2 )
	{
		HeatList->push_back(obj);
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

int Space::Print(vector<TObj> *list, std::ostream& os)
{
	if (!list) return -1;
	if (!os) return -1;

	const_for (list, obj)
	{
		os << *obj << endl;
	}
	os << endl;
	return 0;
}

int Space::Print(vector<TObj> *list, const char* format, ios::openmode mode)
{
	int res;
	fstream fout;
	char fname[64];

	sprintf(fname, format, Time/dt);

	fout.open(fname, mode);
	res = Print(list, fout);
	fout.close();

	return res;
}

int Space::PrintBody(std::ostream& os)
	{ const_for(BodyList, llbody) { Print((**llbody).List, os); } return 0; }
int Space::PrintVorticity(std::ostream& os)
	{ return Print(VortexList, os); }
int Space::PrintHeat(std::ostream& os)
	{ return Print(HeatList, os); }
int Space::PrintBody(const char* filename)
{
	ios::openmode mode = ios::out;
	const_for(BodyList, llbody)
	{
		Print((**llbody).List, filename, mode);
		mode = ios::out | ios::app;
	}
	return 0;
}
int Space::PrintVorticity(const char* filename)
	{ return Print(VortexList, filename); }
int Space::PrintHeat(const char* filename)
	{ return Print(HeatList, filename); }

int Space::Save(const char *filename)
{
	/*long zero = 0;
	FILE * fout = fopen(filename, "w");
	if (!fout) return -1;

	if (VortexList)
	{
		fwrite(&(VortexList->size), sizeof(long), 1, pFile);
		fwrite(List->First, sizeof(TObj), List->size, pFile);
	} else { fwrite(&zero, sizeof(long), 1, pFile); }

	SaveList(VortexList)
	//SaveList(BodyList)
	SaveList(HeatList)

	fclose(pFile);*/
	return 0;
}

int Space::Load(const char *filename)
{
	/*long size;
	if (VortexList) delete VortexList;
	//if (BodyList) delete BodyList;
	if (HeatList) delete HeatList;
	//delete [] BodyControlLayer;

	FILE * pFile;
	pFile = fopen(filename, "r");
	if (!pFile) return -1;

	size_t err;
	#define LoadList(List) 													\
		err = fread(&size, sizeof(long), 1, pFile); 								\
		printf("%ld ", size); 												\
		if (size) 															\
		{ 																	\
			List = (TList<TObj>*)malloc(sizeof(TList<TObj>)); 							\
			List->maxsize = size; 											\
			List->size = size; 												\
			List->First = (TObj*)malloc(size*sizeof(TObj)); 		\
			err = fread(List->First, sizeof(TObj), size, pFile); 			\
		}

	LoadList(VortexList)
	//LoadList(BodyList)
	LoadList(HeatList)

	//if (BodyList) BodyControlLayer = new int[BodyList->size];

	fclose(pFile);*/
	return 0;
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
