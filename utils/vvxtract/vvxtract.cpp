#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core.h"
#include "vvhdf.h"

int main(int argc, char **argv)
{
    int b_xyvalue = 0;
    int b_margins = 0;
    int b_bitness = 0;
    int b_info = 0;
    int b_list = 0;

    while (1) {
        static struct option long_options[] =
        {
            {"version", no_argument,       0,        'v'},
            {"help",    no_argument,       0,        'h'},
            {"format",  required_argument, 0,        'f'},
            {"info",    no_argument,       &b_info,    1},
            {"list",    no_argument,       &b_list,    1},
            {"xyvalue", no_argument,       &b_xyvalue, 1},
            {"margins", no_argument,       &b_margins, 1},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "vhf:", long_options, &option_index);

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
        case 'f':
            /**/ if (!strcmp(optarg, "text"))   b_bitness = 0;
            else if (!strcmp(optarg, "float"))  b_bitness = 4;
            else if (!strcmp(optarg, "double")) b_bitness = 8;
            else {
                fprintf(stderr, "%s: invalid argument --format '%s', ", argv[0], optarg);
                fprintf(stderr, "must be 'text', 'float' or 'double'\n");
                return -1;
            }
            break;
        case '?':
            break;
        case 0:
            if (long_options[option_index].flag != 0) break;
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
        fprintf(stderr, "caption: %s\n", h5a_read<std::string>(fid, "caption").c_str());
        fprintf(stderr, "created: %s\n", h5a_read<std::string>(fid, "time_local").c_str());
        fprintf(stderr, "version: %s\n", h5a_read<std::string>(fid, "git_info").c_str());
        fprintf(stderr, "git_rev: %s\n", h5a_read<std::string>(fid, "git_rev").c_str());
        std::string git_diff = h5a_read<std::string>(fid, "git_diff");
        if (!git_diff.empty()) {
            fprintf(stderr, "git_diff: %s\n", git_diff.c_str());
        }
        return 0;
    }

    return 0;
}
