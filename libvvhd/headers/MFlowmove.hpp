#pragma once

#include "TSpace.hpp"

class flowmove
{
    public:
        flowmove(Space *S, double RemoveEps = 1E-10):
            S(S), dt(S->dt), RemoveEps(RemoveEps), CleanedV_(0) {}
        void MoveAndClean(bool remove, const void** collision);
        void VortexShed();
        void StreakShed();
        void HeatShed();
        void CropHeat(double scale = 16);

        int CleanedV() {return CleanedV_;}
        //int CleanedH();
    private:
        Space *S;
        double dt;
        double RemoveEps;
        int CleanedV_;
        //int CleanedH_;
};
