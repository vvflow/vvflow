#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <cstring>
#include <fstream>
#include <time.h>
#include <assert.h>
using namespace std;

#include "space.h"
#include "space_hdf.cpp"

#ifndef DEF_GITINFO
	#define DEF_GITINFO "not available"
#endif
#ifndef DEF_GITDIFF
	#define DEF_GITDIFF "not available"
#endif
static const char* gitInfo = DEF_GITINFO;
static const char* gitDiff = DEF_GITDIFF;
const char* Space::getGitInfo() {return gitInfo;}
const char* Space::getGitDiff() {return gitDiff;}

Space::Space(): caption()
{
	VortexList = NULL; //new vector<TObj>();
	HeatList = NULL; //new vector<TObj>();
	BodyList = new vector<TBody*>();

	StreakSourceList = new vector<TObj>();
	StreakList = new vector<TObj>();

	InfSpeedX = new ShellScript();
	InfSpeedY = new ShellScript();
	InfCirculation = 0.;
	gravitation = TVec(0., 0.);
	Finish = DBL_MAX;
	Time = dt = TTime(0, 0);
	save_dt = streak_dt = profile_dt = TTime(INT32_MAX, 1);
	Re = Pr = 0.;
	InfMarker = TVec(0., 0.);
}

inline
void Space::FinishStep()
{
	const_for(BodyList, llbody)
	{
		TBody &body = **llbody;
		body.doRotationAndMotion();
	}
	Time= TTime::add(Time, dt);
}

/*
888    888 8888888b.  8888888888 888888888
888    888 888  "Y88b 888        888
888    888 888    888 888        888
8888888888 888    888 8888888    8888888b.
888    888 888    888 888             "Y88b
888    888 888    888 888               888
888    888 888  .d88P 888        Y88b  d88P
888    888 8888888P"  888         "Y8888P"
*/

/*
 .d8888b.         d8888 888     888 8888888888
d88P  Y88b       d88888 888     888 888
Y88b.           d88P888 888     888 888
 "Y888b.       d88P 888 Y88b   d88P 8888888
    "Y88b.    d88P  888  Y88b d88P  888
      "888   d88P   888   Y88o88P   888
Y88b  d88P  d8888888888    Y888P    888
 "Y8888P"  d88P     888     Y8P     8888888888
*/

void dataset_write_list(const char *name, vector<TObj> *list)
{
	if (!commited_obj)
	{
		commited_obj = true;
		H5Tcommit2(fid, "obj_t", obj_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}
	
	// 1D dataspace
	hsize_t dim = list->size_safe();
	if (dim == 0) return;
	hsize_t dim2 = dim*2;
	hsize_t chunkdim = 512;
	hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(prop, 1, &chunkdim);
	H5Pset_deflate(prop, 9);

	hid_t file_dataspace = H5Screate_simple(1, &dim, &dim);
	hid_t mem_dataspace = H5Screate_simple(1, &dim2, &dim2);
	assert(file_dataspace>=0);
	hsize_t offset = 0;
	hsize_t stride = 2;
	hsize_t count = dim;
	H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_SET, &offset, &stride, &count, NULL);
	hid_t file_dataset = H5Dcreate2(fid, name, obj_t, file_dataspace, H5P_DEFAULT, prop, H5P_DEFAULT);
	assert(file_dataset>=0);
	H5Dwrite(file_dataset, obj_t, mem_dataspace, file_dataspace, H5P_DEFAULT, list->begin());
	H5Dclose(file_dataset);
}

