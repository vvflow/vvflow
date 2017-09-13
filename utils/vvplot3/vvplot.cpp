#include <string.h> /* strerro */
#include <inttypes.h> /* PRIu32 */
#include <math.h> /* isfinite */
#include <sys/stat.h> /* chmod */
#include <unistd.h> /* exec */
#include <errno.h>
#include <fcntl.h> /* O_RDONLY */
#include <getopt.h>

/* libgflags */
// #include <gflags/gflags.h>

/* std::... */
#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <sstream>
#include <memory> //unique_ptr

/* exear */
// #include "exear.h"

/* map_... */
// #include "map_vorticity.h"

/* vvhd */
#include <core.h>

// template<typename ... Args>
// std::string strfmt(const char* format, Args ... args)
// {
//     size_t size = snprintf( nullptr, 0, format, args ... ) + 1;
//     std::unique_ptr<char[]> buf(new char[size]);
//     snprintf(buf.get(), size, format, args ...);
//     return std::string(buf.get(), buf.get() + size - 1);
// }

static const double d_nan = nan("");
static const double d_inf = 1.0l/0.0l;

// main options
static bool   opt_B = false;

static bool   opt_V = false;
static int    opt_Vsize = 6; // <=0: plot dots; >0: plot circles
static double opt_Vcirc = 0.;

static bool   opt_S = false;
static double opt_Smin = d_nan;
static double opt_Smax = d_nan;
static double opt_Sstep = d_nan;

static bool   opt_G = false;
static double opt_Grange = d_nan;

static bool   opt_P = false;
static double opt_Pmin = -1.5;
static double opt_Pmax = +1.0;
// other options
// static char   opt_ref_xy;
// static char   opt_ref_S;
// static char   opt_ref_P;

static int opt_width = 1280;
static int opt_height = 720;
static int opt_gray;
static int opt_colorbox;
static int opt_timelabel;
static int opt_holder;
static int opt_spring;

typedef struct {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
} rect_t;
static rect_t opt_rect = {0.};
static double &xmin = opt_rect.xmin;
static double &xmax = opt_rect.xmax;
static double &ymin = opt_rect.ymin;
static double &ymax = opt_rect.ymax;

typedef struct {
    double dxdy;
    int    xres;
    int    yres;
} mesh_t;
static mesh_t opt_mesh_hi = {0., 640, 0};
static mesh_t opt_mesh_lo = {0., 256, 0};


