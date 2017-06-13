#ifndef SPACE_H_
#define SPACE_H_

class Space;
class TBody;

#include "elementary.h"
#include "sorted_tree.h"
// #include "hdf5.h"

#include <iostream>
#include <memory>

using std::vector;
using std::shared_ptr;

typedef int hid_t;
typedef int herr_t;

class Space
{
    public:
        Space();
        std::string caption;
        time_t realtime;

        //FIXME make all lists initialized
        vector<shared_ptr<TBody>> BodyList; // FIXME rename to blist, vlist, hlist, ilist, slist
        vector<TObj> VortexList;
        vector<TObj> HeatList;
        vector<TObj> StreakSourceList;
        vector<TObj> StreakList;
        vector<TObj> SourceList;
        TSortedTree *Tree;

        inline void FinishStep(); //update time and coord variables

        ShellScript InfSpeedX;
        ShellScript InfSpeedY;
        TVec InfSpeed() {return TVec(InfSpeedX.getValue(Time), InfSpeedY.getValue(Time));}
        TVec InfMarker;
        void ZeroSpeed();
        double InfCirculation;
        TVec gravitation; //gravitational acceleration
        TTime Time, dt;
        TTime dt_save, dt_streak, dt_profile;
        double Re, Pr, Finish;

        void Save(const char* format);
        void Load(const char* filename, std::string *info = NULL);
        void Load(hid_t fid, std::string *info = NULL);
        FILE* OpenFile(const char* format);
        void CalcForces();
        void ZeroForces(); //zero all att-> Cp, Fr, Nu, gsum, fric, hsum variables.

        /***************** SAVE/LOAD ******************/
        int LoadVorticityFromFile(const char* filename);
        int LoadVorticity_bin(const char* filename);
        int LoadHeatFromFile(const char* filename);
        int LoadStreak(const char* filename);
        int LoadStreakSource(const char* filename);
        int LoadSource(const char* filename);

        int LoadBody(const char* filename);
        void EnumerateBodies();
        void ZeroBodies(); //zero Cp, Fr, Nu variables.

        /***************** INTEGRALS ******************/
        double integral();
        double gsum();
        double gmax();
        TVec HydroDynamicMomentum();
        double AverageSegmentLength();
        int TotalSegmentsCount();
        bool PointIsInBody(TVec p);
        int get_body_index(const TBody* body) const;
        std::string get_body_name(const TBody* body) const;
        shared_ptr<TBody> collision_detected() const;

    private:
        void Load_v1_3(const char* filename);
        herr_t dataset_read_list(hid_t fid, const char *name, vector<TObj>& list);
        void dataset_write_list(const char* name, const vector<TObj>& list);
        void dataset_write_body(const char* name, const TBody& body);
};

#endif /*SPACE_H_*/