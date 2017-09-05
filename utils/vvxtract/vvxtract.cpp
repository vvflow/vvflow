#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <vector>

#include "core.h"
#include "vvhdf.h"

struct dataset_t {
    size_t cols;
    size_t rows;
    const char* label;
    std::vector<double> mem;
};

int main(int argc, char **argv)
{
    int b_xyvalue = 0;
    int b_margins = 0;
    int b_info = 0;
    int b_list = 0;

    while (1) {
        static struct option long_options[] =
        {
            {"version", no_argument, 0, 'v'},
            {"help",    no_argument, 0, 'h'},
            {"info",    no_argument, 0, 'i'},
            {"list",    no_argument, 0, 'l'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "vhli", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v':
            fprintf(stderr, "vvxtract %s\n", libvvhd_gitinfo);
            fprintf(stderr, "revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] == '\0') return 0;
            fprintf(stderr, "git_diff: %s\n", libvvhd_gitdiff);
            return 0;
        case 'h':
            return execlp("man", "man", "vvxtract", NULL);
        case 'i':
            b_info = 1;
            break;
        case 'l':
            b_list = 1;
            break;
        case '?':
        default:
            exit(1);
        }
    }

    if (argc-optind < 1) {
        fprintf(stderr, "%s: not enough arguments: missing FILE\n", argv[0]);
    }

    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
    hid_t fid = H5Fopen(argv[optind], H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fid < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "%s: can't open file '%s'\n", argv[0], argv[optind]);
        return 2;
    }

    if (argc-optind < 2) {
        b_info = 1;
    }

    if (b_list) {
        H5O_iterate_t h5o_iter = [](hid_t, const char* name, const H5O_info_t* info, void *) -> herr_t
        {
            if (info->type == H5O_TYPE_DATASET) {
                printf("%s\n", name);
            }
            return 0;
        };
        H5Ovisit(fid, H5_INDEX_NAME, H5_ITER_NATIVE, h5o_iter, NULL);
        return 0;
    }

    if (b_info) {
        printf("caption: %s\n", h5a_read<std::string>(fid, "caption").c_str());
        printf("created: %s\n", h5a_read<std::string>(fid, "time_local").c_str());
        printf("version: %s\n", h5a_read<std::string>(fid, "git_info").c_str());
        printf("git_rev: %s\n", h5a_read<std::string>(fid, "git_rev").c_str());
        std::string git_diff = h5a_read<std::string>(fid, "git_diff");
        if (!git_diff.empty()) {
            printf("git_diff: %s\n", git_diff.c_str());
        }
        return 0;
    }

    std::vector<dataset_t> datasets;
    size_t max_rows = 0;
    for (int i=optind+1; i<argc; i++) {
        datasets.emplace_back();
        dataset_t& dat = datasets.back();
        dat.label = argv[i];

        hid_t dataset = H5Dopen2(fid, dat.label, H5P_DEFAULT);
        if (dataset < 0)
        {
            H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
            H5Eprint2(H5E_DEFAULT, stderr);
            fprintf(stderr, "can't open dataset '%s'\n", dat.label);
            return 3;
        }

        hid_t dataspace = H5Dget_space(dataset);
        if (dataspace < 0)
        {
            H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
            H5Eprint2(H5E_DEFAULT, stderr);
            fprintf(stderr, "can't open dataspace '%s'\n", dat.label);
            return 5;
        }

        int ndims = H5Sget_simple_extent_ndims(dataspace);
        if (ndims < 0) {
            H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
            H5Eprint2(H5E_DEFAULT, stderr);
            fprintf(stderr, "can't read dataspace '%s'\n", dat.label);
            return 5;
        } else if (ndims != 2) {
            fprintf(stderr, "can't read dataspace '%s': only 2D datasets supported (have %dD)\n", dat.label, ndims);
            return 5;
        }

        hsize_t dims[ndims];
        H5Sget_simple_extent_dims(dataspace, dims, NULL);
        dat.rows = dims[0];
        dat.cols = dims[1];
        dat.mem.resize(dat.rows*dat.cols);
        max_rows = std::max(max_rows, dat.rows);

        herr_t err = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dat.mem.data());
        if (err < 0)
        {
            H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
            H5Eprint2(H5E_DEFAULT, stderr);
            return 5;
        }
    }

    printf("#");
    for (size_t i=0; i<datasets.size(); i++) {
        dataset_t& dat = datasets[i];
        for (size_t j=0; j<dat.cols; j++) {
            printf("%s", dat.label);
            if (dat.cols>1) {
                printf("[%zd]", j+1);
            }
            printf("\t");
        }
    }
    printf("\n");

    for (size_t row=0; row<max_rows; row++) {
        for (dataset_t& dat: datasets) {
            for (size_t j=0; j<dat.cols; j++) {
                if (row>=dat.rows) {
                    printf("             \t");
                } else {
                    printf("%+.6le\t", dat.mem[row*dat.cols+j]);
                }
            }
        }
        printf("\n");
    }

    return 0;
}
