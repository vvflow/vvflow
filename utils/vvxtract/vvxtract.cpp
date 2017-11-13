#include "TSpace.hpp"
#include "vvhdf.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#include <vector>
#include <string>

using std::shared_ptr;

struct dataset_t {
    size_t cols;
    size_t rows;
    const char* label;
    std::vector<double> mem;

    dataset_t() = delete;
    dataset_t(const char* label):
        cols(), rows(),
        label(label),
        mem() {}
    dataset_t(const dataset_t&) = default;
    dataset_t& operator=(const dataset_t&) = default;
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
        std::string git_diff =  h5a_read<std::string>(fid, "git_diff");
        if (!git_diff.empty()) {
            printf("git_diff: %s\n", git_diff.c_str());
        }
    }

    if (b_info && H5Aexists(fid, "dt")) {
        Space S;
        S.load(fid);

        printf("\n");
        printf("-- space\n");
        printf("S.re = %lg\n", S.re);
        printf("S.inf_g = %lg\n", S.inf_g);
        printf("S.inf_vx = '%s'\n", std::string(S.inf_vx).c_str());
        printf("S.inf_vy = '%s'\n", std::string(S.inf_vy).c_str());
        printf("S.gravity = {%lg, %lg}\n", S.gravity.x, S.gravity.y);
        printf("S.time = '%d/%u' -- %lg\n", S.time.value, S.time.timescale, double(S.time));
        printf("S.dt = '%d/%u' -- %lg\n", S.dt.value, S.dt.timescale, double(S.dt));
        printf("S.dt_save = '%d/%u'\n", S.dt_save.value, S.dt_save.timescale);
        printf("S.dt_streak = '%d/%u'\n", S.dt_streak.value, S.dt_streak.timescale);
        printf("S.dt_profile = '%d/%u'\n", S.dt_profile.value, S.dt_profile.timescale);
        printf("S.finish = %lg\n", S.finish);
        printf("-- #S.body_list = %zd -- number of elements\n", long(S.BodyList.size()));
        printf("-- #S.vort_list = %zd\n", long(S.VortexList.size()));
        printf("-- #S.sink_list = %zd\n", long(S.SourceList.size()));
        printf("-- #S.streak_source_list = %zd\n", long(S.StreakList.size()));
        printf("-- #S.streak_domain_list = %zd\n", long(S.StreakSourceList.size()));

        for (shared_ptr<TBody>& lbody: S.BodyList)
        {
            const TBody& b = *lbody;
            printf("\n");
            const char* blabel;
            printf("-- %s (%s)", S.get_body_name(&b).c_str(), b.label.c_str());
            if (!b.label.empty()){
                blabel = b.label.c_str();
            } else {
                blabel = S.get_body_name(&b).c_str();
            }
            printf("\n");
            #define S2D(v) v.x, v.y
            #define S3D(v) v.r.x, v.r.y, v.o
            #define S4D(v) v.r.x, v.r.y, v.o, v.o*180./C_PI
            printf("%s.label = '%s'\n", blabel, b.label.c_str());
            printf("%s.density = %lg\n", blabel, b.density);
            printf("%s.holder_pos = {%lg, %lg, %lg} -- .d = %lg\n", blabel, S4D(b.holder));
            printf("%s.holder_vx = '%s'\n", blabel, std::string(b.speed_x).c_str());
            printf("%s.holder_vy = '%s'\n", blabel, std::string(b.speed_y).c_str());
            printf("%s.holder_vo = '%s'\n", blabel, std::string(b.speed_o).c_str());
            printf("%s.speed = {%lg, %lg, %lg}\n", blabel, S3D(b.speed_slae));
            printf("%s.delta_pos = {%lg, %lg, %lg} -- .d = %lg\n", blabel, S4D(b.dpos));
            printf("%s.spring_const = {%lg, %lg, %lg}\n", blabel, S3D(b.kspring));
            printf("%s.spring_damping = {%lg, %lg, %lg}\n", blabel, S3D(b.damping));
            printf("%s.collision_min = {%lg, %lg, %lg} -- .d = %lg\n", blabel, S4D(b.collision_min));
            printf("%s.collision_max = {%lg, %lg, %lg} -- .d = %lg\n", blabel, S4D(b.collision_max));
            printf("%s.bounce = %lg\n", blabel, b.bounce);

            uint32_t slip = b.alist.begin()->slip;
            for (auto& latt: b.alist) {
                if (latt.slip != slip) {
                    slip = 3;
                    break;
                }
            }
            printf("%s.slip = %s\n", blabel, (slip==0)?"false -- no-slip":(slip==1)?"true -- slip":"nil -- mixed");
            printf("%s.special_segment = %d\n", blabel, b.special_segment_no);

            if (b.root_body.expired()) {
                printf("%s.root = nil\n", blabel);
            } else {
                shared_ptr<TBody> lroot = b.root_body.lock();
                const char* rlabel;
                if (!lroot->label.empty()){
                    rlabel = lroot->label.c_str();
                } else {
                    rlabel = S.get_body_name(lroot.get()).c_str();
                }
                printf("%s.root = %s\n", blabel, rlabel);
            }

            printf("-- #%s = %zd -- number of segments\n", blabel, b.size());
            printf("-- %s.get_axis() = {%lg, %lg} -- rotation axis\n", blabel, S2D(b.get_axis()));
            printf("-- %s.get_cofm() = {%lg, %lg} -- center of mass\n", blabel, S2D(b.get_cofm()));
            printf("-- %s.get_slen() = %lg -- surface length\n", blabel, b.get_slen());
            printf("-- %s.get_area() = %lg\n", blabel, b.get_area());
            printf("-- %s.get_moi_cofm() = %lg\n", blabel, b.get_moi_cofm());
            printf("-- %s.get_moi_axis() = %lg\n", blabel, b.get_moi_axis());
        }
        return 0;
    }

    std::vector<dataset_t> datasets;
    size_t max_rows = 0;
    for (int i=optind+1; i<argc; i++) {
        datasets.emplace_back(argv[i]);
        dataset_t& dat = datasets.back();

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

    if (datasets.size()) printf("#");
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
    if (datasets.size()) printf("\n");

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
