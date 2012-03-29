#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <malloc.h>
#include <cstring>
#include <fstream>
#include <time.h>
using namespace std;

#include "space.h"

Space::Space(TVec (*sInfSpeed)(double time))
{
	VortexList = NULL; //new vector<TObj>();
	HeatList = NULL; //new vector<TObj>();
	BodyList = new vector<TBody*>();

	StreakSourceList = new vector<TObj>();
	StreakList = new vector<TObj>();

	InfSpeed_link = sInfSpeed;
	InfSpeed_const.zero();
	Time = dt = Re = Pr = 0;
}

inline
void Space::FinishStep()
{
	const_for(BodyList, llbody)
	{
		TBody &body = **llbody;
		body.Rotate(body.RotSpeed(Time) * dt);
		body.Position -= InfSpeed() * dt;
	}
	Time+= dt;
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

void LoadList(vector<TObj> *&list, FILE* fin)
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
	char fname[64]; sprintf(fname, format, int(Time/dt+0.5));
	FILE *fout = fopen(fname, "rb+");
	if (!fout) fout = fopen(fname, "wb");
	if (!fout) { perror("Error saving the space"); return; }

	fseek(fout, 8*128, SEEK_SET);
	//writinh header
	SaveBookmark(fout, 0, "Header  ");
	fwrite(header, 8, N, fout);
	fwrite(&dt, 8, 1, fout);
	fwrite(&Re, 8, 1, fout);
	TVec inf = InfSpeed(); fwrite(&inf, 8, 2, fout);
	time_t rt; time(&rt); int64_t rawtime=rt; fwrite(&rawtime, 8, 1, fout);
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
		SaveBookmark(fout, ++bookmark, "Body    ");
		SaveList((**llbody).List, fout);
		TObj rot;
		rot = (**llbody).RotAxis;
		rot.g = (**llbody).RotSpeed(Time);
		fwrite(&rot, 24, 1, fout);
	}

	fclose(fout);
}

int eq(const char *str1, const char *str2)
{
	for (int i=0; i<8; i++)
	{
		if (str1[i] != str2[i]) return i+1;
	}
	return 9;
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
		int size = (tmp-1024)/8; //computing number of doubles in header
		header = (double*)malloc(8*size); //allocating mem to store it
		fseek(fin, 8*128, SEEK_SET); //seeking to begin of header
		fread(header, 8, size, fin); //reading header
		if (N) *N = size-2; //returning size of header
		Time = header[size-1]; //obtaining current time from header
		InfSpeed_const = TVec(header[size-4], header[size-3]); //get infspeed
		Re = header[size-5]; //get Reynolds
		dt = header[size-6]; //get dt
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

		     if (eq(comment, "Vortexes")>8) LoadList(VortexList, fin);
		else if (eq(comment, "Heat    ")>8) LoadList(HeatList, fin);
		else if (eq(comment, "StrkSrc ")>8) LoadList(StreakSourceList, fin);
		else if (eq(comment, "Streak  ")>8) LoadList(StreakList, fin);
		else if (eq(comment, "Body    ")>4)
		{
			TBody *body = new TBody(this);
			BodyList->push_back(body);

			TObj obj; TAtt att; att.body = body; att.zero();
			int64_t size; fread(&size, 8, 1, fin);
			for (int64_t i=0; i<size; i++)
			{
				fread(&obj, 24, 1, fin);
				body->List->push_back(obj);
				body->AttachList->push_back(att);
			}

			TObj rot; fread(&rot, 24, 1, fin);
			body->SetRotation(TVec(rot), NULL, rot.g);

			body->InsideIsValid = body->isInsideValid();
			body->UpdateAttach();
		}
		else cout << "S->Load(): ignoring field \"" << comment << "\"\n";
	}

	EnumerateBodies();

	return header;
}

FILE* Space::OpenFile(const char* format)
{
	char fname[64]; sprintf(fname, format, int(Time/dt+0.5));
	FILE *fout;
	fout = fopen(fname, "w");
	if (!fout) { perror("Error opening file"); return NULL; }
	return fout;
}

void Space::CalcForces()
{
	const double C_NyuDt_Pi = dt/(C_PI*Re);
	const double C_Nyu_Pi = 1./(C_PI*Re);
	#define body (**llbody)
	const_for(BodyList, llbody)
	{
		double tmp_gsum = 0;
		//TObj tmp_fric(0,0,0);
		const_for(body.AttachList, latt)
		{
			tmp_gsum+= latt->gsum;
			latt->Cp += tmp_gsum;
			latt->Fr += latt->fric * C_NyuDt_Pi;
			latt->Nu += latt->hsum * (Re*Pr / latt->dl.abs());

			body.Friction += latt->dl * (latt->fric * C_Nyu_Pi / latt->dl.abs());
			body.Friction.g += (rotl(*latt)* latt->dl) * (latt->fric  * C_Nyu_Pi / latt->dl.abs());
			body.Nusselt += latt->hsum * (Re*Pr);
		}

		body.Force /= dt;
		body.Force.g /= dt;
		//body.Friction /= dt;
		//body.Friction.g /= dt;
		body.Nusselt /= dt;
	}
	#undef body
}

