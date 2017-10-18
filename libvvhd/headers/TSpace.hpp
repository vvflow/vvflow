#pragma once

#include "TVec.hpp"
#include "TObj.hpp"
#include "TVec3D.hpp"
#include "TBody.hpp"
#include "TTime.hpp"

#include <string>
// #include <memory>
#include <vector>

typedef int hid_t;
typedef int herr_t;

class Space
{
    public:
        Space();
        std::string caption;

        //FIXME make all lists initialized
        std::vector<std::shared_ptr<TBody>> BodyList; // FIXME rename to body_list
        std::vector<TObj> VortexList; // FIXME rename to vort_list
        std::vector<TObj> HeatList; // FIXME rename to heat_list
        std::vector<TObj> SourceList; // FIXME rename to sink_list
        std::vector<TObj> StreakSourceList; // FIXME rename to streak_source_list
        std::vector<TObj> StreakList;       // FIXME rename to streak_points_list
        // TSortedTree *Tree;

        inline void FinishStep(); //update time and coord variables

        TEval InfSpeedX;
        TEval InfSpeedY;
        TVec InfSpeed() const {return TVec(InfSpeedX.eval(Time), InfSpeedY.eval(Time));}
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
        static int load_list_txt(std::vector<TObj>& li, const char* filename);
        static int load_list_bin(std::vector<TObj>& li, const char* filename);
        int load_body_txt(const char* filename);

        void EnumerateBodies();
        void ZeroBodies(); //zero Cp, Fr, Nu variables.

        /***************** INTEGRALS ******************/
        double integral() const;
        double gsum() const;
        double gmax() const;
        TVec HydroDynamicMomentum() const;
        double AverageSegmentLength() const;
        int TotalSegmentsCount() const;
        bool PointIsInBody(TVec p) const;
        int get_body_index(const TBody* body) const;
        std::string get_body_name(const TBody* body) const;

    private:
        void Load_v1_3(const char* filename);
        herr_t dataset_read_list(hid_t fid, const char *name, std::vector<TObj>& list);
        void dataset_write_list(const char* name, const std::vector<TObj>& list);
        void dataset_write_body(const char* name, const TBody& body);
};
