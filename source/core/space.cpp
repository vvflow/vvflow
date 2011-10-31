#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <malloc.h>
#include <cstring>
#include <fstream>
#include <time.h>
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

void SaveBookmark(FILE* fout, int bookmark, const char *comment)
{
	int64_t tmp = ftell(fout);
	fseek(fout, bookmark*16, SEEK_SET);
	fwrite(&tmp, 8, 1, fout);
	fwrite(comment, 1, 8, fout);
	fseek(fout, tmp, SEEK_SET);
}

void SaveList(vector<TObj> *list, FILE* fout)
{
	int64_t size = list->size_safe();
	fwrite(&size, 8, 1, fout);
	if (!list) return;
	const_for (list, obj)
	{
		fwrite(pointer(obj), 24, 1, fout);
	}
}

void LoadList(vector<TObj> *list, FILE* fin)
{
	int64_t size; fread(&size, 8, 1, fin);
	TObj obj;
	if (!list) list = new vector<TObj>;
	for (int64_t i=0; i<size; i++)
	{
		fread(&obj, 24, 1, fin);
		list->push_back(obj);
	}
}

void Space::Save(const char* format, const double header[], int N)
{
	char fname[64]; sprintf(fname, format, int(Time/dt));
	FILE *fout = fopen(fname, "rb+");
	if (!fout) fout = fopen(fname, "wb");
	if (!fout) { perror("Error saving the space"); return; }

	fseek(fout, 8*128, SEEK_SET);
	//writinh header
	SaveBookmark(fout, 0, "Header  ");
	fwrite(header, 8, N, fout);
	time_t rt; time(&rt); int64_t rawtime=rt;
	fwrite(&rawtime, 8, 1, fout);
	fwrite(&Time, 8, 1, fout);

	//writing lists
	SaveBookmark(fout, 1, "Vortexes"); SaveList(VortexList, fout);
	SaveBookmark(fout, 2, "Heat    "); SaveList(HeatList, fout);
	SaveBookmark(fout, 3, "StrkSrc "); SaveList(StreakSourceList, fout);
	SaveBookmark(fout, 4, "Streak  "); SaveList(StreakList, fout);
	//FIXME implement Attaches

	int bookmark = 4;
	const_for(BodyList, llbody)
	{
		SaveBookmark(fout, ++bookmark, "Body");
		SaveList((**llbody).List, fout);
	}

	fclose(fout);
}

int eq(const char *str1, const char *str2)
{
	for (int i=0; i<8; i++)
	{
		if (str1[i] != str2[i]) return i+1;
	}
	return 0;
}

double* Space::Load(const char* fname, int* N)
{
	FILE *fin = fopen(fname, "rb");
	if (!fin) { perror("Error loading the space"); return NULL; }
	double *header = NULL;

	//loading header
	{
		fseek(fin, 16, SEEK_SET); //seek to the 2nd bookmark
		int64_t tmp; fread(&tmp, 8, 1, fin); //getting its address
		int size = (tmp-1024)/8; //conputing number of doubles in header
		header = (double*)malloc(8*size); //allocating mem to store it
		fseek(fin, 8*128, SEEK_SET); //seeking to begin of header
		fread(header, 8, size, fin); //reading header
		if (N) *N = size-2; //returning size of header
		Time = header[size-1]; //obtaining current time from header
	}

	//loading different lists
	int64_t tmp;
	char comment[9]; comment[8]=0;
	for (int i=1; i<64; i++)
	{
		fseek(fin, i*16, SEEK_SET);
		fread(&tmp, 8, 1, fin);
		fread(comment, 8, 1, fin);
		if (!tmp) continue;
		fseek(fin, tmp, SEEK_SET);

		     if (!eq(comment, "Vortexes")) LoadList(VortexList, fin);
		else if (!eq(comment, "Heat    ")) LoadList(HeatList, fin);
		else if (!eq(comment, "StrkSrc ")) LoadList(StreakSourceList, fin);
		else if (!eq(comment, "Streak  ")) LoadList(StreakList, fin);
		else if (eq(comment, "Body    ")>=5)
		{
			TBody *body = new TBody();
			BodyList->push_back(body);

			TObj obj; TAtt att(body, 0); att.zero();
			int64_t size; fread(&size, 8, 1, fin);
			for (int64_t i=0; i<size; i++)
			{
				fread(&obj, 24, 1, fin);
				body->List->push_back(obj);
				body->AttachList->push_back(att);
			}

			body->InsideIsValid = body->isInsideValid();
			body->UpdateAttach();
		}
		else cout << "ignoring field " << comment << endl;
	}

	EnumerateBodies();

	return header;
}

FILE* Space::OpenFile(const char* format)
{
	char fname[64]; sprintf(fname, format, int(Time/dt));
	FILE *fout;
	fout = fopen(fname, "w");
	if (!fout) { perror("Error opening file"); return NULL; }
	return fout;
}

void Space::LoadHeader(const char* fname, char* data, streamsize size)
{
	/*fstream fin;

	fin.open(fname, ios::in | ios::binary);
	fin.read(data, min(size, 1024));
	fin.close();*/
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

void Space::EnumerateBodies()
{
	int eq_no=0;
	const_for(BodyList, llbody)
	{
		#define body (**llbody)
		const_for(body.AttachList, latt)
		{
			//FIXME bc
			latt->bc = bc::noslip;
			latt->eq_no = eq_no++;
		}
		body.AttachList->begin()->bc = bc::tricky;
		#undef body
	}
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
