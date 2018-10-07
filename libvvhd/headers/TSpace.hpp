#pragma once

#include "TVec.hpp"
#include "TObj.hpp"
#include "TVec3D.hpp"
#include "TBody.hpp"
#include "TTime.hpp"

#include <string>
#include <vector>

struct metainfo_t {
    std::string git_rev;
    std::string git_info;
    std::string git_diff;
    std::string time_local;
};

class Space
{
    public:
        Space();
        std::string caption;
        double re; // re = 1/nyu, where nyu is the kinematic viscosity of fluid
        double pr;
        double inf_g; // circulation over an infinite contour
        TEval inf_vx;
        TEval inf_vy;
        TVec inf_marker;
        TVec gravity; // acceleration of gravity
        TTime time; // current simulation time
        TTime dt; // simulation timestep
        TTime dt_save; // period to save results
        TTime dt_streak; // period of streak domains shedding
        TTime dt_profile; // period to save profiles of pressure and friction
        double finish; // time to finish the simulation

        std::vector<std::shared_ptr<TBody>> BodyList; // FIXME rename to body_list
        std::vector<TObj> VortexList; // FIXME rename to vort_list
        std::vector<TObj> HeatList; // FIXME rename to heat_list
        std::vector<TObj> SourceList; // FIXME rename to sink_list
        std::vector<TObj> StreakSourceList; // FIXME rename to streak_source_list
        std::vector<TObj> StreakList;       // FIXME rename to streak_points_list

    public:
        /***************** SAVE/LOAD ******************/
        void save(const char* format);
        void load(const char* filename, metainfo_t *info = NULL);
        void save_hdf(int64_t fid);
        void load_hdf(int64_t fid, metainfo_t *info = NULL);
        void load_v13(const char* filename); // deprecated format (for compatibility)
        FILE* open_file(const char* format);
    
        static int load_list_txt(std::vector<TObj>& li, const char* filename);
        static int load_list_bin(std::vector<TObj>& li, const char* filename);
        int load_body_txt(const char* filename);

    public:
        void calc_forces();
        void zero_forces(); //zero all att-> Cp, Fr, Nu, gsum, fric, hsum variables.

        void finish_step() {
            time = TTime::add(time, dt);
        }
        void zero_speed() {
            for (TObj& lobj: VortexList) {
                lobj.v = TVec();
            }
            for (TObj& lobj: HeatList) {
                lobj.v = TVec();
            }
        }

        void EnumerateBodies();
        void ZeroBodies(); //zero Cp, Fr, Nu variables.

    public: // const methods
        TVec inf_speed() const {
            return TVec(
                inf_vx.eval(time),
                inf_vy.eval(time)
            );
        }

        double integral() const {
            double res = 0;
            for (const TObj& obj: VortexList) {
                res += obj.g * obj.r.abs2();
            }
            return res;
        }

        double gsum() const {
            double res = 0;
            for (const TObj& obj: VortexList) {
                res += obj.g;
            }
            return res;
        }

        double gmax() const {
            double res = 0;
            for (const TObj& obj: VortexList) {
                if ( fabs(obj.g) > fabs(res) ) {
                    res = obj.g;
                }
            }
            return res;
        }

        TVec hydrodynamic_momentum() const {
            TVec res = {};
            for (const TObj& obj: VortexList) {
                res += obj.g * obj.r;
            }
            return res;
        }

        double average_segment_length() const {
            if (!BodyList.size())
                return 0; // std::numeric_limits<double>::lowest();

            const TBody& body = *BodyList.front();
            double L = body.get_slen();
            size_t N = body.size();
            if (N<=1)
                return 0; // std::numeric_limits<double>::lowest();
            return L / (N-1);
        }

        size_t total_segment_count() const {
            size_t res = 0;
            for (const std::shared_ptr<TBody>& lbody: BodyList)
            {
                res += lbody->size();
            }
            return res;
        }

        bool point_is_in_body(TVec p) const {
            for (const std::shared_ptr<TBody>& lbody: BodyList) {
                if (lbody->isPointInvalid(p)) return true;
            }
            return false;
        }

        int get_body_index(const TBody* body) const;
        std::string get_body_name(const TBody* body) const;
};
