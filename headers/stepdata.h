#ifndef STEPDATA_H_
#define STEPDATA_H_

#include "core.h"
#include <iostream>
#include <vector>
#include <time.h>

class Stepdata
{
    public:
        Stepdata(Space* s_, const char* fname, bool b_save_profile);
        ~Stepdata();
        void write();
        void flush();

    private:
        void attribute_write(const char *name, const char *str);
        int h5d_init(int loc_id, const char *name, size_t rows, size_t cols);
        void append(int dataspace_hid, const void* buf);
        void append(int dataspace_hid, double value);
        void append(int dataspace_hid, TVec3D value);
        Space *S;
        int blsize;
        bool b_save_profile;
        time_t last_flush_time;
        int h5f;
        int string_h5t;
        int scalar_h5s;

        int time_h5d;
        std::vector<int> force_born_h5d;
        std::vector<int> force_hydro_h5d;
        std::vector<int> force_holder_h5d;
        std::vector<int> force_friction_h5d;
        std::vector<int> nusselt_h5d;
        std::vector<int> position_h5d;
        std::vector<int> spring_h5d;
        std::vector<int> speed_h5d;
        std::vector<int> pressure_h5d;
        std::vector<int> friction_h5d;
};

#endif