void dataset_write_body(const char* name, TBody *body)
{
	assert(body);
	if (!commited_body_stuff)
	{
		commited_body_stuff = true;
		H5Tcommit2(fid, "boundary_condition_t", bc_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Tcommit2(fid, "heat_condition_t", hc_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Tcommit2(fid, "body_attach_t", att_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}

	hc::HeatCondition heat_condition = body->List->begin()->hc;
	double heat_const = body->List->begin()->heat_const;
	bc::BoundaryCondition special_bc;
	bc::BoundaryCondition general_bc = bc::zero;
	long int special_bc_segment = -1;

	hsize_t dim = body->size();
	struct ATT *mem = (struct ATT*)malloc(sizeof(struct ATT)*dim);
	for(hsize_t i=0; i<dim; i++)
	{
		TAtt latt = body->List->at(i);
		mem[i].x = latt.corner.x;
		mem[i].y = latt.corner.y;
		mem[i].g = latt.g;
		mem[i].gsum = latt.gsum;

		if ( (latt.bc != bc::slip) &&
			(latt.bc != bc::noslip) )
		{
			if (special_bc_segment>=0) assert(0);
			special_bc = (latt.bc == bc::zero) ? bc::zero : bc::steady;
			special_bc_segment = i;
		} else
		{
			if ( (general_bc != bc::zero) && (latt.bc != general_bc) ) assert(0);
			general_bc = latt.bc;
		}

		if (heat_const != latt.heat_const) assert(0);
		if (heat_condition != latt.hc) assert(0);
	}

	hsize_t chunkdim = 512;
	hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(prop, 1, &chunkdim);
	H5Pset_deflate(prop, 9);

	hid_t file_dataspace = H5Screate_simple(1, &dim, &dim);
	assert(file_dataspace>=0);

	hid_t file_dataset = H5Dcreate2(fid, name, att_t, file_dataspace, H5P_DEFAULT, prop, H5P_DEFAULT);
	assert(file_dataset>=0);

	attribute_write(file_dataset, "simplified_dataset", true);
	attribute_write(file_dataset, "holder_position", body->pos);
	attribute_write(file_dataset, "delta_position", body->dPos);
	attribute_write(file_dataset, "speed_x", body->SpeedX->getScript());
	attribute_write(file_dataset, "speed_y", body->SpeedY->getScript());
	attribute_write(file_dataset, "speed_o", body->SpeedO->getScript());
	attribute_write(file_dataset, "speed_slae", body->Speed_slae);
	attribute_write(file_dataset, "speed_slae_prev", body->Speed_slae_prev);
	attribute_write(file_dataset, "spring_const", body->k);
	attribute_write(file_dataset, "density", body->density);
	attribute_write(file_dataset, "force_born", body->Force_born);
	attribute_write(file_dataset, "force_dead", body->Force_dead);
	attribute_write(file_dataset, "friction_prev", body->Friction_prev);

	H5Dwrite(file_dataset, att_t, H5S_ALL, file_dataspace, H5P_DEFAULT, mem);
	H5Dclose(file_dataset);
	free(mem);
}

void Space::Save(const char* format)
{
	char fname[64]; sprintf(fname, format, int(Time/dt+0.5));
	fid = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	assert(fid>=0);
	datatypes_create_all();
	
	attribute_write(fid, "caption", caption.c_str());
	attribute_write(fid, "time", Time);
	attribute_write(fid, "dt", dt);
	attribute_write(fid, "dt_save", save_dt);
	attribute_write(fid, "dt_streak", streak_dt);
	attribute_write(fid, "dt_profile", profile_dt);
	attribute_write(fid, "re", Re);
	attribute_write(fid, "pr", Pr);
	attribute_write(fid, "inf_marker", InfMarker);
	attribute_write(fid, "inf_speed_x", InfSpeedX->getScript());
	attribute_write(fid, "inf_speed_y", InfSpeedY->getScript());
	attribute_write(fid, "inf_circulation", InfCirculation);
	attribute_write(fid, "gravity", gravitation);
	attribute_write(fid, "time_to_finish", Finish);
	attribute_write(fid, "git_info", gitInfo);
	attribute_write(fid, "git_diff", gitDiff);

	time_t rt; time(&rt);
	char *timestr = ctime(&rt); timestr[strlen(timestr)-1] = 0;
	attribute_write(fid, "time_local", timestr);

	dataset_write_list("vort", VortexList);
	dataset_write_list("heat", HeatList);
	dataset_write_list("ink", StreakList);
	dataset_write_list("ink_source", StreakSourceList);

	const_for(BodyList, llbody)
	{
		char body_name[16];
		sprintf(body_name, "body%02zd", BodyList->find(llbody));
		dataset_write_body(body_name, *llbody);
	}

	datatypes_close_all();
	assert(H5Fclose(fid)>=0);
}

/*
888       .d88888b.         d8888 8888888b.
888      d88P" "Y88b       d88888 888  "Y88b
888      888     888      d88P888 888    888
888      888     888     d88P 888 888    888
888      888     888    d88P  888 888    888
888      888     888   d88P   888 888    888
888      Y88b. .d88P  d8888888888 888  .d88P
88888888  "Y88888P"  d88P     888 8888888P"
*/

#include "H5Cpp.h"

void Space::Load(const char* fname)
{
	if (!H5Fis_hdf5(fname))
	{
		Load_v1_3(fname);
		return;
	}

	hid_t fid = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
	assert(fid>=0);
	datatypes_create_all();

	attribute_read(fid, "caption", caption);
	attribute_read(fid, "time", Time);
	attribute_read(fid, "dt", dt);
	attribute_read(fid, "dt_save", save_dt);
	attribute_read(fid, "dt_streak", streak_dt);
	attribute_read(fid, "dt_profile", profile_dt);
	attribute_read(fid, "re", Re);
	attribute_read(fid, "pr", Pr);
	attribute_read(fid, "inf_marker", InfMarker);
	// attribute_read(fid, "inf_speed_x", InfSpeedX->getScript());
	// attribute_read(fid, "inf_speed_y", InfSpeedY->getScript());
	attribute_read(fid, "inf_circulation", InfCirculation);
	attribute_read(fid, "gravity", gravitation);
	attribute_read(fid, "time_to_finish", Finish);
	
	datatypes_close_all();
	assert(H5Fclose(fid)>=0);
}

/*
888       .d88888b.         d8888 8888888b.       .d88888b.  888      8888888b.
888      d88P" "Y88b       d88888 888  "Y88b     d88P" "Y88b 888      888  "Y88b
888      888     888      d88P888 888    888     888     888 888      888    888
888      888     888     d88P 888 888    888     888     888 888      888    888
888      888     888    d88P  888 888    888     888     888 888      888    888
888      888     888   d88P   888 888    888     888     888 888      888    888
888      Y88b. .d88P  d8888888888 888  .d88P     Y88b. .d88P 888      888  .d88P
88888888  "Y88888P"  d88P     888 8888888P"       "Y88888P"  88888888 8888888P"
*/

int eq(const char *str1, const char *str2)
{
	for (int i=0; i<8; i++)
	{
		if (str1[i] != str2[i]) return i+1;
	}
	return 9;
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

void Space::Load_v1_3(const char* fname)
{
	FILE *fin = fopen(fname, "rb");
	if (!fin) { perror("Error loading the space"); return; }

	//loading different lists
	int64_t tmp;
	char comment[9]; comment[8]=0;
	for (int i=0; i<64; i++)
	{
		fseek(fin, i*16, SEEK_SET);
		fread(&tmp, 8, 1, fin);
		fread(comment, 8, 1, fin);
		if (!tmp) continue;
		fseek(fin, tmp, SEEK_SET);

		if (eq(comment, "Header  ")>8)
		{
			char version[8]; fread(&version, 8, 1, fin);
			if (eq(version, "v: 1.3  ") <= 8)
			{
				fprintf(stderr, "Cant read binary file version \"%s\".\n", version);
				exit(1);
			}

			char name[64];
			fread(name, 1, 64, fin);
			caption = name;
			fread(&Time, 8, 1, fin);
			fread(&dt, 8, 1, fin);
			fread(&save_dt, 8, 1, fin);
			fread(&streak_dt, 8, 1, fin);
			fread(&profile_dt, 8, 1, fin);
			fread(&Re, 8, 1, fin);
			fread(&Pr, 8, 1, fin);
			fread(&InfMarker, 16, 1, fin);
			InfSpeedX->read(fin);
			InfSpeedY->read(fin);
			fread(&InfCirculation, 8, 1, fin);
			fread(&gravitation, 16, 1, fin);
			fread(&Finish, 8, 1, fin);

			int64_t rawtime; fread(&rawtime, 8, 1, fin); realtime = rawtime;
		}
		else if (eq(comment, "Vortexes")>8) LoadList(VortexList, fin);
		else if (eq(comment, "Heat    ")>8) LoadList(HeatList, fin);
		else if (eq(comment, "StrkSrc ")>8) LoadList(StreakSourceList, fin);
		else if (eq(comment, "Streak  ")>8) LoadList(StreakList, fin);
		else if (eq(comment, "BData   ")>8)
		{
			TBody *body = new TBody(this);
			BodyList->push_back(body);

			fread(&body->pos, 24, 1, fin);
			fread(&body->dPos, 24, 1, fin);
			body->SpeedX->read(fin);
			body->SpeedY->read(fin);
			body->SpeedO->read(fin);
			fread(&body->Speed_slae, 24, 1, fin);
			fread(&body->Speed_slae_prev, 24, 1, fin);

			fread(&body->k, 24, 1, fin);
			fread(&body->density, 8, 1, fin);

			fread(&body->Force_born, 24, 1, fin);
			fread(&body->Force_dead, 24, 1, fin);
			fread(&body->Friction_prev, 24, 1, fin);
		}
		else if (eq(comment, "Body    ")>8)
		{
			TBody *body = *(BodyList->end()-1);

			TAtt att; att.body = body;
			int64_t size; fread(&size, 8, 1, fin);
			for (int64_t i=0; i<size; i++)
			{
				int64_t bc, hc;
				fread(&att.corner, 16, 1, fin);
				fread(&att.g, 8, 1, fin);
				fread(&bc, 8, 1, fin);
				fread(&hc, 8, 1, fin);
				fread(&att.heat_const, 8, 1, fin);
				att.bc = bc::bc(bc);
				att.hc = hc::hc(hc);
				fread(&att.gsum, 8, 1, fin);

				body->List->push_back(att);
			}
			body->doUpdateSegments();
			body->doFillProperties();
		}
		else fprintf(stderr, "S->Load(): ignoring field \"%s\"", comment);
	}

	EnumerateBodies();

	return;
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
		body.Friction_prev = body.Friction;
		body.Friction = TVec3D();

		const_for(body.List, latt)
		{
			tmp_gsum+= latt->gsum;
			latt->Cp += tmp_gsum;
			latt->Fr += latt->fric * C_NyuDt_Pi;
			latt->Nu += latt->hsum * (Re*Pr / latt->dl.abs());

			body.Friction.r -= latt->dl * (latt->fric * C_Nyu_Pi / latt->dl.abs());
			body.Friction.o -= (rotl(latt->r)* latt->dl) * (latt->fric  * C_Nyu_Pi / latt->dl.abs());
			body.Nusselt += latt->hsum * (Re*Pr);
		}

		body.Force_export.r = body.Force_born.r - body.Force_dead.r;
		body.Force_export.o = body.Force_born.o - body.Force_dead.o;
		body.Nusselt /= dt;
	}

	//FIXME calculate total pressure
	#undef body
}

void Space::SaveProfile(const char* fname, TValues vals)
{
	if (!Time.divisibleBy(profile_dt)) return;
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
		const_for((**llbody).List, latt)
		{
			buf[0] = latt->corner.x;
			buf[1] = latt->corner.y;
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
		const_for((**llbody).List, latt)
		{
			latt->gsum =
			latt->fric =
			latt->hsum = 0;
			latt->ParticleInHeatLayer = -1;
		}

		(**llbody).Force_dead = TVec3D();
		(**llbody).Force_born = TVec3D();
		(**llbody).Friction = TVec3D();
		(**llbody).Nusselt = 0.;
	}
}

/********************************** SAVE/LOAD *********************************/

int Space::LoadVorticityFromFile(const char* filename)
{
	if ( !VortexList ) VortexList = new vector<TObj>();

	FILE *fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return -1; }

	TObj obj(0, 0, 0);
	while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
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
	while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
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
	while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
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
	while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
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

	TAtt att; att.body = body;
	att.heat_const = 0;
	char bc_char('n'), hc_char('n');

	char *pattern;
	switch (cols)
	{
		case 2: pattern = "%lf %lf \n"; break;
		case 3: pattern = "%lf %lf %c \n"; break;
		case 5: pattern = "%lf %lf %c %c %lf \n"; break;
		default: fprintf(stderr, "Bad columns number. Only 2 3 or 5 supported\n"); return -1;
	}

	while ( fscanf(fin, pattern, &att.corner.x, &att.corner.y, &bc_char, &hc_char, &att.heat_const)==cols )
	{
		att.bc = bc::bc(bc_char);
		att.hc = hc::hc(hc_char);
		body->List->push_back(att);
	}

	if (!VortexList)
	const_for(body->List, latt)
	{
		if(latt->bc == bc::noslip) { VortexList = new vector<TObj>(); break; }
	}

	if (!HeatList)
	const_for(body->List, latt)
	{
		if(latt->hc != hc::neglect) { HeatList = new vector<TObj>(); break; }
	}

	fclose(fin);
	body->doUpdateSegments();
	body->doFillProperties();
	EnumerateBodies();

	return 0;
}

void Space::EnumerateBodies()
{
	int eq_no=0;
	TBody *bodyWithInfSteadyBC = NULL;
	//there must be 1 and only 1 body with inf_steady condition

	const_for(BodyList, llbody)
	{
		const_for((**llbody).List, latt)
		{
			if (latt->bc == bc::inf_steady)
			{
				if (bodyWithInfSteadyBC)
					latt->bc = bc::steady;
				else
					bodyWithInfSteadyBC = *llbody;
			}
			latt->eq_no = eq_no;
			eq_no++;
		}

		(**llbody).eq_forces_no = eq_no;
		eq_no+= 3;
	}
}

/********************************* INTEGRALS **********************************/

void Space::ZeroSpeed()
{
	const_for (VortexList, lobj)
	{
		lobj->v = TVec();
	}

	const_for (HeatList, lobj)
	{
		lobj->v = TVec();
	}
}

double Space::integral()
{
	if (!VortexList) return 0;
	double sum = 0;

	const_for (VortexList, obj)
	{
		sum += obj->g * obj->r.abs2();
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
	TVec res(0., 0.);
	if (!VortexList) return res;

	const_for (VortexList, obj)
	{
		res += obj->g * obj->r;
	}
	return res;
}

double Space::AverageSegmentLength()
{
	if (!BodyList->size_safe()) return DBL_MIN;

	double SurfaceLength = BodyList->at(0)->getSurface();
	int N = BodyList->at(0)->size() - 1;

	if (!N) return DBL_MIN;
	return SurfaceLength / N;
}
