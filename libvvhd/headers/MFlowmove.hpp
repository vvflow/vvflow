#pragma once

#include "TSpace.hpp"

class MFlowmove
{
    public:
        MFlowmove(Space *S, double remove_eps = 1E-10):
            S(S), remove_eps(remove_eps) {}
        void move_and_clean(bool remove, const void** collision, size_t* cleaned_v = NULL);
        void vortex_shed();
        void streak_shed();
        void heat_shed();
        void heat_crop(double scale = 16);

    private:
        Space *S;
        double remove_eps;
};
