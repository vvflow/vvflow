#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <time.h>

/********************* HEADER ****************************/

namespace {

Space *FlowMove_S;
double FlowMove_dt;
double FlowMove_RemoveEps;

int FlowMove_CleanedV;
int FlowMove_CleanedH;

double FlowMove_ControlLayerHeight; //height of boundaru layer(not square)
//TObject **blarray;

} //end of namespace

/********************* SOURCE *****************************/

int InitFlowMove(Space *sS, double sdt, double sRemoveEps)
{
	srand (time(NULL));
	FlowMove_S = sS;
	FlowMove_dt = FlowMove_S->dt = sdt;
	FlowMove_RemoveEps = sRemoveEps;
	FlowMove_ControlLayerHeight = (sS->Body) ? sS->Body->AverageSegmentLength() : 0;
	FlowMove_CleanedV = 0;

	return 0;
}

int MoveAndClean(bool remove)
{
	FlowMove_CleanedV = 0;
	FlowMove_CleanedH = 0;

	//MOVING VORTEXES
	if ( FlowMove_S->VortexList )
	{
		auto vlist = FlowMove_S->VortexList;

		for (auto Obj = vlist->begin(); Obj<vlist->end(); Obj++)
		{
			*Obj += Obj->v*FlowMove_dt; Obj->v.zero();

			bool inbody = remove && !FlowMove_S->Body->PointIsValid(*Obj);
			bool toosmall = ( fabs(Obj->g) < FlowMove_RemoveEps );
			if ( inbody || toosmall)
			{
				FlowMove_S->Body->Force -= rotl(*Obj) * Obj->g;
				FlowMove_CleanedV++;
				vlist->erase(Obj);
				Obj--;
			}
		}
	}

	//MOVING HEAT PARTICLES
	if ( FlowMove_S->HeatList )
	{
		FlowMove_S->Body->CleanHeatLayer();

		auto hlist = FlowMove_S->HeatList;

//		double compare = (1 + FlowMove_ControlLayerHeight); compare*= compare;
		for (auto Obj = hlist->begin(); Obj<hlist->end(); Obj++)
		{
			*Obj += Obj->v*FlowMove_dt; Obj->v.zero();

			bool inbody = remove  && FlowMove_S->Body->PointIsValid(*Obj);
			int* inlayer = FlowMove_S->Body->ObjectIsInHeatLayer(*Obj);
			if ( inbody )
			{
				FlowMove_CleanedH++;
				if (inlayer) { (*inlayer)++; }
				hlist->erase(Obj);
				Obj--;
			}
		}
	}

	return 0;
}

int VortexShed()
{
	if ( !FlowMove_S->Body || !FlowMove_S->VortexList ) return -1;
	//double RiseHeight = FlowMove_S->Body->HeatLayerHeight*1E-6;
	TObj ObjCopy(0, 0, 0);

	//FlowMove_CleanedV = 0;

	auto vlist = FlowMove_S->VortexList;
	auto blist = FlowMove_S->Body->List;
	for (auto bvort = blist->begin(); bvort<blist->end(); bvort++)
	{
		if ( (bvort->g < FlowMove_RemoveEps) && (bvort->g > -FlowMove_RemoveEps) ) 
			{ FlowMove_CleanedV++; }
		else
		{
			ObjCopy = *bvort;
			FlowMove_S->Body->Force += rotl(ObjCopy) * ObjCopy.g;
			vlist->push_back(ObjCopy);
		}
	}

	return 0;
}

int HeatShed()
{
	if ( !FlowMove_S->Body || !FlowMove_S->HeatList) return -1;
/*
	long BodyListSize = FlowMove_S->Body->List->size;
	//double dfi = C_2PI/BodyListSize;
	TObject ObjCopy; ZeroVortex(ObjCopy); //ObjCopy.g = dfi * dfi;

	for ( long i=0; i<BodyListSize; i++ )
	{
		double r = 1. + double(rand())*FlowMove_ControlLayerHeight/RAND_MAX;
		double fi= dfi * (i + double(rand())/RAND_MAX);
		ObjCopy.rx = r*cos(fi);
		ObjCopy.ry = r*sin(fi);
		FlowMove_S->HeatList->Copy(&ObjCopy);
	}
*/
	return 0;
}

int CleanedV() { return FlowMove_CleanedV; }
int CleanedH() { return FlowMove_CleanedH; }
