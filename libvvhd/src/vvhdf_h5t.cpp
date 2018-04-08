#define H5_USE_18_API
#include <hdf5.h>

#include "vvhdf.h"
#include "TVec.hpp"
#include "TVec3D.hpp"
#include "TTime.hpp"

template<> hid_t h5t<int32_t>::id = H5T_NATIVE_INT32;
template<> hid_t h5t<uint32_t>::id = H5T_NATIVE_UINT32;
template<> hid_t h5t<double>::id = H5T_NATIVE_DOUBLE;
template<> hid_t h5t<TVec>::id = -1;
template<> hid_t h5t<TVec3D>::id = -1;
template<> hid_t h5t<char*>::id = -1;
template<> hid_t h5t<TTime>::id = -1;

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
template<> hid_t h5t<char*>::init()
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
static void __commit(hid_t hid, hid_t id, const char* name)
{
    if (H5Tcommitted(id))
        return; 
    H5Tcommit(hid, name, id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}
template<> void h5t<int32_t>::commit(hid_t hid) {}
template<> void h5t<uint32_t>::commit(hid_t hid) {}
template<> void h5t<double>::commit(hid_t hid) {}
template<> void h5t<TVec>::commit(hid_t hid) {__commit(hid, id, "vec_t");}
template<> void h5t<TVec3D>::commit(hid_t hid) {__commit(hid, id, "vec3d_t");}
template<> void h5t<char*>::commit(hid_t hid) {__commit(hid, id, "string_t");}
template<> void h5t<TTime>::commit(hid_t hid) {__commit(hid, id, "fraction_t");}

// CLOSE
static void __close(hid_t& id)
{
    H5Tclose(id);
    id = -1;
}
template<> void h5t<TVec>::close() {__close(id);}
template<> void h5t<TVec3D>::close() {__close(id);}
template<> void h5t<char*>::close() {__close(id);}
template<> void h5t<TTime>::close() {__close(id);}

void h5t_close_all()
{
    h5t<TVec>::close();
    h5t<TVec3D>::close();
    h5t<char*>::close();
    h5t<TTime>::close();
}
