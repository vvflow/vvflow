#include <limits>
#include <hdf5.h>

static hid_t fid;
static hid_t DATASPACE_SCALAR()
{
    static hid_t d = H5Screate(H5S_SCALAR);
    return d;
}

// DATATYPES
static hid_t string_t;   bool commited_string = false;
static hid_t fraction_t; bool commited_fraction = false;
static hid_t bool_t;     bool commited_bool = false;
static hid_t vec_t;      bool commited_vec = false;
static hid_t vec3d_t;    bool commited_vec3d = false;
// static hid_t att_detailed_t;

static const hsize_t numbers[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

#define H5ASSERT(expr, msg) if (expr<0) { \
	fprintf(stderr, "%s failed. Aborting.", msg); \
	std::exit(5); \
}

struct ATT {
    double x, y, g, gsum;
};

/*struct ATT_DETAILED {
  double x, y, g, gsum;
  bc::BoundaryCondition bc;
  hc::HeatCondition hc;
  double heat_const;
  };*/

// ############################################################################
// **** READ ******************************************************************
// ############################################################################

template<typename T> void attribute_read(hid_t, const char*, T& value);

bool attread(hid_t hid, const char *name, void *value, hid_t type_id)
{
    if (!H5Aexists(hid, name)) { return false; }

    hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
    if (aid<0) return false;

    if (H5Aread(aid, type_id, value)<0) return false;
    if (H5Aclose(aid)<0) return false;
    return true;
}

template<>
void attribute_read(hid_t hid, const char *name, uint32_t& val)
{
    if (!attread(hid, name, &val, H5T_NATIVE_UINT32))
        val = 0;
}

template<>
void attribute_read(hid_t hid, const char *name, int32_t& val)
{
    if (!attread(hid, name, &val, H5T_NATIVE_INT32))
        val = 0;
}

template<>
void attribute_read(hid_t hid, const char *name, double& val)
{
    if (!attread(hid, name, &val, H5T_NATIVE_DOUBLE))
        val = 0.0;
}

template<>
void attribute_read(hid_t hid, const char *name, TVec& val)
{
    if (!attread(hid, name, &val, vec_t))
        val = TVec();
}

template<>
void attribute_read(hid_t hid, const char *name, TVec3D& val)
{
    if (!attread(hid, name, &val, vec3d_t))
        val = TVec3D();
}

template<>
void attribute_read(hid_t hid, const char *name, TTime& val)
{
    if (!attread(hid, name, &val, fraction_t))
        val = TTime();
}

template<>
void attribute_read(hid_t hid, const char *name, std::string& val)
{
    char *c_buf = NULL;
    if (attread(hid, name, &c_buf, string_t)) val = c_buf;
    else val = "";
    free(c_buf);
}

template<>
void attribute_read(hid_t hid, const char *name, TEval& val)
{
    std::string buf;
    attribute_read<std::string>(hid, name, buf);
    val = buf.c_str();
}

template<>
void attribute_read(hid_t hid, const char *name, bc_t& bc)
{
    uint32_t bc_raw = 0;
    attread(hid, name, &bc_raw, H5T_NATIVE_UINT32);
    switch (bc_raw)
    {
        case 0: bc = bc_t::steady; break;
        case 1: bc = bc_t::kutta; break;
        default:
                fprintf(stderr, "Warning: bad boundary condition (%d), using bc::steady\n", bc_raw);
                bc = bc_t::steady;
    }
}

template<>
void attribute_read(hid_t hid, const char *name, hc_t& hc)
{
    uint32_t hc_raw = 0;
    attread(hid, name, &hc_raw, H5T_NATIVE_UINT32);
    switch (hc_raw)
    {
        case 0: hc = hc_t::neglect; break;
        case 1: hc = hc_t::isolate; break;
        case 2: hc = hc_t::const_t; break;
        case 3: hc = hc_t::const_w; break;
        default:
                fprintf(stderr, "Warning: bad heat condition (%d), using hc::neglect\n", hc_raw);
                hc = hc_t::neglect;
    }
}

// ############################################################################
// **** WRITE *****************************************************************
// ############################################################################

template<typename T> void attribute_write(hid_t, const char*, T value);

