#include "stdio.h"
#include "unistd.h"
#include "malloc.h"
#include "assert.h"
#include "hdf5.h"
#include "core.h"

const char interp[] __attribute__((section(".interp"))) = "/lib64/ld-linux-x86-64.so.2";
static char buffer[256];
Space *S = NULL;
hid_t fid;

inline
void get_args(int *argc, char **argv)
{
	FILE* argf = fopen("/proc/self/cmdline", "rb");
	size_t len = fread(buffer, 1, sizeof(buffer), argf);
	// for (int i=0; i<256; i++) printf("%c", buffer[i]?buffer[i]:'*'); printf("\n");
	for (char *p1=buffer, *p2=buffer; p1 < buffer+len; p1++)
	{
		if (*p1==0)
		{
			argv[(*argc)++] = p2;
			p2=p1+1;
		}
	}
	fclose(argf);
}

static hid_t DATASPACE_SCALAR;
inline
void attribute_write(hid_t hid, const char *name, double value)
{
	if (value == 0) return;
	hid_t aid = H5Acreate2(hid, name, H5T_NATIVE_DOUBLE, DATASPACE_SCALAR, H5P_DEFAULT, H5P_DEFAULT);
	assert(aid>=0);
	assert(H5Awrite(aid, H5T_NATIVE_DOUBLE, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

inline
void attribute_read_double(hid_t hid, const char *name, double &value)
{
	if (!H5Aexists(hid, name)) { value = 0; return; }

	hid_t aid = H5Aopen(hid, name, H5P_DEFAULT);
	assert(aid>=0);

	assert(H5Aread(aid, H5T_NATIVE_DOUBLE, &value)>=0);
	assert(H5Aclose(aid)>=0);
}

extern "C" {
bool map_check(
	const char *filename,
	const char *dsetname,
	double xmin,
	double xmax,
	double ymin,
	double ymax,
	double spacing)
{
	hid_t fid = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (fid < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument file: can't open file '%s'\n", filename);
		exit(-1);
	}
	if (!H5Lexists(fid, dsetname, H5P_DEFAULT)) return false;
	hid_t dataset = H5Dopen2(fid, dsetname, H5P_DEFAULT);
	if (dataset < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument dataset: can't open dataset '%s'\n", dsetname);
		exit(-1);
	}

	double attr[5];
	attribute_read_double(dataset, "xmin", attr[0]);
	attribute_read_double(dataset, "xmax", attr[1]);
	attribute_read_double(dataset, "ymin", attr[2]);
	attribute_read_double(dataset, "ymax", attr[3]);
	attribute_read_double(dataset, "spacing", attr[4]);

	bool result = true;
	result &= attr[0] <= xmin;
	result &= attr[1] >= xmax;
	result &= attr[2] <= ymin;
	result &= attr[3] >= ymax;
	result &= attr[4] <= spacing;
	H5Fclose(fid);
	fprintf(stderr, "%s %s %lf %lf %lf %lf %lf\n", filename, dsetname, xmin, xmax, ymin, ymax, spacing);
	fprintf(stderr, "%lf %lf %lf %lf %lf\n", attr[0], attr[1], attr[2], attr[3], attr[4]);
	fprintf(stderr, "returning %s\n", result?"true":"false");
	return result;
}}

extern "C" {
int map_save(
	const char *filename,
	const char *dsetname,
	const float* data, const hsize_t *dims,
	double xmin,
	double xmax,
	double ymin,
	double ymax,
	double spacing)
{
	// Create HDF file
	H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
	hid_t fid = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (fid < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument file: can't open file '%s'\n", filename);
		return 2;
	}

	// Create dataspace and dataset
	DATASPACE_SCALAR = H5Screate(H5S_SCALAR);
	hid_t file_dataspace = H5Screate_simple(2, dims, dims);
	if (file_dataspace < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}
	if (H5Lexists(fid, dsetname, H5P_DEFAULT)) H5Ldelete(fid, dsetname, H5P_DEFAULT);
	// H5Fclose(fid);
	// return 0;

	hid_t file_dataset = H5Dcreate2(fid, dsetname, H5T_NATIVE_FLOAT, file_dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if (file_dataset < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}

	attribute_write(file_dataset, "xmin", xmin);
	attribute_write(file_dataset, "xmax", xmax);
	attribute_write(file_dataset, "ymin", ymin);
	attribute_write(file_dataset, "ymax", ymax);
	attribute_write(file_dataset, "spacing", spacing);
	
	herr_t err = H5Dwrite(file_dataset, H5T_NATIVE_FLOAT, H5S_ALL, file_dataspace, H5P_DEFAULT, data);
	if (err < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}
	H5Dclose(file_dataset);
	H5Sclose(DATASPACE_SCALAR);
	H5Sclose(file_dataspace);
	H5Fclose(fid);

	return 0;
}}

extern "C" {
int map_extract(const char *filename, const char *dsetname)
{
	H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
	fid = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (fid < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument file: can't open file '%s'\n", filename);
		return 2;
	}

	hid_t dataset = H5Dopen2(fid, dsetname, H5P_DEFAULT);
	if (dataset < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument dataset: can't open dataset '%s'\n", dsetname);
		return 3;
	}

	double args[5];
	attribute_read_double(dataset, "xmin", args[0]);
	attribute_read_double(dataset, "xmax", args[1]);
	attribute_read_double(dataset, "ymin", args[2]);
	attribute_read_double(dataset, "ymax", args[3]);
	attribute_read_double(dataset, "spacing", args[4]);

	hid_t dataspace = H5Dget_space(dataset);
	if (dataspace < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}
	hsize_t dims[2];
	H5Sget_simple_extent_dims(dataspace, dims, dims);
	float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);
	herr_t err = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, dataspace, H5P_DEFAULT, mem);
	if (err < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}
	
	// fprintf(stdout, "%lf %lf %lf %lf %lf\n", xmin, xmax, ymin, ymax, spacing);
	// fwrite(args, sizeof(double), 5, stdout);
	for (int i=0; i<dims[0]; i++)
	for (int j=0; j<dims[1]; j++)
	{
		float x = args[0] + i*args[4];
		float y = args[2] + j*args[4];
		fwrite(&x, sizeof(float), 1, stdout);
		fwrite(&y, sizeof(float), 1, stdout);
		fwrite(mem+i*dims[1]+j, sizeof(float), 1, stdout);
	}
	fflush(stdout);
	// assert(c == dims[0]*dims[1]);
	// fwrite(dims, sizeof(hsize_t), 2, stdout);
	// fwrite(&xmin, sizeof(double), 1, stdout);

	H5Sclose(dataspace);
	H5Dclose(dataset);
	H5Fclose(fid);
	// printf("%s\n", argv[0]);
	return 0;

}}

// extern "C" {
int main()
{
	int argc = 0;
	char *argv[256];
	get_args(&argc, argv);

	// for (int i=0; i<argc; i++)
	// {
	// 	printf("%d %s\n", i, argv[i]);
	// }

	if (argc != 3)
	{
		fprintf(stderr, "libvvplot.so compiled with:\n");
		fprintf(stderr, " - libvvhd git_commit %s\n", Space().getGitInfo());
		unsigned ver[3];
		H5get_libversion(&ver[0], &ver[1], &ver[2]);
		fprintf(stderr, " - libhdf version %u.%u.%u\n", ver[0], ver[1], ver[2]);
		fprintf(stderr, "It can be executed to extract a map from hdf file:\n   usage: libvvplot.so file dataset\n");
		_exit(1);
	}

	map_extract(argv[1], argv[2]);

	_exit(0);
}//}

