#include "core.h"
#include "string.h"
#include "stdlib.h"
#include "libvvplot_api.h"

void print_version()
{
	fprintf(stderr, "libvvplot.so compiled with:\n");
	fprintf(stderr, " - libvvhd git_commit %s\n", Space().getGitInfo());
	unsigned ver[3];
	H5get_libversion(&ver[0], &ver[1], &ver[2]);
	fprintf(stderr, " - libhdf version %u.%u.%u\n", ver[0], ver[1], ver[2]);
	fflush(stderr);
}

void print_help()
{
	fprintf(stderr, "Usage: libvvplot {-h,-v,-p,-M,-m,-V,-L,-I} FILE DATASET [ARGS]\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, " -h : show this message\n");
	fprintf(stderr, " -v : show version info\n");
	fprintf(stderr, " -p : print dataset from hdf file in plain text\n");
	fprintf(stderr, " -M : extract a binary dataset from hdf file (x, y, value)\n");
	fprintf(stderr, " -m : extract a binary dataset from hdf file (binary matrix)\n");
	fprintf(stderr, " -V : calculate velocity args of format px,py\n");
	fprintf(stderr, " -L : extract a list of domains from hdf file\n");
	fprintf(stderr, " -I : plot isolines on a dataset with constants in args\n");
	fflush(stderr);
}


int main(int argc, char **argv)
{
	/**/ if (argc<2) { print_help(); exit(1); }
	else if (!strcmp(argv[1], "-h")) { print_help(); exit(0); }
	else if (!strcmp(argv[1], "-v")) { print_version(); exit(0); }
	else if (argc < 4) { print_help(); exit(1); }
	else if (!strcmp(argv[1], "-I") || !strcmp(argv[1], "-p") ||
	         !strcmp(argv[1], "-M") || !strcmp(argv[1], "-L") ||
             !strcmp(argv[1], "-m") || !strcmp(argv[1], "-V")) {;}
	else {fprintf(stderr, "Bad option '%s'. See '-h' for help.\n", argv[1]); exit(-1); }

	H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
	hid_t fid = H5Fopen(argv[2], H5F_ACC_RDONLY, H5P_DEFAULT);
	if (fid < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument file: can't open file '%s'\n", argv[2]);
		return 2;
	}

	if (!strcmp(argv[1], "-p"))
	{
		dset_print(fid, argv[3]);
	}
	else if (!strcmp(argv[1], "-M"))
	{
		map_extract(fid, argv[3], binary_mode::xyvalue);
	}
	else if (!strcmp(argv[1], "-m"))
	{
		map_extract(fid, argv[3], binary_mode::matrix);
	}
	else if (!strcmp(argv[1], "-V"))
	{
        TVec* points = (TVec*)malloc((argc-4)*sizeof(TVec));
        for (int i=4; i<argc; i++)
        {
            int len;
            if (sscanf(argv[i], "%lf,%lf%n", &points[i-4].x, &points[i-4].y, &len)!=2 || argv[i][len])
            {
                fprintf(stderr, "error: bad point %s: the format is 'px,py'\n", argv[i]);
                exit(-1);
            }
        }
		velocity_print(fid, points, argc-4);
        free(points);
	}
	else if (!strcmp(argv[1], "-L"))
	{
		list_extract(fid, argv[3]);
	}
	else if (!strcmp(argv[1], "-I"))
	{
		float* vals = (float*)malloc((argc-4)*sizeof(float));
		for (int i=4; i<argc; i++)
        {
            int len;
            if (sscanf(argv[i], "%f%n", &vals[i-4], &len)!=1 || argv[i][len])
            {
                fprintf(stderr, "error: argument should be float");
                exit(-1);
            }
        }
		map_isoline(fid, argv[3], vals, argc-4);
        free(vals);
	}
	H5Fclose(fid);

	exit(0);
}

