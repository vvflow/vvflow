#define H5_USE_18_API
#include <hdf5.h>

#include "vvhdf.h"
#include "TVec.hpp"
#include "TObj.hpp"
#include "TVec3D.hpp"

#include <limits>
#include <string>

template<typename T>
T h5a_read(hid_t hid, const char* name) {
    if (!H5Aexists(hid, name))
        return T();
    
    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid<0)
        return T();

    T result = T();
    if (H5Aread(aid, h5t<T>::id, &result)<0)
        return T();
    (void)H5Aclose(aid);
    return result;
}

template<>
TVec3D h5a_read(hid_t hid, const char* name)
{
    if (!H5Aexists(hid, name))
        return TVec3D();
    
    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid<0)
        throw std::runtime_error("H5Aopen failed");

    TVec3D ret;
    herr_t err = H5Aread(aid, h5t<TVec3D>::init(), &ret);
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
    if (!H5Aexists(hid, name))
        return "";
    
    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid<0)
        throw std::runtime_error("H5Aopen failed");

    char* buf = NULL;
    herr_t err = H5Aread(aid, h5t<std::string>::init(), &buf);
    if (err < 0)
        throw std::runtime_error("H5Aread failed");
    
    err = H5Aclose(aid);
    if (err < 0)
        throw std::runtime_error("H5Aclose failed");

    std::string ret = buf;
    free(buf);
    return ret;
}

// Explicit instantiation:
template int32_t h5a_read(hid_t, const char*);
template uint32_t h5a_read(hid_t, const char*);
template double h5a_read(hid_t, const char*);
template TVec3D h5a_read(hid_t, const char*);
template std::string h5a_read(hid_t, const char*);
