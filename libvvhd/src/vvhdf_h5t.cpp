#define H5_USE_18_API
#include <hdf5.h>

#include "vvhdf.h"
#include "TVec3D.hpp"

template<> hid_t h5t<int32_t>::id = H5T_NATIVE_INT32;
template<> hid_t h5t<uint32_t>::id = H5T_NATIVE_UINT32;
template<> hid_t h5t<double>::id = H5T_NATIVE_DOUBLE;
template<> hid_t h5t<TVec3D>::id = -1;
template<> hid_t h5t<std::string>::id = -1;

/* TVec3D */
template<> hid_t h5t<TVec3D>::init()
{
    if (id >= 0) return id;
    id = H5Tcreate(H5T_COMPOUND, 24);
    H5Tinsert(id, "x", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(id, "y", 8, H5T_NATIVE_DOUBLE);
    H5Tinsert(id, "o", 16, H5T_NATIVE_DOUBLE);
    H5Tpack(id);
    return id;
}
template<> void h5t<TVec3D>::commit(hid_t hid)
{
    if (H5Tcommitted(id))
        return; 
    H5Tcommit(hid, "vec3d_t", id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}
template<> void h5t<TVec3D>::close()
{
    H5Tclose(id);
    id = -1;
}

/* string_t */
template<> hid_t h5t<std::string>::init()
{
    if (id >= 0) return id;
    id = H5Tcopy(H5T_C_S1);
    H5Tset_size(id, H5T_VARIABLE);
    return id;
}
template<> void h5t<std::string>::commit(hid_t hid)
{
    if (H5Tcommitted(id))
        return; 
    H5Tcommit(hid, "string_t", id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}
template<> void h5t<std::string>::close()
{
    H5Tclose(id);
    id = -1;
}

void h5t_close_all()
{
    h5t<std::string>::close();
}
