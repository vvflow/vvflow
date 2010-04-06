#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <time.h>
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 // = 2*PI

/********************* HEADER ****************************/

namespace {

Space *FlowMove_S;
double FlowMove_dt;
double FlowMove_dt_2PI;
double FlowMove_RemoveEps;
double FlowMove_minusRemoveEps; // = -RemoveEps

double FlowMove_Fx, FlowMove_Fy; //Forces

int FlowMove_CleanedV_inbody, FlowMove_CleanedV_toosmall;
int FlowMove_CleanedH;

double FlowMove_dfi;
double FlowMove_ControlLayerHeight; //height of boundaru layer(not square)
//TVortex **blarray;

} //end of namespace

/********************* SOURCE *****************************/

int InitFlowMove(Space *sS, double sdt, double sRemoveEps/*, Statistic *sstatistic, double sblheight, int scountfi*/)
{
	srand (time(NULL));
	FlowMove_S = sS;
	FlowMove_dt = sdt;
	FlowMove_dt_2PI = sdt*M_1_2PI;
	FlowMove_RemoveEps = sRemoveEps;
	FlowMove_minusRemoveEps = -sRemoveEps;
	if (sS->BodyList )//&& sS->HeatList) 
		FlowMove_ControlLayerHeight = M_2PI/sS->BodyList->size;
	else
		FlowMove_ControlLayerHeight=0;

	if (sS->BodyList)
		FlowMove_dfi = M_2PI/sS->BodyList->size;
	else
		FlowMove_dfi = 0;

	//printf("%lf\n", FlowMove_ControlLayerHeight);
	//statistic = sstatistic;
	//blheight = sblheight;
	//countfi = scountfi;
	//blarray = new TVortex*[countfi];

	return 0;
}

int MoveAndClean(bool remove)
{
	int i, j, lsize;
	TVortex *Obj;
	TList *list;

	FlowMove_CleanedV_inbody = FlowMove_CleanedV_toosmall = 0;
	FlowMove_CleanedH = 0;

	//for (i=0; i<countfi; i++) { blarray[i] = NULL; }

	//cleaning control layer
	if ( FlowMove_S->BodyList )
	{
		int *field = FlowMove_S->BodyControlLayer;
		lsize = FlowMove_S->BodyList->size;
		for ( i=0; i<lsize; i++ )
		{
			*field = 0;
			field++;
		}
	}

	//MOVING VORTEXES
	if ( FlowMove_S->VortexList )
	{
		list = FlowMove_S->VortexList;
		lsize = list->size;
		Obj = list->Elements;
		for ( i=0; i<lsize; i++)
		{
			Obj->rx += Obj->vx*FlowMove_dt_2PI; Obj->vx = 0;
			Obj->ry += Obj->vy*FlowMove_dt_2PI; Obj->vy = 0;
			
			bool inbody = remove  && ( FlowMove_S->BodyList && ((Obj->rx*Obj->rx + Obj->ry*Obj->ry) < 1) );
			bool toosmall = ( (Obj->g < FlowMove_RemoveEps) && (Obj->g > FlowMove_minusRemoveEps) );
			if ( inbody )
			{
				FlowMove_S->ForceX += Obj->ry* Obj->g;
				FlowMove_S->ForceY -= Obj->rx* Obj->g;
				FlowMove_CleanedV_inbody++;
				list->Remove(Obj);
			} else
			if ( toosmall )
			{
				FlowMove_S->ForceX += Obj->ry* Obj->g;
				FlowMove_S->ForceY -= Obj->rx* Obj->g;
				FlowMove_CleanedV_toosmall++;
				list->Remove(Obj);
			}
			else { Obj++; }
		}
	}

	//MOVING HEAT PARTICLES
	if ( FlowMove_S->HeatList )
	{
		list = FlowMove_S->HeatList;
		lsize = list->size;
		Obj = list->Elements;

		double compare = (1 + FlowMove_ControlLayerHeight); compare*= compare;
		for ( i=0; i<lsize; i++)
		{
			Obj->rx += Obj->vx*FlowMove_dt_2PI; Obj->vx = 0;
			Obj->ry += Obj->vy*FlowMove_dt_2PI; Obj->vy = 0;
			double rabs2 = Obj->rx*Obj->rx + Obj->ry*Obj->ry;
			if ( remove && (rabs2 < 1) )
			{
				FlowMove_CleanedH++;
				list->Remove(Obj);
			} else if (remove && FlowMove_S->BodyControlLayer && (rabs2 < compare) )
			{
				FlowMove_CleanedH++;
				double fi = atan2(Obj->ry, Obj->rx); if (fi < 0) fi+= M_2PI;
				int field = floor(fi/FlowMove_dfi);
				FlowMove_S->BodyControlLayer[field]++;
				list->Remove(Obj);
			}
			else { Obj++; }
		}
	}

	//MoveList(VortexList,  FlowMove_CleanedV)
	//MoveList(HeatList, FlowMove_CleanedH)

	if ( FlowMove_S->RotationV) FlowMove_S->Angle+= FlowMove_S->RotationV(FlowMove_S->Time)*FlowMove_dt;
	if ( FlowMove_S->InfSpeedY) FlowMove_S->BodyY-= FlowMove_S->InfSpeedY(FlowMove_S->Time)*FlowMove_dt;
	//FlowMove_S->Time+= FlowMove_dt;

	return 0;
}

