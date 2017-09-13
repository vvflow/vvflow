// #include <string.h> /* strerror */
// #include <inttypes.h> /* PRIu32 */
// #include <sys/stat.h> /* chmod */
#include <unistd.h> /* exec */
#include <math.h> /* nan */
// #include <errno.h>
// #include <fcntl.h> /* O_RDONLY */

/* std::... */
#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <sstream>
#include <memory> //unique_ptr

/* vvhd */
#include <core.h>

#include "./optparse.h"
static const double d_nan = nan("");
static const double d_inf = 1.0l/0.0l;

// main options
namespace opt {
    bool   B = false;

    bool   V = false;
    int    Vsize = 6; // =0: plot dots; >0: plot circles
    double Vcirc = 0.;

    bool   S = false;
    double Smin = d_nan;
    double Smax = d_nan;
    double Sstep = d_nan;

    bool   G = false;
    double Grange = d_nan;

    bool   P = false;
    double Pmin = -1.5;
    double Pmax = +1.0;
    // other options
    // char   ref_xy;
    // char   ref_S;
    // char   ref_P;

    int width = 1280;
    int height = 720;
    int gray;
    int colorbox;
    int timelabel;
    int holder;
    int spring;

    rect_t rect = {0.};
    mesh_t mesh_hi = {0., 640, 0};
    mesh_t mesh_lo = {0., 256, 0};

    const char* input = NULL;
    const char* target = NULL;
}

// template<typename ... Args>
// std::string strfmt(const char* format, Args ... args)
// {
//     size_t size = snprintf( nullptr, 0, format, args ... ) + 1;
//     std::unique_ptr<char[]> buf(new char[size]);
//     snprintf(buf.get(), size, format, args ...);
//     return std::string(buf.get(), buf.get() + size - 1);
// }


int main(int argc, char **argv)
{
    opt::parse(argc, argv);

    Space S;
    S.Load(opt::input);
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
