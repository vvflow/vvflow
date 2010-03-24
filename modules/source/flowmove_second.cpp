#include "flowmove_second.h"
#include "math.h"
#include <cstdlib>
#include <time.h>
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 // = 2*PI

/********************* HEADER ****************************/

namespace {

Space *FlowMove2_S;
double FlowMove2_dt;
double FlowMove2_dt_2PI;
double FlowMove2_dt_4PI;
double FlowMove2_RemoveEps;
double FlowMove2_minusRemoveEps; // = -RemoveEps

double FlowMove2_Fx, FlowMove_Fy; //Forces

int FlowMove2_CleanedV_inbody, FlowMove2_CleanedV_toosmall;
int FlowMove2_CleanedH;

double FlowMove2_dfi;
double FlowMove2_ControlLayerHeight; //height of boundaru layer(not square)
//TVortex **blarray;

} //end of namespace

/********************* SOURCE *****************************/

int InitFlowMove_Second(Space *sS, double sdt, double sRemoveEps)
{
	srand (time(NULL));
	FlowMove2_S = sS;
	FlowMove2_dt = sdt;
	FlowMove2_dt_2PI = sdt*M_1_2PI;
	FlowMove2_dt_4PI = FlowMove2_dt_2PI*0.5;
	FlowMove2_RemoveEps = sRemoveEps;
	FlowMove2_minusRemoveEps = -sRemoveEps;
	if (sS->BodyList )//&& sS->HeatList) 
		FlowMove2_ControlLayerHeight = M_2PI/sS->BodyList->size;
	else
		FlowMove2_ControlLayerHeight=0;

	if (sS->BodyList)
		FlowMove2_dfi = M_2PI/sS->BodyList->size;
	else
		FlowMove2_dfi = 0;

	return 0;
}

int Move1()
{
	int i, j, lsize;
	TVortex *Vort;
	TList *list;
	
	#define MoveList(List, ListType) 										\
		if ( FlowMove2_S->List ) 											\
		{ 																	\
			list = FlowMove2_S->List; 										\
			lsize = list->size; 											\
			Vort = list->Elements; 											\
			for ( i=0; i<lsize; i++) 										\
			{ 																\
				Vort->rx += Vort->vx*FlowMove2_dt_2PI; Vort->vxtmp = Vort->vx; Vort->vx = 0; 			\
				Vort->ry += Vort->vy*FlowMove2_dt_2PI; Vort->vytmp = Vort->vy; Vort->vy = 0; 			\
				Vort++; 													\
			} 																\
		}

	MoveList(VortexList,  FlowMove2_CleanedV)
	MoveList(HeatList, FlowMove2_CleanedH)

	#undef MoveList
	return 0;
}

int Move2()
{
	int i, j, lsize;
	TVortex *Vort;
	TList *list;

	#define MoveList(List, ListType) 										\
		if ( FlowMove2_S->List ) 											\
		{ 																	\
			list = FlowMove2_S->List; 										\
			lsize = list->size; 											\
			Vort = list->Elements; 											\
			for ( i=0; i<lsize; i++) 										\
			{ 																\
				Vort->rx += (Vort->vx-Vort->vxtmp)*FlowMove2_dt_4PI; Vort->vxtmp = Vort->vx = 0; 			\
				Vort->ry += (Vort->vy-Vort->vytmp)*FlowMove2_dt_4PI; Vort->vytmp = Vort->vy = 0; 			\
				Vort++; 													\
			} 																\
		}

	MoveList(VortexList,  FlowMove2_CleanedV)
	MoveList(HeatList, FlowMove2_CleanedH)

	if ( FlowMove2_S->RotationV) FlowMove2_S->Angle+= FlowMove2_S->RotationV(FlowMove2_S->Time)*FlowMove2_dt;
	FlowMove2_S->Time+= FlowMove2_dt;

	#undef MoveList
	return 0;
}

