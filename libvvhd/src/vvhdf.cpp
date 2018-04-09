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
    
    h5t<T>::init();

    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid < 0)
        throw std::runtime_error("H5Aopen (" + std::string(name) + ") failed");

    T ret = T();
    herr_t err = H5Aread(aid, h5t<T>::id, &ret);
    if (err < 0)
        throw std::runtime_error("H5Aread (" + std::string(name) + ") failed");

    err = H5Aclose(aid);
    if (err < 0)
        throw std::runtime_error("H5Aclose (" + std::string(name) + ") failed");
    return ret;
}

template<>
std::string h5a_read(hid_t hid, const char* name)
{
    const char* buf = h5a_read<const char*>(hid, name);
    std::string ret = buf?:"";
    free(const_cast<char*>(buf));
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
template bc_t h5a_read(hid_t, const char*);
template hc_t h5a_read(hid_t, const char*);

///////////////////////////////////////////////////////////////////////////////
// H5A_WRITE //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static hid_t h5s_scalar()
{
    static hid_t h5s = H5Screate(H5S_SCALAR);
    return h5s;
}

template<typename T>
void h5a_write(hid_t hid, const char* name, T val)
{
    h5t<T>::init();

    hid_t aid = H5Acreate(hid, name, h5t<T>::id, h5s_scalar(), H5P_DEFAULT, H5P_DEFAULT);
    if (aid < 0)
        throw std::runtime_error("H5Acreate (" + std::string(name) + ") failed");
    
    herr_t err = H5Awrite(aid, h5t<T>::id, &val);
    if (err < 0)
        throw std::runtime_error("H5Awrite (" + std::string(name) + ") failed");

    err = H5Aclose(aid);
    if (err < 0)
        throw std::runtime_error("H5Aclose (" + std::string(name) + ") failed");
}

template<>
void h5a_write(hid_t hid, const char* name, std::string const& str)
{
    if (str.empty())
        return;

    h5a_write<char const*> (hid, name, str.c_str());
}


template<>
void h5a_write(hid_t hid, const char* name, bc_t bc)
{
    uint32_t bc_raw = 0;
    switch (bc) {
    case bc_t::steady: bc_raw = 0; break;
    case bc_t::kutta: bc_raw = 1; break;
    }
    h5a_write<uint32_t> (hid, name, bc_raw);
}

template<>
void h5a_write(hid_t hid, const char* name, hc_t hc)
{
    uint32_t hc_raw = 0;
    switch (hc) {
    case hc_t::neglect: hc_raw = 0; break;
    case hc_t::isolate: hc_raw = 1; break;
    case hc_t::const_t: hc_raw = 2; break;
    case hc_t::const_w: hc_raw = 3; break;
    }
    h5a_write<uint32_t> (hid, name, hc_raw);
}

// Explicit instantiation:
template void h5a_write(hid_t, const char*, int32_t );
template void h5a_write(hid_t, const char*, uint32_t );
template void h5a_write(hid_t, const char*, double );
template void h5a_write(hid_t, const char*, TVec );
template void h5a_write(hid_t, const char*, TVec3D );
template void h5a_write(hid_t, const char*, char const* );
template void h5a_write(hid_t, const char*, std::string const& );
template void h5a_write(hid_t, const char*, TTime );
template void h5a_write(hid_t, const char*, bc_t );
template void h5a_write(hid_t, const char*, hc_t );
