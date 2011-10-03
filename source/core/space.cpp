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
	StreakSourceList = new vector<TObj>();
	StreakList = new vector<TObj>();

	InfSpeed_link = sInfSpeed;
	Time = dt = 0;
}

/********************************** SAVE/LOAD *********************************/

int bookmark;
void SaveBookmark(FILE* fout)
{
	size_t tmp = ftell(fout);
	fseek(fout, bookmark*8, SEEK_SET);
	fwrite(&tmp, 8, 1, fout);
	fseek(fout, tmp, SEEK_SET);
	bookmark++;
}

void SaveList(vector<TObj> *list, FILE* fout)
{
	size_t size = list->size_safe();
	fwrite(&size, 8, 1, fout);
	if (!list) return;
	const_for (list, obj)
	{
		fwrite(pointer(obj), 24, 1, fout);
	}
}

void Space::Save(const char* format)
{
	int res;
	char fname[64]; sprintf(fname, format, int(Time/dt));
	FILE *fout;
	fout = fopen(fname, "rb+");
	if (!fout) fout = fopen(fname, "wb");
	if (!fout) { perror("Error saving the space"); return; }
	fseek(fout, 1024, SEEK_SET);

	bookmark = 0;
	SaveBookmark(fout); SaveList(VortexList, fout);
	SaveBookmark(fout); SaveList(HeatList, fout);
	SaveBookmark(fout); const_for(BodyList, llbody)
	{
		SaveList((**llbody).List, fout);
	}
	SaveBookmark(fout); SaveList(StreakSourceList, fout);
	SaveBookmark(fout); SaveList(StreakList, fout);

	fclose(fout);
}

void Space::Load(const char* format)
{

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

int Space::LoadVorticity_bin(const char* filename)
{
	if ( !VortexList ) return -1;

	fstream fin;
	fin.open(filename, ios::in | ios::binary);
	if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return -1; } 

	fin.seekg (0, ios::end);
	size_t N = (size_t(fin.tellg())-1024)/(sizeof(double)*3);
	fin.seekp(1024, ios::beg);

	TObj obj(0, 0, 0);
	
	while ( fin.good() )
	{
		fin.read(pchar(&obj), 3*sizeof(double));
		VortexList->push_back(obj);
	}

	fin.close();
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
	return Print_bymask(VortexList, filename, ios::out | ios::in | ios::binary);
}

int Space::PrintHeat(const char* filename)
{
	return Print_bymask(HeatList, filename);
}

/********************************* HEADERS ************************************/

void Space::LoadHeader(const char* fname, char* data, streamsize size)
{
	fstream fin;

	fin.open(fname, ios::in | ios::binary);
	fin.read(data, min(size, 1024));
	fin.close();
}

void Space::PrintHeader(const char* format, const char* data, size_t size)
{
	FILE *fout;
	char fname[64]; sprintf(fname, format, int(Time/dt));

	fout = fopen(fname, "rb+");
	fwrite(data, min(size, 1024), 1, fout);
	fclose(fout);
}

/********************************* INTEGRALS **********************************/

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