int Clean()
{
	int i, j, lsize;
	TVortex *Obj;
	TList *list;

	FlowMove2_CleanedV_inbody = FlowMove2_CleanedV_toosmall = 0;
	FlowMove2_CleanedH = 0;

	//cleaning control layer
	if ( FlowMove2_S->BodyList )
	{
		int *field = FlowMove2_S->BodyControlLayer;
		lsize = FlowMove2_S->BodyList->size;
		for ( i=0; i<lsize; i++ )
		{
			*field = 0;
			field++;
		}
	}

	//CLEANING VORTEXES
	if ( FlowMove2_S->VortexList )
	{
		list = FlowMove2_S->VortexList;
		lsize = list->size;
		Obj = list->Elements;
		for ( i=0; i<lsize; i++)
		{
			bool inbody = ( FlowMove2_S->BodyList && ((Obj->rx*Obj->rx + Obj->ry*Obj->ry) < 1) );
			bool toosmall = ( (Obj->g < FlowMove2_RemoveEps) && (Obj->g > FlowMove2_minusRemoveEps) );
			if ( inbody )
			{
				FlowMove2_CleanedV_inbody++;
				list->Remove(Obj);
			} else
			if ( toosmall )
			{
				FlowMove2_CleanedV_toosmall++;
				list->Remove(Obj);
			}
			else { Obj++; }
		}
	}

	//CLEANING HEAT PARTICLES
	if ( FlowMove2_S->HeatList )
	{
		list = FlowMove2_S->HeatList;
		lsize = list->size;
		Obj = list->Elements;

		double compare = (1 + FlowMove2_ControlLayerHeight); compare*= compare;
		for ( i=0; i<lsize; i++)
		{
			double rabs2 = Obj->rx*Obj->rx + Obj->ry*Obj->ry;
			if ( FlowMove2_S->BodyList && (rabs2 < 1) )
			{
				FlowMove2_CleanedH++;
				list->Remove(Obj);
			} else if (FlowMove2_S->BodyControlLayer && (rabs2 < compare) )
			{
				FlowMove2_CleanedH++;
				double fi = atan2(Obj->ry, Obj->rx); if (fi < 0) fi+= M_2PI;
				int field = floor(fi/FlowMove2_dfi);
				FlowMove2_S->BodyControlLayer[field]++;
				list->Remove(Obj);
			}
			else { Obj++; }
		}
	}

	return 0;
}

int VortexShed()
{
	if ( !FlowMove2_S->BodyList ) return -1;

	int i, bsize;
	TVortex *Vort;
	TList *blist = FlowMove2_S->BodyList;
	TList *vlist;

/*	double x,g;
	TVortex *HVort;
	TVortex *Vort;
	TVortex *BVort; */

	FlowMove2_CleanedV_inbody = FlowMove2_CleanedV_toosmall = 0;
	if ( FlowMove2_S->VortexList )
	{
		vlist = FlowMove2_S->VortexList;
		bsize = blist->size;
		Vort = blist->Elements;
		for ( i=0; i<bsize; i++)
		{
			if ( (Vort->g < FlowMove2_RemoveEps) && (Vort->g > FlowMove2_minusRemoveEps) ) 
				{ FlowMove2_CleanedV_toosmall++; }
			else
			{
				FlowMove2_S->ForceX -= Vort->ry* Vort->g;
				FlowMove2_S->ForceY += Vort->rx* Vort->g;

				TVortex VortCopy = *Vort;
				double r = 1. + double(rand())*FlowMove2_ControlLayerHeight/RAND_MAX;
				double fi= FlowMove2_dfi * (i + double(rand())/RAND_MAX); // don't use += here, cuz it causes systematic error;
				VortCopy.rx = r*cos(fi);
				VortCopy.ry = r*sin(fi);
				VortCopy.g = Vort->g;
				VortCopy.vx = VortCopy.vy = 0;
				vlist->Add(VortCopy);
			}

			Vort++;
		}
	}
	
	
	
	return 0;
/*
	for (i=0; i<BodyList->size(); i++)
	{
		BVort = (TVortex*)BodyList->Item(i);
		if( fabs(BVort->g) < RemoveEps) { BVort->g =0; cleaned++; continue; }
		Vort = new TVortex(BVort->r, BVort->g, Free);
		VortexField->Add(Vort);
		statistic->BornCount[i/statistic->Step]++;
		statistic->BornSum += Vort->g*rotl(Vort->r);
	}*/
	/*x = 2*M_PI/HeatCount;
	g = dt*x;
	for (i = 0; i< HeatCount; i++)
	{
		HVort = new TVortex(Point( cos(x*i), sin(x*i) ), g, Heat);
		HeatList->Add(HVort);
	}*/
}

int HeatShed()
{
	if ( !FlowMove2_S->BodyList ) return -1;

	int BodyListSize = FlowMove2_S->BodyList->size;
	double dfi = M_2PI/BodyListSize;
	TVortex Vort; ZeroVortex(Vort); Vort.g = dfi * dfi;

	for ( int i=0; i<BodyListSize; i++ )
	{
		double r = 1. + double(rand())*FlowMove2_ControlLayerHeight/RAND_MAX;
		double fi= dfi * (i + double(rand())/RAND_MAX); // don't use += here, cuz it causes systematic error;
		Vort.rx = r*cos(fi);
		Vort.ry = r*sin(fi);
		FlowMove2_S->HeatList->Copy(&Vort);
	}

	return 0;
}

int CleanedV_inbody() { return FlowMove2_CleanedV_inbody; }
int CleanedV_toosmall() { return FlowMove2_CleanedV_toosmall; }
int CleanedH() { return FlowMove2_CleanedH; }