void attwrite(hid_t hid, const char *name, void *value, hid_t type_id)
{
    hid_t aid = H5Acreate2(hid, name, type_id, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
    if (H5Awrite(aid, type_id, value)<0) return;
    if (H5Aclose(aid)<0) return;
}

template<>
void attribute_write(hid_t hid, const char *name, uint32_t val)
{
    if (val) attwrite(hid, name, &val, H5T_NATIVE_UINT32);
}

template<>
void attribute_write(hid_t hid, const char *name, int32_t val)
{
    if (val) attwrite(hid, name, &val, H5T_NATIVE_INT32);
}

template<>
void attribute_write(hid_t hid, const char *name, double val)
{
    if (val) attwrite(hid, name, &val, H5T_NATIVE_DOUBLE);
}

template<>
void attribute_write(hid_t hid, const char *name, bc_t bc)
{
    if (bc == bc_t::steady) return;

    uint32_t bc_raw = 0;
    /**/ if (bc == bc_t::kutta) bc_raw = 1;
    attwrite(hid, name, &bc_raw, H5T_NATIVE_UINT32);
}

template<>
void attribute_write(hid_t hid, const char *name, hc_t hc)
{
    if (hc == hc_t::neglect) return;

    uint32_t hc_raw = 0;
    /**/ if (hc == hc_t::isolate) hc_raw = 1;
    else if (hc == hc_t::const_t) hc_raw = 2;
    else if (hc == hc_t::const_w) hc_raw = 3;
    attwrite(hid, name, &hc_raw, H5T_NATIVE_UINT32);
}

template<>
void attribute_write(hid_t hid, const char *name, std::string str)
{
    if (str.empty()) return;
    if (!commited_string)
    {
        H5Tcommit2(fid, "string_t", string_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        commited_string = true;
    }

    hid_t aid = H5Acreate2(hid, name, string_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
    H5ASSERT(aid, "H5Acreate");
    const char* c_str = str.c_str();
    H5ASSERT(H5Awrite(aid, string_t, &c_str), "H5Awrite");
    H5ASSERT(H5Aclose(aid), "H5Aclose");
}

template<>
void attribute_write(hid_t hid, const char *name, TTime tval)
{
    if (!tval.value) return;
    if (!commited_fraction)
    {
        commited_fraction = true;
        H5Tcommit2(fid, "fraction_t", fraction_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }

    hid_t aid = H5Acreate2(hid, name, fraction_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
    H5ASSERT(aid, "H5Acreate");
    H5ASSERT(H5Awrite(aid, fraction_t, &tval), "H5Awrite");
    H5ASSERT(H5Aclose(aid), "H5Aclose");
}

template<>
void attribute_write(hid_t hid, const char *name, TVec vec)
{
    if (vec.iszero()) return;
    if (!commited_vec)
    {
        commited_vec = true;
        H5Tcommit2(fid, "vec_t", vec_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }

    hid_t aid = H5Acreate2(hid, name, vec_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
    H5ASSERT(aid, "H5Acreate");
    H5ASSERT(H5Awrite(aid, vec_t, &vec), "H5Awrite");
    H5ASSERT(H5Aclose(aid), "H5Aclose");
}

template<>
void attribute_write(hid_t hid, const char *name, TVec3D vec3d)
{
    if (vec3d.iszero()) return;
    if (!commited_vec3d)
    {
        commited_vec3d = true;
        H5Tcommit2(fid, "vec3d_t", vec3d_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }

    hid_t aid = H5Acreate2(hid, name, vec3d_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
    H5ASSERT(aid, "H5Acreate");
    H5ASSERT(H5Awrite(aid, vec3d_t, &vec3d), "H5Awrite");
    H5ASSERT(H5Aclose(aid), "H5Aclose");
}

// ############################################################################
// **** DATATYPES *************************************************************
// ############################################################################

void datatypes_create_all()
{
    commited_string = false;
    commited_fraction = false;
    commited_bool = false;
    commited_vec = false;
    commited_vec3d = false;

    string_t = H5Tcopy(H5T_C_S1);
    H5Tset_size(string_t, H5T_VARIABLE);

    fraction_t = H5Tcreate(H5T_COMPOUND, 8);
    H5Tinsert(fraction_t, "value", 0, H5T_STD_I32LE);
    H5Tinsert(fraction_t, "timescale", 4, H5T_STD_U32LE);

    bool_t = H5Tenum_create(H5T_NATIVE_INT);
    {int val = 0; H5Tenum_insert(bool_t, "FALSE", &val);}
    {int val = 1; H5Tenum_insert(bool_t, "TRUE", &val);}

    vec_t = H5Tcreate(H5T_COMPOUND, 16);
    H5Tinsert(vec_t, "x", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(vec_t, "y", 8, H5T_NATIVE_DOUBLE);
    H5Tpack(vec_t);

    vec3d_t = H5Tcreate(H5T_COMPOUND, 24);
    H5Tinsert(vec3d_t, "x", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(vec3d_t, "y", 8, H5T_NATIVE_DOUBLE);
    H5Tinsert(vec3d_t, "o", 16, H5T_NATIVE_DOUBLE);
    H5Tpack(vec3d_t);
}

void datatypes_close_all()
{
    if (string_t>0) { H5Tclose(string_t); string_t = 0; }
    if (fraction_t>0) { H5Tclose(fraction_t); fraction_t = 0; }
    if (bool_t>0) { H5Tclose(bool_t); bool_t = 0; }
    if (vec_t>0) { H5Tclose(vec_t); vec_t = 0; }
    if (vec3d_t>0) { H5Tclose(vec3d_t); vec3d_t = 0; }
}

#undef H5ASSERT