void Space::SaveProfile(const char* fname, double save_dt, TValues vals)
{
	if (!divisible(Time, save_dt, dt/2)) return;
	int32_t vals_32=vals, N=0;
	const_for(BodyList, llbody) { N+= (**llbody).size(); }
	if (!N) return;
	if (!VortexList) vals_32 &= ~(val::Cp | val::Fr);
	if (!HeatList) vals_32 &= ~val::Nu;

	FILE *fout = fopen(fname, "ab");
	if (!fout) { perror("Error saving the body profile"); return; }
	if (!ftell(fout)) { fwrite(&vals_32, 4, 1, fout); fwrite(&N, 4, 1, fout); }
	float time_tmp = Time; fwrite(&time_tmp, 4, 1, fout);
	float buf[5];

	const_for(BodyList, llbody)
	{
		TBody &body = **llbody;
		const_for(body.AttachList, latt)
		{
			buf[0] = body.obj(latt)->rx;
			buf[1] = body.obj(latt)->ry;
			buf[2] = latt->Cp/save_dt;
			buf[3] = latt->Fr/save_dt;
			buf[4] = latt->Nu/save_dt;

			fwrite(buf, 4, 2, fout);
			if (vals_32 & val::Cp) fwrite(buf+2, 4, 1, fout);
			if (vals_32 & val::Fr) fwrite(buf+3, 4, 1, fout);
			if (vals_32 & val::Nu) fwrite(buf+4, 4, 1, fout);

			latt->Cp = latt->Fr = latt->Nu = 0;
		}
	}
	fclose(fout);
}

void Space::ZeroForces()
{
	const_for(BodyList, llbody)
	{
		const_for((**llbody).AttachList, latt)
		{
			latt->gsum =
			latt->fric =
			latt->hsum = 0;
			latt->ParticleInHeatLayer = -1;
		}

		(**llbody).Force.zero();
		(**llbody).Friction.zero();
		(**llbody).Nusselt = 0;
	}
}

/********************************** SAVE/LOAD *********************************/

int Space::LoadVorticityFromFile(const char* filename)
{
	if ( !VortexList ) VortexList = new vector<TObj>();

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
	if ( !VortexList ) VortexList = new vector<TObj>();

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
	if ( !HeatList ) HeatList = new vector<TObj>();

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

int Space::LoadStreak(const char* filename)
{
	FILE *fin = fopen(filename, "r");
	if (!fin) { perror("Error opening streak file"); return -1; }

	TObj obj(0, 0, 0);
	while ( fscanf(fin, "%lf %lf %lf", &obj.rx, &obj.ry, &obj.g)==3 )
	{
		StreakList->push_back(obj);
	}

	fclose(fin);
	return 0;
}

int Space::LoadStreakSource(const char* filename)
{
	FILE *fin = fopen(filename, "r");
	if (!fin) { perror("Error opening streak source file"); return -1; }

	TObj obj(0, 0, 0);
	while ( fscanf(fin, "%lf %lf %lf", &obj.rx, &obj.ry, &obj.g)==3 )
	{
		StreakSourceList->push_back(obj);
	}

	fclose(fin);
	return 0;
}

int Space::LoadBody(const char* filename, int cols)
{
	TBody *body = new TBody(this);
	BodyList->push_back(body);

	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObj obj(0, 0, 0);
	TAtt att; att.body = body; att.zero();
	char *pattern;
	switch (cols)
	{
		case 2: pattern = "%lf %lf \n"; break;
		case 3: pattern = "%lf %lf %d \n"; break;
		case 5: pattern = "%lf %lf %d %d %lf \n"; break;
		default: fprintf(stderr, "Bad columns number. Only 2 3 or 5 supported\n"); return -1;
	}

	while ( fscanf(fin, pattern, &obj.rx, &obj.ry, &att.bc, &att.hc, &att.heat_const)==cols )
	{
		body->List->push_back(obj);
		body->AttachList->push_back(att);
	}

	if (!VortexList)
	const_for(body->AttachList, latt)
	{
		if(latt->bc == bc::noslip) { VortexList = new vector<TObj>(); break; }
	}

	if (!HeatList)
	const_for(body->AttachList, latt)
	{
		if(latt->hc != hc::neglect) { HeatList = new vector<TObj>(); break; }
	}

	fclose(fin);
	body->InsideIsValid = body->isInsideValid();
	body->UpdateAttach();
	EnumerateBodies();

	return 0;
}

void Space::EnumerateBodies(bool cheat)
{
	int eq_no=0;
	const_for(BodyList, llbody)
	{
		const_for((**llbody).AttachList, latt)
		{
			//FIXME load bc
			if (cheat) latt->bc = bc::noslip;
			latt->eq_no = eq_no++;
		}

		if (cheat) (**llbody).AttachList->begin()->bc = bc::steady;
	}
	if (cheat) (**BodyList->begin()).AttachList->begin()->bc = bc::inf_steady;
}

/********************************* INTEGRALS **********************************/

void Space::ZeroSpeed()
{
	const_for (VortexList, lobj)
	{
		lobj->v.zero();
	}

	const_for (HeatList, lobj)
	{
		lobj->v.zero();
	}
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

double Space::AverageSegmentLength()
{
	if (!BodyList) return DBL_MIN;
	double SurfaceLength = 0;
	int N = 0;
	const_for(BodyList, llbody)
	{
		SurfaceLength+= (**llbody).SurfaceLength();
		N+= (**llbody).size() - 1;
	}

	if (!N) return DBL_MIN;
	return SurfaceLength / N;
}
