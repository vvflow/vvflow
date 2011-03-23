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
//TVortex **blarray;

} //end of namespace

/********************* SOURCE *****************************/

int InitFlowMove(Space *sS, double sdt, double sRemoveEps)
{
	srand (time(NULL));
	FlowMove_S = sS;
	FlowMove_dt = FlowMove_S->dt = sdt;
	FlowMove_RemoveEps = sRemoveEps;
	FlowMove_ControlLayerHeight = (sS->Body) ? sS->Body->SurfaceLenght()/(sS->Body->List->size-1) : 0;

	return 0;
}

int MoveAndClean(bool remove)
{
	FlowMove_CleanedV = 0;
	FlowMove_CleanedH = 0;

	//MOVING VORTEXES
	if ( FlowMove_S->VortexList )
	{
		TList<TObject> &VList = *FlowMove_S->VortexList;
		TObject *Obj = VList.First;
		TObject *&LastObj = VList.Last;
		for ( ; Obj<LastObj; Obj++)
		{
			Obj->rx += Obj->vx*FlowMove_dt; Obj->vx = 0;
			Obj->ry += Obj->vy*FlowMove_dt; Obj->vy = 0;
			
			bool inbody = remove  && FlowMove_S->Body->PointIsValid(Obj->rx, Obj->ry);
			bool toosmall = ( (Obj->g < FlowMove_RemoveEps) && (Obj->g > -FlowMove_RemoveEps) );
			if ( inbody || toosmall)
			{
				FlowMove_S->Body->ForceX += Obj->ry* Obj->g;
				FlowMove_S->Body->ForceY -= Obj->rx* Obj->g;
				FlowMove_CleanedV++;
				VList.Remove(Obj);
				Obj--;
			}
		}
	}

	//MOVING HEAT PARTICLES
	if ( FlowMove_S->HeatList )
	{
		FlowMove_S->Body->CleanHeatLayer();

		TList<TObject> &HList = *FlowMove_S->HeatList;
		TObject *Obj = HList.First;
		TObject *&LastObj = HList.Last;
//		double compare = (1 + FlowMove_ControlLayerHeight); compare*= compare;
		for ( ; Obj<LastObj; Obj++)
		{
			Obj->rx += Obj->vx*FlowMove_dt; Obj->vx = 0;
			Obj->ry += Obj->vy*FlowMove_dt; Obj->vy = 0;
			//double rabs2 = Obj->rx*Obj->rx + Obj->ry*Obj->ry;

			bool inbody = remove  && FlowMove_S->Body->PointIsValid(Obj->rx, Obj->ry);
			int* inlayer = FlowMove_S->Body->ObjectIsInHeatLayer(*Obj);
			if ( inbody )
			{
				FlowMove_CleanedH++;
				if (inlayer) { (*inlayer)++; }
				HList.Remove(Obj);
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
	TObject ObjCopy; ZeroObject(ObjCopy);

	FlowMove_CleanedV = 0;

	TObject *BVort = FlowMove_S->Body->List->First;
	TObject *&LastBVort = FlowMove_S->Body->List->Last;
	TList<TVortex> &VList = *FlowMove_S->VortexList;
	for ( ; BVort<LastBVort; BVort++)
	{
		if ( (BVort->g < FlowMove_RemoveEps) && (BVort->g > -FlowMove_RemoveEps) ) 
			{ FlowMove_CleanedV++; }
		else
		{
			ObjCopy.rx = BVort->rx;//*RiseHeight;
			ObjCopy.ry = BVort->ry;//*RiseHeight;
			ObjCopy.g = BVort->g;
			FlowMove_S->Body->ForceX -= ObjCopy.ry* ObjCopy.g;
			FlowMove_S->Body->ForceY += ObjCopy.rx* ObjCopy.g;
			VList.Add(ObjCopy);
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
