#include <assert.h>
#include "hdf5.h"

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
static hid_t bc_t;       bool commited_bc = false;
static hid_t hc_t;       bool commited_hc = false;
// static hid_t att_detailed_t;

static const hsize_t numbers[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

struct ATT {
	double x, y, g, gsum;
};

/*struct ATT_DETAILED {
	double x, y, g, gsum;
	bc::BoundaryCondition bc;
	hc::HeatCondition hc;
	double heat_const;
};*/

/******************************************************************************
***** STRING ******************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, const char *str)
{
	if (!strlen(str)) return;
	if (!commited_string)
	{
		H5Tcommit2(fid, "string_t", string_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		commited_string = true;
	}

	hid_t aid = H5Acreate2(hid, name, string_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, string_t, &str)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, std::string &buf)
{
	if (!H5Aexists(hid, name)) { buf = ""; return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	char *c_buf;
	assert(H5Aread(aid, string_t, &c_buf)>=0);
	buf = c_buf;
	free(c_buf);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** FRACTION ****************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, TTime time)
{
	if (time.value == INT32_MAX) return;
	if (!commited_fraction)
	{
		commited_fraction = true;
		H5Tcommit2(fid, "fraction_t", fraction_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}

	hid_t aid = H5Acreate2(hid, name, fraction_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, fraction_t, &time)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, TTime &time)
{
	if (!H5Aexists(hid, name)) { time = TTime(INT32_MAX, 1); return; }
	
	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, fraction_t, &time)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** BOOL ********************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, bool value)
{
	if (!commited_bool)
	{
		commited_bool = true;
		H5Tcommit2(fid, "bool_t", bool_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}

	int v = value;
	hid_t aid = H5Acreate2(hid, name, bool_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, bool_t, &v)>=0);
	assert(H5Aclose(aid)>=0);
}

bool attribute_read_bool(hid_t hid, const char *name)
{
	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	bool res;
	assert(H5Aread(aid, bool_t, &res)>=0);
	assert(H5Aclose(aid)>=0);
	return res;
}

/******************************************************************************
***** DOUBLE ******************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, double value)
{
	if (value == 0) return;
	hid_t aid = H5Acreate2(hid, name, H5T_NATIVE_DOUBLE, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, H5T_NATIVE_DOUBLE, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, double &value)
{
	if (!H5Aexists(hid, name)) { value = 0; return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, H5T_NATIVE_DOUBLE, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** LONG INT ****************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, long int value)
{
	if (value == 0) return;
	hid_t aid = H5Acreate2(hid, name, H5T_NATIVE_LONG, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, H5T_NATIVE_LONG, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, long int &value)
{
	if (!H5Aexists(hid, name)) { value = 0; return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, H5T_NATIVE_LONG, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** VEC *********************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, TVec vec)
{
	if (vec.iszero()) return;
	if (!commited_vec)
	{
		commited_vec = true;
		H5Tcommit2(fid, "vec_t", vec_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}

	hid_t aid = H5Acreate2(hid, name, vec_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, vec_t, &vec)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, TVec &vec)
{
	if (!H5Aexists(hid, name)) { vec = TVec(0, 0); return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, vec_t, &vec)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** VEC3D *******************************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, TVec3D vec3d)
{
	if (vec3d.iszero()) return;
	if (!commited_vec3d)
	{
		commited_vec3d = true;
		H5Tcommit2(fid, "vec3d_t", vec3d_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}

	hid_t aid = H5Acreate2(hid, name, vec3d_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, vec3d_t, &vec3d)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, TVec3D &vec3d)
{
	if (!H5Aexists(hid, name)) { vec3d = TVec3D(0, 0, 0); return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, vec3d_t, &vec3d)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** BOUNDARY CONDITION ******************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, bc::BoundaryCondition bc)
{
	if (!commited_bc)
	{
		commited_bc = true;
		H5Tcommit2(fid, "boundary_condition_t", bc_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}
	
	hid_t aid = H5Acreate2(hid, name, bc_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, bc_t, &bc)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, bc::BoundaryCondition &bc)
{
	assert(H5Aexists(hid, name));

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, bc_t, &bc)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** HEAT CONDITION **********************************************************
******************************************************************************/

void attribute_write(hid_t hid, const char *name, hc::HeatCondition hc)
{
	if (hc == hc::neglect) return;
	if (!commited_hc)
	{
		commited_hc = true;
		H5Tcommit2(fid, "heat_condition_t", hc_t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}

	hid_t aid = H5Acreate2(hid, name, hc_t, DATASPACE_SCALAR(), H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, hc_t, &hc)>=0);
	assert(H5Aclose(aid)>=0);
}

void attribute_read(hid_t hid, const char *name, hc::HeatCondition &hc)
{
	if(!H5Aexists(hid, name)) { hc = hc::neglect; return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, hc_t, &hc)>=0);
	assert(H5Aclose(aid)>=0);
}

/******************************************************************************
***** DATATYPES ***************************************************************
******************************************************************************/

void datatypes_create_all()
{
	commited_string = false;
	commited_fraction = false;
	commited_bool = false;
	commited_vec = false;
	commited_vec3d = false;
	commited_bc = false;

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

	bc_t = H5Tenum_create(H5T_NATIVE_INT);
	{int val = bc::slip;       H5Tenum_insert(bc_t, "slip",       &val);}
	{int val = bc::noslip;     H5Tenum_insert(bc_t, "noslip",     &val);}
	{int val = bc::zero;       H5Tenum_insert(bc_t, "zero",       &val);}
	{int val = bc::steady;     H5Tenum_insert(bc_t, "steady",     &val);}
	{int val = bc::inf_steady; H5Tenum_insert(bc_t, "inf_steady", &val);}

	hc_t = H5Tenum_create(H5T_NATIVE_INT);
	{int val = hc::neglect;	H5Tenum_insert(hc_t, "neglect", &val);}
	{int val = hc::isolate;	H5Tenum_insert(hc_t, "isolate", &val);}
	{int val = hc::const_t;	H5Tenum_insert(hc_t, "const_t", &val);}
	{int val = hc::const_W;	H5Tenum_insert(hc_t, "const_w", &val);}
}

void datatypes_close_all()
{
	if (string_t>0) { H5Tclose(string_t); string_t = 0; }
	if (fraction_t>0) { H5Tclose(fraction_t); fraction_t = 0; }
	if (bool_t>0) { H5Tclose(bool_t); bool_t = 0; }
	if (vec_t>0) { H5Tclose(vec_t); vec_t = 0; }
	if (vec3d_t>0) { H5Tclose(vec3d_t); vec3d_t = 0; }
	if (bc_t>0) { H5Tclose(bc_t); bc_t = 0; }
	if (hc_t>0) { H5Tclose(hc_t); hc_t = 0; }
}