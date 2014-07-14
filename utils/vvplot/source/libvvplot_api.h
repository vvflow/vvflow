#ifndef __LIBVVPLOT_API__
#define __LIBVVPLOT_API__

#include "hdf5.h"

inline
void attribute_read_double(hid_t hid, const char *name, double &value)
{
	if (!H5Aexists(hid, name)) { value = 0; return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	if (aid<0) { return; }

	H5Aread(aid, H5T_NATIVE_DOUBLE, &value);
	H5Aclose(aid);
}

extern "C" {
	bool map_check(hid_t fid, const char *dsetname,
		double xmin, double xmax, double ymin, double ymax, double spacing);
	int map_save(hid_t fid, const char *dsetname, const float* data, const hsize_t *dims,
		double xmin, double xmax, double ymin, double ymax, double spacing);
	int map_extract(hid_t fid, const char *dsetname);
	int list_extract(hid_t fid, const char *dsetname);
	int map_isoline(hid_t fid, const char *dsetname, float *_cvals, int _cnum);

	int map_pressure(hid_t fid, char RefFrame, double xmin, double xmax, double ymin, double ymax, double spacing);
	int map_streamfunction(hid_t fid, char RefFrame, double xmin, double xmax, double ymin, double ymax, double spacing);
	int map_vorticity(hid_t fid, double xmin, double xmax, double ymin, double ymax, double spacing);
}

#endif