int Move()
{
	int i, j, lsize;
	TVortex *Vort;
	TList *list;

	#define MoveList(List, ListType) 										\
		if ( FlowMove_S->List ) 											\
		{ 																	\
			list = FlowMove_S->List; 										\
			lsize = list->size; 											\
			Vort = list->Elements; 											\
			for ( i=0; i<lsize; i++) 										\
			{ 																\
				Vort->rx += Vort->vx*FlowMove_dt_2PI; Vort->vx = 0; 			\
				Vort->ry += Vort->vy*FlowMove_dt_2PI; Vort->vy = 0; 			\
				Vort++; 													\
			} 																\
		}

	MoveList(VortexList,  FlowMove_CleanedV)
	MoveList(HeatList, FlowMove_CleanedH)

	if ( FlowMove_S->RotationV) FlowMove_S->Angle+= FlowMove_S->RotationV(FlowMove_S->Time)*FlowMove_dt;
	if ( FlowMove_S->InfSpeedY) FlowMove_S->BodyY-= FlowMove_S->InfSpeedY(FlowMove_S->Time)*FlowMove_dt;
	//FlowMove_S->Time+= FlowMove_dt;

	#undef MoveList
	return 0;
}

int Clean()
{
	int i, j, lsize;
	TVortex *Obj;
	TList *list;

	FlowMove_CleanedV_inbody = FlowMove_CleanedV_toosmall = 0;
	FlowMove_CleanedH = 0;

	//cleaning control layer
	if ( FlowMove_S->BodyList )
	{
		int *field = FlowMove_S->BodyControlLayer;
		lsize = FlowMove_S->BodyList->size;
		for ( i=0; i<lsize; i++ )
		{
			*field = 0;
			field++;
		}
	}

	//CLEANING VORTEXES
	if ( FlowMove_S->VortexList )
	{
		list = FlowMove_S->VortexList;
		lsize = list->size;
		Obj = list->Elements;
		for ( i=0; i<lsize; i++)
		{
			bool inbody = ( FlowMove_S->BodyList && ((Obj->rx*Obj->rx + Obj->ry*Obj->ry) < 1) );
			bool toosmall = ( (Obj->g < FlowMove_RemoveEps) && (Obj->g > FlowMove_minusRemoveEps) );
			if ( inbody )
			{
				FlowMove_CleanedV_inbody++;
				list->Remove(Obj);
			} else
			if ( toosmall )
			{
				FlowMove_CleanedV_toosmall++;
				list->Remove(Obj);
			}
			else { Obj++; }
		}
	}

	//CLEANING HEAT PARTICLES
	if ( FlowMove_S->HeatList )
	{
		list = FlowMove_S->HeatList;
		lsize = list->size;
		Obj = list->Elements;

		double compare = (1 + FlowMove_ControlLayerHeight); compare*= compare;
		for ( i=0; i<lsize; i++)
		{
			double rabs2 = Obj->rx*Obj->rx + Obj->ry*Obj->ry;
			if ( FlowMove_S->BodyList && (rabs2 < 1) )
			{
				FlowMove_CleanedH++;
				list->Remove(Obj);
			} else if (FlowMove_S->BodyControlLayer && (rabs2 < compare) )
			{
				FlowMove_CleanedH++;
				double fi = atan2(Obj->ry, Obj->rx); if (fi < 0) fi+= M_2PI;
				int field = floor(fi/FlowMove_dfi);
				FlowMove_S->BodyControlLayer[field]++;
				list->Remove(Obj);
			}
			else { Obj++; }
		}
	}

	return 0;
}

