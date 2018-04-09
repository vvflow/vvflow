#define H5_USE_18_API
#include <hdf5.h>

#include "vvhdf.h"
#include "TVec.hpp"
#include "TVec3D.hpp"
#include "TTime.hpp"

#include <stdexcept>

// ID
template<> hid_t h5t<int32_t>::id = H5T_NATIVE_INT32;
template<> hid_t h5t<uint32_t>::id = H5T_NATIVE_UINT32;
template<> hid_t h5t<double>::id = H5T_NATIVE_DOUBLE;
template<> hid_t h5t<TVec>::id = -1;
template<> hid_t h5t<TVec3D>::id = -1;
template<> hid_t h5t<const char*>::id = -1;
template<> hid_t h5t<TTime>::id = -1;

// NAME
template<> const char* h5t<int32_t>::name = "int32_t";
template<> const char* h5t<uint32_t>::name = "uint32_t";
template<> const char* h5t<double>::name = "double";
template<> const char* h5t<TVec>::name = "vec_t";
template<> const char* h5t<TVec3D>::name = "vec3d_t";
template<> const char* h5t<const char*>::name = "string_t";
template<> const char* h5t<TTime>::name = "fraction_t";

// INIT
template<> hid_t h5t<int32_t>::init() {return id;}
template<> hid_t h5t<uint32_t>::init() {return id;}
template<> hid_t h5t<double>::init() {return id;}
template<> hid_t h5t<TVec>::init()
{
    if (id >= 0) return id;
    id = H5Tcreate(H5T_COMPOUND, 16);
    H5Tinsert(id, "x", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(id, "y", 8, H5T_NATIVE_DOUBLE);
    H5Tpack(id);
    return id;
}
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
template<> hid_t h5t<const char*>::init()
{
    if (id >= 0) return id;
    id = H5Tcopy(H5T_C_S1);
    H5Tset_size(id, H5T_VARIABLE);
    return id;
}
template<> hid_t h5t<TTime>::init()
{
    if (id >= 0) return id;
    id = H5Tcreate(H5T_COMPOUND, 8);
    H5Tinsert(id, "value", 0, H5T_STD_I32LE);
    H5Tinsert(id, "timescale", 4, H5T_STD_U32LE);
    return id;
}

// COMMIT
template<typename T>
void h5t<T>::commit(hid_t hid)
{
    if (id < 0 || H5Tcommitted(id))
        return;
    herr_t err = H5Tcommit(hid, name, id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (err < 0)
        throw std::runtime_error("H5Tcommit (" + std::string(name) + ") failed");
}

void h5t_commit_all(hid_t hid)
{
    h5t<TVec>::commit(hid);
    h5t<TVec3D>::commit(hid);
    h5t<const char*>::commit(hid);
    h5t<TTime>::commit(hid);
}


// CLOSE
template<typename T>
void h5t<T>::close()
{
    if (id < 0)
        return;
    herr_t err = H5Tclose(id);
    id = -1;
    if (err < 0)
        throw std::runtime_error("H5Tclose (" + std::string(name) + ") failed");
}

void h5t_close_all()
{
    h5t<TVec>::close();
    h5t<TVec3D>::close();
    h5t<const char*>::close();
    h5t<TTime>::close();
}
