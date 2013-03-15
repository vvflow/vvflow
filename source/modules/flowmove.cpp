#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <time.h>

flowmove::flowmove(Space *sS, double sRemoveEps)
{
	S = sS;
	dt = S->dt;
	RemoveEps = sRemoveEps;
	CleanedV_ = 0;
}

void flowmove::MoveAndClean(bool remove, bool zero_speed)
{
	CleanedV_ = 0;
	auto vlist = S->VortexList;
	auto hlist = S->HeatList;

	//Move bodies
	const_for (S->BodyList, llbody)
	{
		(**llbody).doRotationAndMotion();
	}

	S->InfMarker+= S->InfSpeed()*S->dt;

	//MOVING VORTEXES
	if ( vlist )
	const_for (vlist, lobj)
	{
		lobj->r += lobj->v*dt; if(zero_speed) lobj->v = TVec(0., 0.);

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			if (!invalid_inbody)
			invalid_inbody = (**llbody).isPointInvalid(lobj->r);
		}

		if ( remove && invalid_inbody )
		{
			TBody *badbody = invalid_inbody->body;
			badbody->Force_dead.r += rotl(lobj->r) * lobj->g;
			badbody->Force_dead.o +=  (lobj->r - badbody->pos.r - badbody->dPos.r).abs2() * lobj->g;
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

	const_for(S->BodyList, llbody)
	{
		(**llbody).Force_dead.r /= dt;
		(**llbody).Force_dead.o /= 2.*dt;
	}

	//MOVING HEAT PARTICLES
	if ( hlist )
	const_for (hlist, lobj)
	{
		lobj->r += lobj->v*dt; if(zero_speed) lobj->v = TVec(0., 0.);

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			invalid_inbody = (**llbody).isPointInvalid(lobj->r);
			if (invalid_inbody) break;
		}

		if ( invalid_inbody )
		{
			invalid_inbody->hsum -= lobj->g;
			hlist->erase(lobj);
			lobj--; continue;
		}

		TAtt* inlayer = NULL;
		const_for(S->BodyList, llbody)
		{
			inlayer = (**llbody).isPointInHeatLayer(lobj->r);
			if (inlayer) break;
		}

		if ( inlayer )
		switch (inlayer->hc)
		{
			case hc::const_t:
				if (inlayer->ParticleInHeatLayer >= 0)
				{
					inlayer->hsum -= lobj->g;
					hlist->erase(lobj);
					lobj--; continue;
				} else
				{
					inlayer->ParticleInHeatLayer = hlist->find(lobj);
				}
				break;
		} else
		if ( fabs(lobj->g) < RemoveEps )
		{
			//remove merged particles
			hlist->erase(lobj);
			lobj--;
		}
	}

	//MOVING Streak PARTICLES
	if ( S->StreakList )
	const_for (S->StreakList, lobj)
	{
		lobj->r += lobj->v*dt; lobj->v = TVec(0., 0.);

		TAtt* invalid_inbody = NULL;
		const_for(S->BodyList, llbody)
		{
			if ((**llbody).isPointInvalid(lobj->r))
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
		#define body (**llbody)
		body.Force_born = TVec3D(0., 0., 0.);

		const_for(body.List, latt)
		{
			if (fabs(latt->g) < RemoveEps)
				{ CleanedV_++; body.g_dead+= latt->g; }
			else if ( (latt->bc == bc::noslip) || ((latt+1)->bc == bc::noslip) )
			{
				ObjCopy.r = latt->corner + rotl(latt->dl)*1e-4;
				ObjCopy.g = latt->g;
				body.Force_born.r += rotl(latt->corner) * ObjCopy.g;
				body.Force_born.o += (latt->corner-body.pos.r-body.dPos.r).abs2() * ObjCopy.g;
				latt->gsum+= ObjCopy.g;
				vlist->push_back(ObjCopy);
			}
		}

		body.Force_born.r /= dt;
		body.Force_born.o /= 2.*dt;
		#undef body
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
		const_for(body.List, latt)
		{
			switch (latt->hc)
			{
				case hc::isolate:
					if (latt->hsum)
					{
						hlist->push_back(TObj(latt->r+rotl(latt->dl)*0.5, -latt->hsum));
						latt->hsum = 0;
					}
					break;
				case hc::const_t:
				{
					double tmp_g(latt->dl.abs2() * latt->heat_const);
					TObj *tmp_obj = (latt->ParticleInHeatLayer>=0)? hlist->begin()+latt->ParticleInHeatLayer : NULL;
					if (tmp_obj)
					{
						latt->hsum += tmp_g - tmp_obj->g;
						tmp_obj->g = tmp_g;
					} else
					{
						latt->hsum += tmp_g;
						hlist->push_back(TObj(latt->r+rotl(latt->dl)*0.5, tmp_g));
					}
				}
					break;
				case hc::const_W:
				{
					double tmp_g(latt->dl.abs2() * latt->heat_const);
					latt->hsum += tmp_g;
					hlist->push_back(TObj(latt->r+rotl(latt->dl)*0.5, tmp_g));
				}
					break;
			}
		}
		#undef body
	}
}

void flowmove::CropHeat(double scale)
{
	if (!S->HeatList) return;
	const_for(S->HeatList, lobj)
	{
		if (lobj->r.x > scale)
			S->HeatList->erase(lobj);
	}
}

void flowmove::StreakShed()
{
	if (!S->Time.divisibleBy(S->streak_dt)) return;
	const_for(S->StreakSourceList, lobj)
	{
		S->StreakList->push_back(*lobj);
	}
}