int VortexShed()
{
	if ( !FlowMove_S->BodyList ) return -1;

	int i, bsize;
	TVortex *Vort;
	TVortex VortCopy = *Vort; ZeroVortex(VortCopy);
	TList *blist = FlowMove_S->BodyList;
	TList *vlist;

/*	double x,g;
	TVortex *HVort;
	TVortex *Vort;
	TVortex *BVort; */

	double RiseHeight = 1.+FlowMove_dfi*1E-6;

	FlowMove_CleanedV_inbody = FlowMove_CleanedV_toosmall = 0;
	if ( FlowMove_S->VortexList )
	{
		vlist = FlowMove_S->VortexList;
		bsize = blist->size;
		//double gsumm = FlowMove_S->gsumm();
		//double CirculationAddition = -gsumm/bsize;
		//FlowMove_S->Angle+= M_2PI*gsumm*FlowMove_dt;
		Vort = blist->Elements;
		for ( i=0; i<bsize; i++)
		{
			if ( (Vort->g < FlowMove_RemoveEps) && (Vort->g > FlowMove_minusRemoveEps) ) 
				{ FlowMove_CleanedV_toosmall++; }
			else
			{
				
/*
				double r = 1. + double(rand())*FlowMove_ControlLayerHeight/RAND_MAX;
				double fi= FlowMove_dfi * (i + double(rand())/RAND_MAX); // don't use += here, cuz it causes systematic error;
				VortCopy.rx = r*cos(fi);
				VortCopy.ry = r*sin(fi);
*/
				VortCopy.rx = Vort->rx*RiseHeight;
				VortCopy.ry = Vort->ry*RiseHeight;
				VortCopy.g = Vort->g;// + CirculationAddition;
				FlowMove_S->ForceX -= VortCopy.ry* VortCopy.g;
				FlowMove_S->ForceY += VortCopy.rx* VortCopy.g;
				vlist->Add(VortCopy);
			}

			Vort++;
		}
	}
	
	FlowMove_S->Time+= FlowMove_dt;
	
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
	if ( !FlowMove_S->BodyList ) return -1;

	int BodyListSize = FlowMove_S->BodyList->size;
	double dfi = M_2PI/BodyListSize;
	TVortex Vort; ZeroVortex(Vort); Vort.g = dfi * dfi;

	for ( int i=0; i<BodyListSize; i++ )
	{
		double r = 1. + double(rand())*FlowMove_ControlLayerHeight/RAND_MAX;
		double fi= dfi * (i + double(rand())/RAND_MAX); // don't use += here, cuz it causes systematic error;
		Vort.rx = r*cos(fi);
		Vort.ry = r*sin(fi);
		FlowMove_S->HeatList->Copy(&Vort);
	}

	return 0;
}

int CleanedV_inbody() { return FlowMove_CleanedV_inbody; }
int CleanedV_toosmall() { return FlowMove_CleanedV_toosmall; }
int CleanedH() { return FlowMove_CleanedH; }
