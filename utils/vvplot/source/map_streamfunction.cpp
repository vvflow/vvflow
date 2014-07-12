#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include <time.h>

#include "libvvplot_api.h"
#include "core.h"
#include "hdf5.h"

static double Rd2;
static TVec RefFrame_Speed;
inline static double atan(TVec p) {return atan(p.y/p.x);}

double Psi(Space* S, TVec p)
{
	double psi1(0), psi2(0), psi3(0), psi4(0);

	const_for(S->BodyList, llbody)
	{
		#define b (**llbody)

		double psi_g_tmp=0, psi_q_tmp=0;

		//fprintf(stderr, "body %lf\n", b.RotationSpeed_slae);
		if (!b.Speed_slae.iszero())
		const_for(b.List, latt)
		{
			TVec Vs = b.Speed_slae.r + b.Speed_slae.o * rotl(latt->r - (b.pos.r + b.dPos.r));
			double g = -Vs * latt->dl;
			double q = -rotl(Vs) * latt->dl;
			psi_g_tmp+= log((p-latt->r).abs2() + Rd2) * g;
			psi_q_tmp+= atan(p-latt->r) * q;
			//fprintf(stderr, "attach %lf\n", b.RotationSpeed_slae);
		}
		//psi1-= (psi_g_tmp*0.5 + psi_q_tmp) * C_1_2PI;

		const_for(b.List, latt)
		{
			psi2 += log((p-latt->r).abs2() + Rd2) * latt->g;
		}

		#undef b
	}
	psi2*= -0.5*C_1_2PI;

	TSortedNode* Node = S->Tree->findNode(p);
	if (!Node) return 0;
	const_for (Node->FarNodes, llfnode)
	{
		TObj obj = (**llfnode).CMp;
		psi3+= log((p-obj.r).abs2() + Rd2)*obj.g;
		obj = (**llfnode).CMm;
		psi3+= log((p-obj.r).abs2() + Rd2)*obj.g;
	}
	const_for (Node->NearNodes, llnnode)
	{
		for (TObj *lobj = (**llnnode).vRange.first; lobj < (**llnnode).vRange.last; lobj++)
		{
			psi3+= log((p-lobj->r).abs2() + Rd2) * lobj->g;
		}
	}
	psi3*= -0.5*C_1_2PI;

	return psi1 + psi2 + psi3 + p*rotl(S->InfSpeed() - RefFrame_Speed);
}

extern "C" {
int map_streamfunction(hid_t fid, char RefFrame, double xmin, double xmax, double ymin, double ymax, double spacing)
{
	Space *S = new Space();
	S->Load(fid);

	double dl = S->AverageSegmentLength(); Rd2 = dl*dl/25;
	S->Tree = new TSortedTree(S, 8, dl*20, 0.3);

	/**************************** LOAD ARGUMENTS ******************************/
	switch (RefFrame)
	{
		case 'o': RefFrame_Speed = TVec(0, 0); break;
		case 'f': RefFrame_Speed = S->InfSpeed(); break;
		case 'b': RefFrame_Speed = S->BodyList->at(0)->Speed_slae.r; break;
		default:
		fprintf(stderr, "Bad reference frame\n");
		fprintf(stderr, "Available options are:\n");
		fprintf(stderr, " 'o' : original reference frame\n" );
		fprintf(stderr, " 'f' : fluid reference frame\n" );
		fprintf(stderr, " 'b' : body reference frame\n" );
	}


	S->Tree->build();
	hsize_t dims[2] = {(xmax-xmin)/spacing + 1, (ymax-ymin)/spacing + 1};
	float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);

	for (int xi=0; xi<dims[0]; xi++)
	{
		double x = xmin + double(xi)*spacing;
		#pragma omp parallel for ordered schedule(dynamic, 10)
		for (int yj=0; yj<dims[1]; yj++)
		{
			double y = ymin + double(yj)*spacing;
			mem[xi*dims[1]+yj] = Psi(S, TVec(x, y));
		}
	}

	char map_name[] = "map_streamfunction.?";
	map_name[19] = RefFrame;
	map_save(fid, map_name, mem, dims, xmin, xmax, ymin, ymax, spacing);
	free(mem);

	return 0;
}}
