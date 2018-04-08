#define H5_USE_18_API
#include <hdf5.h>

#include "vvhdf.h"
#include "TVec.hpp"
#include "TObj.hpp"
#include "TVec3D.hpp"
#include "TBody.hpp"
#include "TTime.hpp"

#include <string>

template<typename T>
T h5a_read(hid_t hid, const char* name) {
    if (!H5Aexists(hid, name))
        return T();
    
    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid < 0)
        throw std::runtime_error("H5Aopen failed");

    T ret = T();
    herr_t err = H5Aread(aid, h5t<T>::init(), &ret);
    if (err < 0)
        throw std::runtime_error("H5Aread failed");

    err = H5Aclose(aid);
    if (err < 0)
        throw std::runtime_error("H5Aclose failed");
    return ret;
}

template<>
std::string h5a_read(hid_t hid, const char* name)
{
    char* buf = h5a_read<char*>(hid, name);
    std::string ret = buf?:"";
    free(buf);
    return ret;
}

template<>
bc_t h5a_read(hid_t hid, const char* name)
{
    uint32_t bc_raw = h5a_read<uint32_t>(hid, name);
    switch (bc_raw) {
    case 0:
        return bc_t::steady;
    case 1:
        return bc_t::kutta;
    default:
        fprintf(stderr, "Warning: bad boundary condition (%d), using bc::steady\n", bc_raw);
        return bc_t::steady;
    }
}

template<>
hc_t h5a_read(hid_t hid, const char* name)
{
    uint32_t hc_raw = h5a_read<uint32_t>(hid, name);
    switch (hc_raw) {
    case 0:
        return hc_t::neglect;
    case 1:
        return hc_t::isolate;
    case 2:
        return hc_t::const_t;
    case 3:
        return hc_t::const_w;
    default:
        fprintf(stderr, "Warning: bad heat condition (%d), using hc::neglect\n", hc_raw);
        return hc_t::neglect;
    }
}

// Explicit instantiation:
template int32_t h5a_read(hid_t, const char*);
template uint32_t h5a_read(hid_t, const char*);
template double h5a_read(hid_t, const char*);
template TVec h5a_read(hid_t, const char*);
template TVec3D h5a_read(hid_t, const char*);
template std::string h5a_read(hid_t, const char*);
template TTime h5a_read(hid_t, const char*);
