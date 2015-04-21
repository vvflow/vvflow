#include "core.h"
#include "flowmove.h"
#include "math.h"
#include <cstdlib>
#include <algorithm>
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
    S->InfMarker+= S->InfSpeed()*S->dt;

    // Move bodies
    // to prevent bodies from separating, fix tangential velocities
    for (auto& lbody: S->BodyList)
    {
        auto root = lbody->root_body.lock();
        if (!root) continue;
        double da = root->speed_slae.o * dt;
        TVec dr = lbody->holder.r - root->get_axis();
        lbody->speed_slae.r -= (dr+rotl(dr)*da) - (dr*cos(da)+rotl(dr)*sin(da));
    }
    for (auto& lbody: S->BodyList) lbody->doRotationAndMotion();
    // Move vortexes, heat, ink
    for (auto& lobj: S->VortexList) lobj.r += lobj.v * dt;
    for (auto& lobj: S->HeatList)   lobj.r += lobj.v * dt;
    for (auto& lobj: S->StreakList) lobj.r += lobj.v * dt;

    // Remove small vortexes, small heat
    // FIXME попробовать переписать с std::remoe_if() и лямбдой
    for (auto lobj = S->VortexList.begin(); lobj < S->VortexList.end(); )
    {
        if ( fabs(lobj->g) < RemoveEps ) S->VortexList.erase(lobj);
        else lobj++;
    }
    for (auto lobj = S->HeatList.begin(); lobj < S->HeatList.end(); )
    {
        if ( fabs(lobj->g) < RemoveEps ) S->HeatList.erase(lobj);
        else lobj++;
    }




    if (remove)
        for (auto lobj = S->VortexList.begin(); lobj < S->VortexList.end(); )
        {
            TAtt* bad_segment = NULL;
            TBody* bad_body = NULL;
            for (auto& lbody: S->BodyList)
            {
                bad_segment = lbody->isPointInvalid(lobj->r);
                if (bad_segment) { bad_body = lbody.get(); break; }
            }
            if (!bad_body) { lobj++; continue; }

            bad_body->force_dead.r += rotl(lobj->r) * lobj->g;
            bad_body->force_dead.o += (lobj->r - bad_body->get_axis()).abs2() * lobj->g;
            bad_segment->gsum -= lobj->g;
            bad_body->g_dead += lobj->g;
            CleanedV_++;
            S->VortexList.erase(lobj);
        }

    for (auto lobj = S->HeatList.begin(); lobj < S->HeatList.end(); )
    {
        TAtt* bad_segment = NULL;
        TBody* bad_body = NULL;
        // check if particle is in body
        for (auto& lbody: S->BodyList)
        {
            bad_segment = lbody->isPointInvalid(lobj->r);
            if (bad_segment) { bad_body = lbody.get(); break; }
        }

        if (bad_body)
        {
            bad_segment->hsum -= lobj->g;
            S->HeatList.erase(lobj);
            continue;
        }

        // check if particle is in heat layer
        bad_segment = NULL;
        bad_body = NULL;
        for (auto& lbody: S->BodyList)
        {
            bad_segment = lbody->isPointInHeatLayer(lobj->r);
            if (bad_segment) { bad_body = lbody.get(); break; }
        }

        if ( !bad_body || bad_body->heat_condition != hc_t::const_t) { lobj++; continue; }

        if (bad_segment->heat_layer_obj_no >= 0)
        {
            bad_segment->hsum -= lobj->g;
            S->HeatList.erase(lobj);
            continue;
        } else
        {
            bad_segment->heat_layer_obj_no = lobj - S->HeatList.begin();
            lobj++;
            continue;
        }
    }

    for (auto lobj = S->StreakList.begin(); lobj < S->StreakList.end(); )
    {
        if ( S->PointIsInBody(lobj->r) ) S->StreakList.erase(lobj);
        else lobj++;
    }

    if (zero_speed)
    {
        for (auto& lobj: S->VortexList) lobj.v = TVec(0., 0.);
        for (auto& lobj: S->HeatList)   lobj.v = TVec(0., 0.);
        for (auto& lobj: S->StreakList) lobj.v = TVec(0., 0.);
    }

    for (auto& lbody: S->BodyList)
    {
        lbody->force_dead.r /= dt;
        lbody->force_dead.o /= 2.*dt;
    }
}

void flowmove::VortexShed()
{
    for (auto& lbody: S->BodyList)
    {
        lbody->force_born = TVec3D(0., 0., 0.);

        for (auto& latt: lbody->alist)
        {
            if (fabs(latt.g) < RemoveEps)
            {
                CleanedV_++;
                lbody->g_dead+= latt.g;
            }
            else if ( !latt.slip )
            {
                S->VortexList.push_back(TObj(latt.corner + rotl(latt.dl)*1e-4, latt.g));
                lbody->force_born.r += rotl(latt.corner) * latt.g;
                lbody->force_born.o += (latt.corner - lbody->get_axis()).abs2() * latt.g;
                latt.gsum+= latt.g;
            }
        }

        lbody->force_born.r /= dt;
        lbody->force_born.o /= 2.*dt;
    }
}

void flowmove::HeatShed()
{
    for (auto& lbody: S->BodyList)
    {
        if (lbody->heat_condition == hc_t::isolate)
            for (auto& latt: lbody->alist)
            {
                if (!latt.hsum) continue;
                S->HeatList.push_back(TObj(latt.r+rotl(latt.dl)*0.5, -latt.hsum));
                latt.hsum = 0;
            }

        else if (lbody->heat_condition == hc_t::const_t)
            for (auto& latt: lbody->alist)
            {
                double tmp_g = latt.dl.abs2() * latt.heat_const;
                TObj *tmp_obj = (latt.heat_layer_obj_no>=0)? S->HeatList.data()+latt.heat_layer_obj_no : NULL;
                if (tmp_obj)
                {
                    latt.hsum += tmp_g - tmp_obj->g;
                    tmp_obj->g = tmp_g;
                } else
                {
                    latt.hsum += tmp_g;
                    S->HeatList.push_back(TObj(latt.r+rotl(latt.dl)*0.5, tmp_g));
                }
            }

        else if (lbody->heat_condition == hc_t::const_w)
            for (auto& latt: lbody->alist)
            {
                double tmp_g = latt.dl.abs2() * latt.heat_const;
                latt.hsum += tmp_g;
                S->HeatList.push_back(TObj(latt.r+rotl(latt.dl)*0.5, tmp_g));
            }
    }
}

void flowmove::CropHeat(double scale)
{
    for (auto lobj = S->HeatList.begin(); lobj < S->HeatList.end(); )
    {
        if (lobj->r.x > scale) S->HeatList.erase(lobj);
        else lobj++;
    }
}

void flowmove::StreakShed()
{
    if (!S->Time.divisibleBy(S->streak_dt)) return;
    for (auto& lobj: S->StreakSourceList)
    {
        S->StreakList.push_back(lobj);
    }
}

