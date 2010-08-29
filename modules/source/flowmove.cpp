#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <time.h>

/********************* HEADER ****************************/

namespace {

Space *FlowMove_S;
double FlowMove_dt;
double FlowMove_RemoveEps;

int FlowMove_CleanedV_inbody, FlowMove_CleanedV_toosmall;
int FlowMove_CleanedH;

double FlowMove_dfi;
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
	FlowMove_ControlLayerHeight = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;

	FlowMove_dfi = (sS->BodyList) ? C_2PI/sS->BodyList->size : 0;

	return 0;
}

int MoveAndClean(bool remove)
{
	FlowMove_CleanedV_inbody = FlowMove_CleanedV_toosmall = 0;
	FlowMove_CleanedH = 0;

	//cleaning control layer
	if ( FlowMove_S->BodyList )
	{
		int *field = FlowMove_S->BodyControlLayer;
		long lsize = FlowMove_S->BodyList->size;
		for ( long i=0; i<lsize; i++ )
		{
			*field = 0;
			field++;
		}
	}

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
			
			bool inbody = remove  && ( FlowMove_S->BodyList && ((Obj->rx*Obj->rx + Obj->ry*Obj->ry) < 1.) );
			bool toosmall = ( (Obj->g < FlowMove_RemoveEps) && (Obj->g > -FlowMove_RemoveEps) );
			if ( inbody )
			{
				FlowMove_S->ForceX += Obj->ry* Obj->g;
				FlowMove_S->ForceY -= Obj->rx* Obj->g;
				FlowMove_CleanedV_inbody++;
				VList.Remove(Obj);
				Obj--;
			} else
			if ( toosmall )
			{
				FlowMove_S->ForceX += Obj->ry* Obj->g;
				FlowMove_S->ForceY -= Obj->rx* Obj->g;
				FlowMove_CleanedV_toosmall++;
				VList.Remove(Obj);
				Obj--;
			}
		}
	}

	//MOVING HEAT PARTICLES
	if ( FlowMove_S->HeatList )
	{
		TList<TObject> &HList = *FlowMove_S->HeatList;
		TObject *Obj = HList.First;
		TObject *&LastObj = HList.Last;
		double compare = (1 + FlowMove_ControlLayerHeight); compare*= compare;
		for ( ; Obj<LastObj; Obj++)
		{
			Obj->rx += Obj->vx*FlowMove_dt; Obj->vx = 0;
			Obj->ry += Obj->vy*FlowMove_dt; Obj->vy = 0;
			double rabs2 = Obj->rx*Obj->rx + Obj->ry*Obj->ry;
			if ( remove && (rabs2 < 1) )
			{
				FlowMove_CleanedH++;
				HList.Remove(Obj);
				Obj--;
			} else if (remove && FlowMove_S->BodyControlLayer && (rabs2 < compare) )
			{
				FlowMove_CleanedH++;
				double fi = atan2(Obj->ry, Obj->rx); if (fi < 0) fi+= C_2PI;
				int field = floor(fi/FlowMove_dfi);
				FlowMove_S->BodyControlLayer[field]++;
				HList.Remove(Obj);
				Obj--;
			}
		}
	}

	return 0;
}

int VortexShed()
{
	if ( !FlowMove_S->BodyList || !FlowMove_S->VortexList ) return -1;
	double RiseHeight = 1.+FlowMove_dfi*1E-6;
	TVortex VortCopy; ZeroObject(VortCopy);

	FlowMove_CleanedV_inbody = FlowMove_CleanedV_toosmall = 0;

	TObject *BVort = FlowMove_S->BodyList->First;
	TObject *&LastBVort = FlowMove_S->BodyList->Last;
	TList<TVortex> &VList = *FlowMove_S->VortexList;
	for ( ; BVort<LastBVort; BVort++)
	{
		if ( (BVort->g < FlowMove_RemoveEps) && (BVort->g > -FlowMove_RemoveEps) ) 
			{ FlowMove_CleanedV_toosmall++; }
		else
		{
			VortCopy.rx = BVort->rx*RiseHeight;
			VortCopy.ry = BVort->ry*RiseHeight;
			VortCopy.g = BVort->g;
			FlowMove_S->ForceX -= VortCopy.ry* VortCopy.g;
			FlowMove_S->ForceY += VortCopy.rx* VortCopy.g;
			VList.Add(VortCopy);
		}
	}

	return 0;
}

int HeatShed()
{
	if ( !FlowMove_S->BodyList || !FlowMove_S->HeatList) return -1;

	long BodyListSize = FlowMove_S->BodyList->size;
	double dfi = C_2PI/BodyListSize;
	TObject ObjCopy; ZeroVortex(ObjCopy); ObjCopy.g = dfi * dfi;

	for ( long i=0; i<BodyListSize; i++ )
	{
		double r = 1. + double(rand())*FlowMove_ControlLayerHeight/RAND_MAX;
		double fi= dfi * (i + double(rand())/RAND_MAX);
		ObjCopy.rx = r*cos(fi);
		ObjCopy.ry = r*sin(fi);
		FlowMove_S->HeatList->Copy(&ObjCopy);
	}

	return 0;
}

int CleanedV_inbody() { return FlowMove_CleanedV_inbody; }
int CleanedV_toosmall() { return FlowMove_CleanedV_toosmall; }
int CleanedH() { return FlowMove_CleanedH; }
