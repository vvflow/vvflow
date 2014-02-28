#include "stepdata.h"
#include "assert.h"
#include "hdf5.h"

Stepdata::Stepdata(Space* s_)
{
	S = s_;
	file_hid = -1;
	string_hid = -1;
	DATASPACE_SCALAR = -1;
}

void Stepdata::attribute_write(const char *name, const char *str)
{
	hid_t aid = H5Acreate2(file_hid, name, string_hid, DATASPACE_SCALAR, H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, string_hid, &str)>=0);
	assert(H5Aclose(aid)>=0);
}

void Stepdata::append(int dataspace_hid, const void *buf)
{
	if (dataspace_hid < 0) return;
	hsize_t dims[2];
	hsize_t ext[2];
	hsize_t offset[2];
	hid_t filespace = H5Dget_space(dataspace_hid);
	H5Sget_simple_extent_dims(filespace, dims, NULL);
	offset[0] = dims[0];
	offset[1] = 0;
	ext[0] = 1;
	ext[1] = dims[1];
	dims[0]++;
	H5Dset_extent(dataspace_hid, dims);
	filespace = H5Dget_space(dataspace_hid);
	H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, ext, NULL);

	hid_t memspace = H5Screate_simple(2, ext, NULL);
	H5Dwrite(dataspace_hid, H5T_NATIVE_DOUBLE, memspace, filespace, H5P_DEFAULT, buf);

	H5Sclose(filespace);
	H5Sclose(memspace);
}

void Stepdata::append(int dataspace_hid, double value)
{
	append(dataspace_hid, &value);
}

void Stepdata::append(int dataspace_hid, TVec3D value)
{
	double arr[3] = {value.r.x, value.r.y, value.o};
	append(dataspace_hid, arr);
}

int Stepdata::create_dataset(int loc_id, const char *name, int cols)
{
	hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
	hsize_t chunkdims[2] = {100, cols};
	H5Pset_chunk(prop, 2, chunkdims);
	H5Pset_deflate(prop, 9);

	hsize_t dims[2] = {0, cols};
	hsize_t maxdims[2] = {H5S_UNLIMITED, cols};
	hid_t time_s = H5Screate_simple(2, dims, maxdims);
	hid_t dataset = H5Dcreate2(loc_id, name, H5T_NATIVE_DOUBLE, time_s, H5P_DEFAULT, prop, H5P_DEFAULT);
	H5Sclose(time_s);
	H5Pclose(prop);

	return dataset;
}

void Stepdata::create(const char *format)
{
	char filename[256]; sprintf(filename, format, S->caption.c_str());
	file_hid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	DATASPACE_SCALAR = H5Screate(H5S_SCALAR);

	string_hid = H5Tcopy(H5T_C_S1);
	H5Tset_size(string_hid, H5T_VARIABLE);
	H5Tcommit2(file_hid, "string_t", string_hid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	attribute_write("caption", S->caption.c_str());
	attribute_write("git_info", S->getGitInfo());
	attribute_write("git_diff", S->getGitDiff());

	force_d_hid = new int[S->BodyList->size_safe()];
	friction_d_hid = new int[S->BodyList->size_safe()];
	nusselt_d_hid = new int[S->BodyList->size_safe()];
	position_d_hid = new int[S->BodyList->size_safe()];
	spring_d_hid = new int[S->BodyList->size_safe()];
	speed_d_hid = new int[S->BodyList->size_safe()];

	time_d_hid = create_dataset(file_hid, "time", 1);
	const_for(S->BodyList, llbody)
	{
		int body_n = S->BodyList->find(llbody);
		char body_name[16];
		sprintf(body_name, "body%02d", body_n);
		hid_t body_g_hid = H5Gcreate(file_hid, body_name, 0);

		force_d_hid[body_n] = create_dataset(body_g_hid, "force", 3);
		friction_d_hid[body_n] = create_dataset(body_g_hid, "friction", 3);
		nusselt_d_hid[body_n] = -1; //create_dataset(body_g_hid, "nusselt", 1);
		position_d_hid[body_n] = create_dataset(body_g_hid, "holder_position", 3);
		spring_d_hid[body_n] = create_dataset(body_g_hid, "delta_position", 3);
		speed_d_hid[body_n] = create_dataset(body_g_hid, "speed_slae", 3);

		H5Gclose(body_g_hid);
	}

	H5Fflush(file_hid, H5F_SCOPE_GLOBAL);
}

void Stepdata::write()
{
	append(time_d_hid, S->Time);

	const_for(S->BodyList, llbody)
	{
		int body_n = S->BodyList->find(llbody);
		append(force_d_hid[body_n], (**llbody).Force_export);
		append(friction_d_hid[body_n], (**llbody).Friction);
		append(nusselt_d_hid[body_n], (**llbody).Nusselt);
		append(position_d_hid[body_n], (**llbody).pos);
		append(spring_d_hid[body_n], (**llbody).dPos);
		append(speed_d_hid[body_n], (**llbody).Speed_slae);
	}

	H5Fflush(file_hid, H5F_SCOPE_GLOBAL);
}

void Stepdata::close()
{
	H5Dclose(time_d_hid);

	H5Tclose(string_hid); string_hid = -1;
	H5Sclose(DATASPACE_SCALAR); DATASPACE_SCALAR = -1;
	H5Fclose(file_hid); file_hid = -1;

	delete [] force_d_hid;
	delete [] friction_d_hid;
	delete [] nusselt_d_hid;
	delete [] position_d_hid;
	delete [] spring_d_hid;
	delete [] speed_d_hid;
}


