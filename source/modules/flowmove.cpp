#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <time.h>

flowmove::flowmove(Space *sS, double sdt, double sRemoveEps)
{
	S = sS;
	dt = S->dt = sdt;
	RemoveEps = sRemoveEps;
	CleanedV_ = 0;
}

void flowmove::MoveAndClean(bool remove, bool zero_speed)
{
	CleanedV_ = 0;
	auto vlist = S->VortexList;
	auto hlist = S->HeatList;

	//MOVING VORTEXES
	if ( vlist )
	const_for (vlist, lobj)
	{
		*lobj += lobj->v*dt; if(zero_speed) lobj->v.zero();

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			if (!invalid_inbody)
			invalid_inbody = (**llbody).PointIsInvalid(*lobj);
		}

		if ( remove && invalid_inbody )
		{
			TBody *badbody = invalid_inbody->body;
			badbody->Force -= rotl(*lobj) * lobj->g;
			badbody->Force.g -=  lobj->abs2() * lobj->g;
			invalid_inbody->gsum -= lobj->g;
			badbody->g_dead += lobj->g;
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

	//MOVING HEAT PARTICLES
	if ( hlist )
	const_for (hlist, lobj)
	{
		*lobj += lobj->v*dt; if(zero_speed) lobj->v.zero();

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			invalid_inbody = (**llbody).PointIsInvalid(*lobj);
			if (invalid_inbody) break;
		}

		if ( invalid_inbody )
		{
			invalid_inbody->heat -= lobj->g;
			hlist->erase(lobj);
			lobj--; continue;
		}

		TAtt* inlayer = NULL;
		const_for(S->BodyList, llbody)
		{
			inlayer = (**llbody).PointIsInHeatLayer(*lobj);
			if (inlayer) break;
		}

		if ( inlayer )
		switch (inlayer->hc)
		{
			case hc::const_t:
				if (inlayer->ParticleInHeatLayer >= 0)
				{
					inlayer->heat -= lobj->g;
					hlist->erase(lobj);
					lobj--; continue;
				} else
				{
					inlayer->ParticleInHeatLayer = hlist->find(lobj);
				}
				break;
		}
	}

	//MOVING Streak PARTICLES
	if ( S->StreakList )
	const_for (S->StreakList, lobj)
	{
		*lobj += lobj->v*dt; lobj->v.zero();

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			if ((**llbody).PointIsInvalid(*lobj))
			{
				S->StreakList->erase(lobj);
				lobj--;
				break;
			}
		}
	}
}

void flowmove::VortexShed()
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
				body.Force.g += ObjCopy.abs2() * ObjCopy.g;
				          latt ->gsum+= 0.5*ObjCopy.g;
				body.prev(latt)->gsum+= 0.5*ObjCopy.g;
				vlist->push_back(ObjCopy);
			}
		}
	}
}

void flowmove::HeatShed()
{
	auto hlist = S->HeatList;
	if (!hlist) return;
	TObj ObjCopy(0, 0, 0);

	const_for(S->BodyList, llbody)
	{
		#define body (**llbody)
		const_for(body.AttachList, latt)
		{
			switch (latt->hc)
			{
				case hc::isolate:
					if (latt->heat)
					{
						hlist->push_back(TObj(*latt+rotl(latt->dl)*0.5, -latt->heat));
						latt->heat = 0;
					}
					break;
				case hc::const_t:
				{
					double tmp_g(latt->dl.abs2() * latt->heat_const);
					TObj *tmp_obj = (latt->ParticleInHeatLayer>=0)? hlist->begin()+latt->ParticleInHeatLayer : NULL;
					if (tmp_obj)
					{
						latt->heat += tmp_g - tmp_obj->g;
						tmp_obj->g = tmp_g;
					} else
					{
						latt->heat += tmp_g;
						hlist->push_back(TObj(*latt+rotl(latt->dl)*0.5, tmp_g));
					}
				}
					break;
				case hc::const_W:
				{
					double tmp_g(latt->dl.abs2() * latt->heat_const);
					latt->heat += tmp_g;
					hlist->push_back(TObj(*latt+rotl(latt->dl)*0.5, tmp_g));
				}
					break;
			}
		}
		#undef body
	}
}

void flowmove::CropHeat()
{
	TVec c(0,0), r(0,0);
	int N=0;
	const_for(S->BodyList, llbody)
	const_for((**llbody).List, lobj)
	{
		c+= *lobj;
		N++;
	}
	c/= N;

	const_for(S->BodyList, llbody)
	const_for((**llbody).List, lobj)
	{
		r.rx = max(r.rx, (lobj->rx - c.rx));
		r.ry = max(r.ry, (lobj->ry - c.ry));
	}
	r *= 5;

	TVec bl(c+r), tr(c-r);
	const_for(S->HeatList, lobj)
	{
		if ((lobj->rx > tr.rx) || (lobj->rx < bl.rx) ||
		    (lobj->ry > tr.ry) || (lobj->ry < bl.ry))
			S->HeatList->erase(lobj);
	}
}

void flowmove::StreakShed()
{
	const_for(S->StreakSourceList, lobj)
	{
		S->StreakList->push_back(*lobj);
	}
}

