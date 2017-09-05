#include <hdf5.h>
#include "elementary.h"

// inline
// void attribute_read_double(hid_t hid, const char *name, double &value)
// {
//     if (!H5Aexists(hid, name)) { value = 0; return; }

//     hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
//     if (aid<0) { return; }

//     H5Aread(aid, H5T_NATIVE_DOUBLE, &value);
//     H5Aclose(aid);
// }

template<typename T>
T h5a_read(hid_t hid, const char* name);

// Available instantiations:
template<> int32_t h5a_read(hid_t, const char*);
template<> uint32_t h5a_read(hid_t, const char*);
template<> std::string h5a_read(hid_t, const char*);
// template   std::string h5a_read(hid_t, const char*);


// template<typename T>
// void h5a_write(hid_t hid, const char* name, T value);
