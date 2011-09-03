#include "flowmove2.h"
#include "math.h"
#include <cstdlib>
#include <time.h>

flowmove2::flowmove2(Space *sS, double sdt, double sRemoveEps)
{
	S = sS;
	dt = S->dt = sdt;
	RemoveEps = sRemoveEps;
	CleanedV_ = 0;
}

void flowmove2::MovePart1()
{
	CleanedV_ = 0;

	auto vlist = S->VortexList;

	//MOVING VORTEXES
	if ( vlist )
	const_for (vlist, lobj)
	{
		*lobj += lobj->v*dt; lobj->v = -lobj->v;
		CheckObject(lobj, true);
	}
}

void flowmove2::MovePart2()
{
	auto vlist = S->VortexList;
	double dt2 = dt*0.5;

	//MOVING VORTEXES
	if ( vlist )
	const_for (vlist, lobj)
	{
		*lobj += lobj->v*dt2; lobj->v.zero();
		CheckObject(lobj, true);
	}
}

void flowmove2::VortexShed()
{
	auto vlist = S->VortexList;
	if (!vlist) return;
	TObj ObjCopy(0, 0, 0);

	const_for(S->BodyList, llbody)
	{
		TBody &body = **llbody;
		const_for(body.List, lbobj)
		{
			TAtt *latt = body.att(lbobj);
			if (fabs(lbobj->g) < RemoveEps)
				{ CleanedV_++; body.g_dead+= lbobj->g; }
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

void flowmove2::CheckObject(TObj*& lobj, bool remove)
{
	auto vlist = S->VortexList;
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
		invalid_inbody->body->g_dead += lobj->g;
		CleanedV_++;
		vlist->erase(lobj);
		lobj--;
	} else
	if ( fabs(lobj->g) < RemoveEps )
	{
		//remove merged vortexes
		vlist->erase(lobj);
		lobj--;
	}
}
