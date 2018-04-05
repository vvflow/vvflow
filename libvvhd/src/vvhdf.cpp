#include "vvhdf.h"

#include <limits>
#include <string>

template<typename T>
struct h5t{
    static hid_t id;
};

template<> hid_t h5t<int32_t>::id = H5T_NATIVE_INT32;
template<> hid_t h5t<uint32_t>::id = H5T_NATIVE_UINT32;
template<> hid_t h5t<double>::id = H5T_NATIVE_DOUBLE;

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
std::string h5a_read(hid_t hid, const char* name)
{
    if (!H5Aexists(hid, name))
        return "";
    
    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid<0)
        return "";

    static hid_t id = 0;
    if (!id) {
        id = H5Tcopy(H5T_C_S1);
        H5Tset_size(id, H5T_VARIABLE);
    }

    char* buf = NULL;
    if (H5Aread(aid, id, &buf)<0)
        return "";
    (void)H5Aclose(aid);
    std::string result = buf;
    free(buf);
    return result;
}

// Explicit instantiation:
template int32_t h5a_read(hid_t, const char*);
template uint32_t h5a_read(hid_t, const char*);
template double h5a_read(hid_t, const char*);
template std::string h5a_read(hid_t, const char*);
