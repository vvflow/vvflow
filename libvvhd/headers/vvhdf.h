#pragma once

#include "elementary.h"

#include <hdf5.h>
#include <string>

template<typename T>
T h5a_read(hid_t hid, const char* name);

// Available instantiations:
template<> int32_t h5a_read(hid_t, const char*);
template<> uint32_t h5a_read(hid_t, const char*);
template<> std::string h5a_read(hid_t, const char*);

// template<typename T>
// void h5a_write(hid_t hid, const char* name, T value);
