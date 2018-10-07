#include "MFlowmove.hpp"

#include <ctime>
#include <cmath>
#include <map>

using std::shared_ptr;

const double d_nan = 0.0l/0.0l;

void MFlowmove::move_and_clean(bool remove, const void** collision, size_t* cleaned_v)
{
    size_t _tmp = 0;
    if (cleaned_v == nullptr) {
        cleaned_v = &_tmp;
    }
    *cleaned_v = 0;

    double current_dt = S->dt;
    if (collision == nullptr) {
        throw std::invalid_argument("MFlowmove::move_and_clean(): invalid collision pointer");
    }
    *collision = nullptr;

    // Detect collision
    // If required, shorten dt and set collision state
    for (std::shared_ptr<TBody>& lbody: S->BodyList)
    {
        TVec3D cur_pos_ = lbody->holder + lbody->dpos;
        TVec3D new_pos_ = cur_pos_ + S->dt*lbody->speed_slae;

        #define COMPONENTS(vec) {vec.r.x, vec.r.y, vec.o};

        double cur_pos[] = COMPONENTS(cur_pos_);
        double new_pos[] = COMPONENTS(new_pos_);
        double speed[]   = COMPONENTS(lbody->speed_slae);
        double max[] = COMPONENTS(lbody->collision_max);
        double min[] = COMPONENTS(lbody->collision_min);
        const double* kspring_[] = COMPONENTS(&lbody->kspring);

        for (int i=0; i<3; i++) {
            double dt_ = d_nan;
            if (TBody::isrigid(*kspring_[i])) {
                /* do nothing */
            } else if (new_pos[i] > max[i]) {
                dt_ = (max[i] - cur_pos[i]) / speed[i];
            } else if (new_pos[i] < min[i]) {
                dt_ = (min[i] - cur_pos[i]) / speed[i];
            }

            if (dt_ < current_dt) {
                current_dt = dt_;
                *collision = kspring_[i];
            }
        }
        #undef COMPONENTS
    }

    S->inf_marker+= S->inf_speed()*current_dt;

    // Move bodies
    std::map<TBody*,TVec3D> deltaHolder;
    std::map<TBody*,TVec3D> deltaBody;
    // to prevent bodies from separating, fix tangential velocities
    for (std::shared_ptr<TBody>& lbody: S->BodyList)
    {
        TVec3D dHolder = lbody->speed(S->time);
        TVec3D dBody = lbody->speed_slae;

        std::shared_ptr<TBody> root = lbody->root_body.lock();
        if (root) {
            TVec dr = lbody->holder.r - root->get_axis();
            dHolder.r += root->speed_slae.r + rotl(dr)*root->speed_slae.o;
            dHolder.o += root->speed_slae.o;
        }

        dHolder.r *= current_dt;
        dHolder.o *= current_dt;
        dBody.r *= current_dt;
        dBody.o *= current_dt;
        auto child = lbody;
        while(root)
        {
            TVec dr = child->holder.r - root->get_axis();
            double da = root->speed_slae.o * current_dt;
            TVec error = (dr+rotl(dr)*da) - (dr*cos(da)+rotl(dr)*sin(da));
            dHolder.r -= error;
            dBody.r -= error;

            child = root;
            root = root->root_body.lock();
        }

        deltaHolder[lbody.get()] = dHolder;
        deltaBody[lbody.get()] = dBody;
    }

    // пробегаем в цикле все тела А
    // если у тела скорость ноль - пропускаем
    // пробегаем в цикле все отрезки
    // пробегаем в цикле все тела Б
    // если тело А == Б - пропускаем
    // если вершина тела А после этого шага залезет внутрь тела Б - устраиваем соударение

    for (auto& lbody: S->BodyList) lbody->move(deltaHolder[lbody.get()], deltaBody[lbody.get()]);
    // Move vortexes, heat, ink
    for (auto& lobj: S->VortexList) lobj.r += lobj.v * current_dt;
    for (auto& lobj: S->HeatList)   lobj.r += lobj.v * current_dt;
    for (auto& lobj: S->StreakList) lobj.r += lobj.v * current_dt;

    // Remove small vortexes, small heat
    // FIXME попробовать переписать с std::remoe_if() и лямбдой
    for (auto lobj = S->VortexList.begin(); lobj < S->VortexList.end(); )
    {
        if ( fabs(lobj->g) < remove_eps ) S->VortexList.erase(lobj);
        else lobj++;
    }
    for (auto lobj = S->HeatList.begin(); lobj < S->HeatList.end(); )
    {
        if ( fabs(lobj->g) < remove_eps ) S->HeatList.erase(lobj);
        else lobj++;
    }

    if (remove)
    {
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

            bad_body->fdt_dead.r += rotl(lobj->r) * lobj->g;
            bad_body->fdt_dead.o += (lobj->r - bad_body->get_axis()).abs2() * lobj->g;
            bad_segment->gsum -= lobj->g;
            bad_body->g_dead += lobj->g;
            *cleaned_v += 1;
            S->VortexList.erase(lobj);
        }
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
        if ( S->point_is_in_body(lobj->r) ) S->StreakList.erase(lobj);
        else lobj++;
    }

    if (true)
    {
        for (auto& lobj: S->VortexList) lobj.v = TVec(0., 0.);
        for (auto& lobj: S->HeatList)   lobj.v = TVec(0., 0.);
        for (auto& lobj: S->StreakList) lobj.v = TVec(0., 0.);
    }

    for (auto& lbody: S->BodyList)
    {
        TVec axis = lbody->get_axis();
        for (auto& latt: lbody->alist)
        {
            if (fabs(latt.g) < remove_eps || latt.slip)
            {
                lbody->fdt_dead.r += rotl(latt.corner) * latt.g;
                lbody->fdt_dead.o += (latt.corner - axis).abs2() * latt.g;
                latt.gsum -= latt.g;
                lbody->g_dead += latt.g;
            }
        }
    }
}

void MFlowmove::vortex_shed()
{
    for (auto& lbody: S->BodyList)
    {
        for (auto& latt: lbody->alist)
        {
            // gsum инкрементится даже для отрезков со скольжением,
            // хотя реальные вихри не рождаются
            // декремент происходит на этапе MoveAndClean
            // что бы правильно считались силы и распред давления.
            latt.gsum+= latt.g;

            if (fabs(latt.g) >= remove_eps && !latt.slip)
            {
                S->VortexList.push_back(TObj(latt.corner + rotl(latt.dl)*1e-4, latt.g));
            }
        }
    }
}

void MFlowmove::heat_shed()
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

void MFlowmove::heat_crop(double scale)
{
    for (auto lobj = S->HeatList.begin(); lobj < S->HeatList.end(); )
    {
        if (lobj->r.x > scale) S->HeatList.erase(lobj);
        else lobj++;
    }
}

void MFlowmove::streak_shed()
{
    if (!S->time.divisibleBy(S->dt_streak)) return;
    for (auto& lobj: S->StreakSourceList)
    {
        S->StreakList.push_back(lobj);
    }
}