int main(int argc, char **argv)
{
    while (1) {
        static struct option long_options[] =
        {
            {"version", no_argument, 0, 'v'},
            {"help",    no_argument, 0, 'h'},

            {"no-gray",      no_argument, &opt_gray, 0},
            {   "gray",      no_argument, &opt_gray, 1},
            {"no-colorbox",  no_argument, &opt_colorbox, 0},
            {   "colorbox",  no_argument, &opt_colorbox, 1},
            {"no-timelabel", no_argument, &opt_timelabel, 0},
            {   "timelabel", no_argument, &opt_timelabel, 1},
            {"no-holder",    no_argument, &opt_holder, 0},
            {   "holder",    no_argument, &opt_holder, 1},
            {"no-spring",    no_argument, &opt_spring, 0},
            {   "spring",    no_argument, &opt_spring, 1},

            {"size",   required_argument, 0, 's'},
            {"res-hi", required_argument, 0, 0xff00},
            {"res-lo", required_argument, 0, 0xff01},

            {"B",       no_argument, NULL, 'B'},
            {"V", required_argument, NULL, 'V'},
            {"S", required_argument, NULL, 'S'},
            {"G", required_argument, NULL, 'G'},
            {"P", required_argument, NULL, 'P'},

            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "vhBVSGPx:y:s:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        // printf("DEBUG: '%c', ind=%d, arg=%s\n", c, option_index, optarg);
        bool fail = false;
        int  optn = 0;
        switch (c) {
        case 'v': // -v, --version
            fprintf(stderr, "vvplot %s\n", libvvhd_gitinfo);
            fprintf(stderr, "revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] == '\0') return 0;
            fprintf(stderr, "git_diff: %s\n", libvvhd_gitdiff);
            return 0;
        case 'h': // -h, --help
            return execlp("man", "man", "vvplot", NULL);
        case 'B': // -B, --B
            opt_B = true;
            break;
        case 'V': // -V, --V SIZE
            opt_V = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%d %n", &opt_Vsize, &optn) < 1;
            fail = fail || !(opt_Vsize >= 0); 
            // if (!fail) {
            //     printf("VSIZE -> %d\n", opt_Vsize);
            // }
            break;
        case 'S': // -S, --S SMIN,SMAX,SSTEP
            opt_S = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%lf , %lf , %lf %n", &opt_Smin, &opt_Smax, &opt_Sstep, &optn) < 3;
            fail = fail || !isfinite(opt_Smin);
            fail = fail || !isfinite(opt_Smax);
            fail = fail || !(opt_Smax >= opt_Smin);
            fail = fail || !isfinite(opt_Sstep);
            fail = fail || !(opt_Sstep>0);
            // if (!fail) {
            //     printf("SMIN -> %lf\n", opt_Smin);
            //     printf("SMAX -> %lf\n", opt_Smax);
            //     printf("SSTEP -> %lf\n", opt_Sstep);
            // }
            break;
        case 'G': // -G, --G GAMMA
            opt_G = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%lf %n", &opt_Grange, &optn) < 1;
            fail = fail || !isfinite(opt_Grange); 
            fail = fail || !(opt_Grange > 0);
            // if (!fail) {
            //     printf("GAMMA -> %lf\n", opt_Grange);
            // }
            break;
        case 'P': // -P, --P PMIN,PMAX
            opt_P = true;
            if (!optarg) break;
            fail = sscanf(optarg, "%lf , %lf %n", &opt_Pmin, &opt_Pmax, &optn) < 2;
            fail = fail || !isfinite(opt_Pmin);
            fail = fail || !isfinite(opt_Pmax);
            fail = fail || !(opt_Pmax >= opt_Pmin);
            // if (!fail) {
            //     printf("PMIN -> %lf\n", opt_Pmin);
            //     printf("PMAX -> %lf\n", opt_Pmax);
            // }
            break;
        case 'x': // -x
            fail = sscanf(optarg, "%lf , %lf %n", &xmin, &xmax, &optn) < 2;
            fail = fail || isinf(xmin) || isinf(xmax);
            // if (!fail) {
            //     printf("XMIN -> %lf\n", xmin);
            //     printf("XMAX -> %lf\n", xmax);
            // }
            break;
        case 'y': // -y
            fail = sscanf(optarg, "%lf , %lf %n", &ymin, &ymax, &optn) < 2;
            fail = fail || isinf(ymin) || isinf(ymax);
            // if (!fail) {
            //     printf("YMIN -> %lf\n", ymin);
            //     printf("YMAX -> %lf\n", ymax);
            // }
            break;
        case 's': // --size
            fail = sscanf(optarg, "%dx%d %n", &opt_width, &opt_height, &optn) < 2;
            fail = fail || !(opt_width>=0);
            fail = fail || !(opt_height>=0);
            // if (!fail) {
            //     printf("W -> %d\n", opt_width);
            //     printf("H -> %d\n", opt_height);
            // }
            break;
        case 0xff00: // --res-hi
            fail = sscanf(optarg, "%d %n", &opt_mesh_hi.xres, &optn) < 1;
            fail = fail || !(opt_mesh_hi.xres>0);
            // if (!fail) {
            //     printf("RES_HI -> %d\n", opt_mesh_hi.xres);
            // }
            break;
        case 0xff01: // --res-lo
            fail = sscanf(optarg, "%d %n", &opt_mesh_lo.xres, &optn) < 1;
            fail = fail || !(opt_mesh_lo.xres>0);
            // if (!fail) {
            //     printf("RES_LO -> %d\n", opt_mesh_lo.xres);
            // }
            break;
        case 0:
            if (long_options[option_index].flag != 0) {
                // printf("%s -> %d\n", long_options[option_index].name, *long_options[option_index].flag);
                break;
            }
        case '?':
        default:
            exit(1);
        }

        if (optarg) {
            fail = fail || optarg[optn]!='\0';
        }

        if (fail) {
            fprintf(stderr, "%s: ", argv[0]);
            if (option_index) {
                fprintf(stderr, "option '--%s': ", long_options[option_index].name);
            } else {
                fprintf(stderr, "option '-%c': ", c);
            }
            fprintf(stderr, "bad argument '%s'\n", optarg);
            exit(1);
        }
    }


    if (argc-optind != 2) {
        if (argc-optind < 1) fprintf(stderr, "%s: missing INPUT file\n", argv[0]);
        if (argc-optind < 2) fprintf(stderr, "%s: missing TARGET path\n", argv[0]);
        if (argc-optind > 2) fprintf(stderr, "%s: too many command line arguments\n", argv[0]);
        fprintf(stderr, "Usage: vvplot [OPTIONS] INPUT TARGET. See `vvplot --help'.\n");
        exit(1);
    }

    /* XMIN, XMAX may be NaN, but NaNs are never swapped */
    if (xmin > xmax) { std::swap(xmin, xmax); }
    if (ymin > ymax) { std::swap(ymin, ymax); }
    
    if (1) {
        /* Trying to adjust plot dimensions
        Exactly one parameter should be missing */
        uint8_t var_cnt = 0;
        std::string var_names = "";
        #define COMMA std::string(var_cnt?", ":"")
        if (!opt_width)  { var_names+= COMMA+ "--size (W)"; var_cnt++; }
        if (!opt_height) { var_names+= COMMA+ "--size (H)"; var_cnt++; }
        if (xmin==xmax)  { var_names+= COMMA+ "-x (XMIN, XMAX)"; var_cnt++; }
        if (ymin==ymax)  { var_names+= COMMA+ "-y (YMIN, YMAX)"; var_cnt++; }
        if (isnan(xmin)) { var_names+= COMMA+ "-x (XMIN)"; var_cnt++; }
        if (isnan(xmax)) { var_names+= COMMA+ "-x (XMAX)"; var_cnt++; }
        if (isnan(ymin)) { var_names+= COMMA+ "-y (YMIN)"; var_cnt++; }
        if (isnan(ymax)) { var_names+= COMMA+ "-y (YMAX)"; var_cnt++; }
        #undef COMMA

        if (var_cnt < 1) {
            opt_height = 0;
        } else if (var_cnt > 1) {
            fprintf(stderr, "%s: too many options ommited: %s.\n", argv[0], var_names.c_str());
            exit(1);
        }

        /* By now we have exactly one unknown dimension */
        if (!opt_width)  { opt_width  = double(opt_height)*(xmax-xmin)/(ymax-ymin); opt_width-=opt_width%2; }
        if (!opt_height) { opt_height = double(opt_width) *(ymax-ymin)/(xmax-xmin); opt_height-=opt_height%2; }
        if (xmin==xmax)  { double xc=xmin, dx=(ymax-ymin)*opt_width/opt_height/2.0; xmax=xc+dx; xmin=xc-dx; }
        if (ymin==ymax)  { double yc=ymin, dy=(xmax-xmin)*opt_height/opt_width/2.0; ymax=yc+dy; ymin=yc-dy; }
        if (isnan(xmin)) { xmin = xmax - (ymax-ymin)*opt_width/opt_height; }
        if (isnan(xmax)) { xmax = xmin + (ymax-ymin)*opt_width/opt_height; }
        if (isnan(ymin)) { ymin = ymax - (xmax-xmin)*opt_height/opt_width; }
        if (isnan(ymax)) { ymax = ymin + (xmax-xmin)*opt_height/opt_width; }

        opt_mesh_lo.dxdy = (xmax-xmin)/opt_mesh_lo.xres;
        opt_mesh_lo.yres = ceil((ymax-ymin)/opt_mesh_lo.dxdy);

        opt_mesh_hi.dxdy = (xmax-xmin)/opt_mesh_hi.xres;
        opt_mesh_hi.yres = ceil((ymax-ymin)/opt_mesh_hi.dxdy);

        opt_Vcirc = (xmax-xmin)/opt_width*opt_Vsize/2; // radius
        // fprintf(stderr, "VCIRC -> %lf\n", opt_Vcirc);
    }

    std::string input_filename = argv[optind+0];
    std::string output_filename = argv[optind+1];
    Space S;
    S.Load(input_filename.c_str());
    return 0;
    // Exear *e = new Exear(output_filename);

    // {
    //     std::stringstream main_sh;
    //     /* By default gnuplot writes the plot to the stdout
    //     this tiny bash script protects the user from damaging tty
    //     and executes gnuplot only if user redirected stdout to file or pipe */
    //     // main_sh << "#!/bin/bash -x" << std::endl;
    //     main_sh << "test -t 1";
    //     main_sh << " && echo \"You are trying to write the plot to stdout. It may be harmful.\"";
    //     main_sh << " && echo \"Use: $0 > plot.png\"";
    //     main_sh << " && echo \"Or   $0 | display\"";
    //     main_sh << " && exit 1" << std::endl;
    //     main_sh << "exec ./main.gp" << std::endl;
    //     e->append("main.sh", main_sh.str(), 0755);
    // }

    // /* This is the gnuplot script itself */
    // std::stringstream main_gp;
    // main_gp << "#!/usr/bin/env gnuplot" << std::endl;
    // main_gp << "set terminal pngcairo enhanced"
    // << strfmt( "  size %d, %d", width, height);
    // main_gp << "  font \"FreeSerif, 26\"" << std::endl;
    // main_gp << "unset output" << std::endl;
    // main_gp << "unset key" << std::endl;
    // main_gp << "set border 0" << std::endl;
    // main_gp << "set lmargin 0" << std::endl;
    // main_gp << "set rmargin 0" << std::endl;
    // main_gp << "set bmargin 0" << std::endl;
    // main_gp << "set tmargin 0" << std::endl;
    // main_gp << "set zeroaxis" << std::endl;
    // main_gp << "unset xtics" << std::endl;
    // main_gp << "unset ytics" << std::endl
    // << strfmt( "set xrange [%lg:%lg]", xmin, xmax) << std::endl
    // << strfmt( "set yrange [%lg:%lg]", ymin, ymax) << std::endl;
    // main_gp << "unset colorbox" << std::endl;

    // main_gp << "set object 1 rectangle";
    // main_gp << " from screen 0,0 to screen 1,1";
    // main_gp << " fillcolor rgb \"#808080\"";
    // main_gp << " behind" << std::endl;
    // main_gp << "color(x) = (x>0) ? 0xffffff : 0x000000" << std::endl;


    // if (FLAGS_timelabel) {
    //     main_gp << strfmt("set label \"t = %lf\" at graph 0.05, 0.90 front", S.Time) << std::endl;
    // }

    // std::stringstream plot_cmd;
    // #define DELIMITER (!plot_cmd.tellp()?"plot \\\n    ":", \\\n    ")

    // if (FLAGS_g > 0.0) {
    //     main_gp << "set palette defined (-1 \"#000000\", 1 \"#ffffff\")" << std::endl
    //     << strfmt( "set cbrange [%lg:%lg]\n", -FLAGS_g, FLAGS_g);

    //     plot_cmd << DELIMITER;
    //     plot_cmd << "'map_vorticity' binary matrix";
    //     plot_cmd << " u 1:2:3";
    //     plot_cmd << " with image";

    //     std::stringstream bin;
    //     map_vorticity(&S, bin, rect, mesh_hi);
    //     e->append("map_vorticity", bin.str());
    // }

    // if (FLAGS_v) {
    //     plot_cmd << DELIMITER;
    //     plot_cmd << "'vortex_list'";
    //     plot_cmd << " binary format='%3float'";
    //     plot_cmd << " u 1:2:(color($3))";
    //     plot_cmd << " with dots lc rgb variable";
    // }

    // if (FLAGS_V) {
    //     plot_cmd << DELIMITER;
    //     plot_cmd << "'vortex_list'";
    //     plot_cmd << " binary format='%3float'"
    //     << strfmt(  " u 1:2:(%lf):(color($3))", circle_size);
    //     plot_cmd << " with circles lc rgb variable fs transparent solid 0.3 noborder";
    // }

    // if (FLAGS_V || FLAGS_v) {
    //     std::stringstream bin;
    //     for (auto& lobj: S.VortexList) {
    //         float f[3] = {float(lobj.r.x), float(lobj.r.y), float(lobj.g)};
    //         bin.write(reinterpret_cast<const char*>(f), sizeof(f));
    //     }

    //     e->append("vortex_list", bin.str());
    // }

    // if (FLAGS_b) {
    //     for (auto& lbody: S.BodyList) {
    //         plot_cmd << DELIMITER;
    //         plot_cmd << "'" << lbody->label << "'";
    //         plot_cmd << " binary format='%2float'";
    //         plot_cmd << " with filledcurve lc rgb '#adadad' lw 2 fs solid border -1";

    //         std::stringstream bin;
    //         for (auto& latt: lbody->alist) {
    //             TVec p = latt.corner;
    //             float f[2] = {float(p.x), float(p.y)};
    //             bin.write(reinterpret_cast<const char*>(f), sizeof(f));
    //         }
    //         { /* close the curve */
    //             TVec p = lbody->alist.front().corner;
    //             float f[2] = {float(p.x), float(p.y)};
    //             bin.write(reinterpret_cast<const char*>(f), sizeof(f));
    //         }

    //         e->append(lbody->label.c_str(), bin.str());
    //     }
    // }
    // #undef DELIMITER

    // main_gp << plot_cmd.str() << std::endl;    
    // e->append("main.gp", main_gp.str(), 0755);

    // {
    //     std::stringstream info;
    //     for (int i=0; i<argc; i++) {
    //         info << argv[i] << std::endl;
    //     }
    //     e->append("#argv", info.str());
    // }

    // delete e;
    // if (FLAGS_exec) {
    //     int ret = execl(output_filename.c_str(), "", (char*)NULL);
    //     if (ret) {
    //         fprintf(stderr, "execlp failed: %s (%d)\n", strerror(errno), errno);
    //     }
    // }

    // return 0;
}
