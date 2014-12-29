#include "stepdata.h"
#include "assert.h"
#include "hdf5.h"
#include "body.h"

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

	int blsize = S->BodyList.size();
	born_d_hid = new int[blsize];
	hydro_d_hid = new int[blsize];
	holder_d_hid = new int[blsize];
	friction_d_hid = new int[blsize];
	nusselt_d_hid = new int[blsize];
	position_d_hid = new int[blsize];
	spring_d_hid = new int[blsize];
	speed_d_hid = new int[blsize];

	time_d_hid = create_dataset(file_hid, "time", 1);
	for (auto& lbody: S->BodyList)
	{
		int body_n = lbody->get_index();
		hid_t body_g_hid = H5Gcreate2(file_hid, lbody->get_name().c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

		born_d_hid[body_n] = create_dataset(body_g_hid, "force_born", 3);
		hydro_d_hid[body_n] = create_dataset(body_g_hid, "force_hydro", 3);
		holder_d_hid[body_n] = create_dataset(body_g_hid, "force_holder", 3);
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

	for (auto& lbody: S->BodyList)
	{
		int body_n = lbody->get_index();
		append(born_d_hid[body_n], lbody->force_born - lbody->force_dead);
		append(hydro_d_hid[body_n], lbody->force_hydro);
		append(holder_d_hid[body_n], lbody->force_holder);
		append(friction_d_hid[body_n], lbody->friction);
		append(nusselt_d_hid[body_n], lbody->nusselt);
		append(position_d_hid[body_n], lbody->holder);
		append(spring_d_hid[body_n], lbody->dpos);
		append(speed_d_hid[body_n], lbody->speed_slae);
	}

	H5Fflush(file_hid, H5F_SCOPE_GLOBAL);
}

void Stepdata::close()
{
	H5Dclose(time_d_hid);

	H5Tclose(string_hid); string_hid = -1;
	H5Sclose(DATASPACE_SCALAR); DATASPACE_SCALAR = -1;
	H5Fclose(file_hid); file_hid = -1;

	delete [] born_d_hid;
	delete [] hydro_d_hid;
	delete [] holder_d_hid;
	delete [] friction_d_hid;
	delete [] nusselt_d_hid;
	delete [] position_d_hid;
	delete [] spring_d_hid;
	delete [] speed_d_hid;
}


