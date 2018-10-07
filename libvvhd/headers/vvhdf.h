#pragma once

#include <hdf5.h>
#include <string>

void h5_throw(std::string fn, std::string arg);

template<typename T>
// <int32_t>
// <uint32_t>
// <double>
// <TVec>
// <TVec3D>
// <char*>
// <TTime>
struct h5t{
    static hid_t id;
    static const char* name;
    static hid_t init();
    static void commit(hid_t hid);
    static void close();
};

template<typename T>
void h5t_commit(hid_t hid);

template<typename T>
void h5t_close();

void h5t_commit_all(hid_t hid);
void h5t_close_all();

template<typename T>
// <int32_t>
// <uint32_t>
// <double>
// <TVec>
// <TVec3D>
// <std::string>
// <TTime>
// <bc_t>
// <hc_t>
T h5a_read(hid_t hid, const char* name);

template<typename T>
// <int32_t>
// <uint32_t>
// <double>
// <TVec>
// <TVec3D>
// <char const*>
// <std::string const&>
// <TTime>
// <bc_t>
// <hc_t>
void h5a_write(hid_t hid, const char* name, T val);
