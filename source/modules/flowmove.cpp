#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <time.h>

/********************************** HEADER ************************************/
namespace {

Space *S;
double dt;
double RemoveEps;

int CleanedV_;
int CleanedH_;

double FlowMove_ControlLayerHeight; //height of boundaru layer(not square)

}

/****************************** MAIN FUNCTIONS ********************************/

void InitFlowMove(Space *sS, double sdt, double sRemoveEps)
{
	S = sS;
	dt = S->dt = sdt;
	RemoveEps = sRemoveEps;
	CleanedV_ = CleanedH_ = 0;
}

void MoveAndClean(bool remove)
{
	if (!S) {cerr << "MoveAndClean() is called before initialization"
	              << endl; return; }

	CleanedV_ = CleanedH_ = 0;

	auto vlist = S->VortexList;
	auto hlist = S->HeatList;

	//MOVING VORTEXES
	if ( vlist )
	const_for (vlist, lobj)
	{
		*lobj += lobj->v*dt; lobj->v.zero();

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			if (!invalid_inbody)
			invalid_inbody = (**llbody).PointIsInvalid(*lobj);
		}

		if ( remove && invalid_inbody )
		{
			invalid_inbody->body->Force -= rotl(*lobj) * lobj->g;
			invalid_inbody->gsum -= lobj->g;
			CleanedV_++;
			vlist->erase(lobj);
			lobj--;
		} else
		if ( fabs(lobj->g) < RemoveEps )
		{
			//FlowMove_CleanedV++;
			vlist->erase(lobj);
			lobj--;
		}
	}

	//MOVING HEAT PARTICLES
	if ( hlist )
	{
		const_for(S->BodyList, llbody)
		{
			(**llbody).CleanHeatLayer();
		}

		const_for (hlist, lobj)
		{
			*lobj += lobj->v*dt; lobj->v.zero();

			TAtt* invalid_inbody;
			int* inlayer;
			const_for(S->BodyList, llbody)
			{
				invalid_inbody = (**llbody).PointIsInvalid(*lobj);
				inlayer = (**llbody).ObjectIsInHeatLayer(*lobj);
			}
			if ( remove && invalid_inbody )
			{
				CleanedH_++;
				hlist->erase(lobj);
				lobj--;
			} else
			if (inlayer) { (*inlayer)++; }
		}
	}
}

void VortexShed()
{
	if (!S) {cerr << "VortexShed() is called before initialization"
	              << endl; return; }
	if (!S->VortexList) return;
	TObj ObjCopy(0, 0, 0);

	//FlowMove_CleanedV = 0;

	auto vlist = S->VortexList;

	const_for(S->BodyList, llbody)
	{
		TBody &body = **llbody;
		const_for(body.List, lbobj)
		{
			TAtt *latt = body.att(lbobj);
			if (fabs(lbobj->g) < RemoveEps)
				{ CleanedV_++; }
			else if ( (latt->bc == bc::noslip)
			       || ((**llbody).prev(latt)->bc == bc::noslip) )
			{
				ObjCopy = *lbobj;
				body.Force += rotl(ObjCopy) * ObjCopy.g;
				          latt ->gsum+= 0.5*ObjCopy.g;
				body.prev(latt)->gsum+= 0.5*ObjCopy.g;
				vlist->push_back(ObjCopy);
			}
		}
	}
}

void HeatShed()
{
	if (!S) {cerr << "HeatShed() is called before initialization"
	              << endl; return; }
	if (!S->HeatList) return;
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
}

int CleanedV() { return CleanedV_; }
int CleanedH() { return CleanedH_; }
