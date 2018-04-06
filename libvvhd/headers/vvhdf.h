#pragma once

#include "elementary.h"

#include <hdf5.h>
#include <string>

template<typename T>
// <int32_t>
// <uint32_t>
// <double>
// <std::string>
struct h5t{
    static hid_t id;
    static hid_t init();
    static void commit(hid_t hid);
    static void close();
};

void h5t_close_all();

template<typename T>
// <int32_t>
// <uint32_t>
// <double>
// <std::string>
T h5a_read(hid_t hid, const char* name);

// Available instantiations:

// template<typename T>
// void h5a_write(hid_t hid, const char* name, T value);
