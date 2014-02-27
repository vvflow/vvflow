#include "stepdata.h"
#include "assert.h"
#include "hdf5.h"

Stepdata::Stepdata(Space* s_)
{
	S = s_;
	fid = -1;
	string_hid = -1;
	DATASPACE_SCALAR = -1;
}

void Stepdata::attribute_write(const char *name, const char *str)
{
	hid_t aid = H5Acreate2(fid, name, string_hid, DATASPACE_SCALAR, H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, string_hid, &str)>=0);
	assert(H5Aclose(aid)>=0);
}

void Stepdata::open(const char *filename)
{
	fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	DATASPACE_SCALAR = H5Screate(H5S_SCALAR);

	string_hid = H5Tcopy(H5T_C_S1);
	H5Tset_size(string_hid, H5T_VARIABLE);
	H5Tcommit2(fid, "string_t", string_hid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	attribute_write("caption", S->caption.c_str());
	attribute_write("git_info", S->getGitInfo());
	attribute_write("git_diff", S->getGitDiff());	
}

void Stepdata::write()
{

}

void Stepdata::close()
{
	H5Tclose(string_hid); string_hid = -1;
	H5Fclose(fid); fid = -1;
	H5Sclose(DATASPACE_SCALAR); DATASPACE_SCALAR = -1;
}


