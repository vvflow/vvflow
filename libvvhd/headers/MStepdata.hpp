#pragma once

#include "TSpace.hpp"

#include <vector>

class Stepdata
{
    public:
        Stepdata(Space* S, const char* fname, bool b_save_profile);
        Stepdata(const Stepdata&) = delete;
        Stepdata& operator=(const Stepdata&) = delete;
        ~Stepdata();
        void write();
        void flush();

    private:
        int64_t h5d_init(int64_t loc_id, const char *name, size_t rows, size_t cols);
        void append(int64_t h5d, const void* buf);
        void append(int64_t h5d, double value);
        void append(int64_t h5d, TVec3D value);

        Space *S;
        bool b_save_profile;
        size_t blsize;
        time_t last_flush_time;

        int64_t h5f;
        int64_t time_h5d;
        std::vector<int64_t> force_hydro_h5d;
        std::vector<int64_t> force_holder_h5d;
        std::vector<int64_t> force_friction_h5d;
        std::vector<int64_t> nusselt_h5d;
        std::vector<int64_t> position_h5d;
        std::vector<int64_t> spring_h5d;
        std::vector<int64_t> speed_h5d;
        std::vector<int64_t> pressure_h5d;
        std::vector<int64_t> friction_h5d;
};
