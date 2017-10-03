#include "./exear.hpp"
#include "./optparse.hpp"

/* vvhd */
#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "XVorticity.hpp"
#include "XStreamfunction.hpp"
#include "XIsoline.hpp"

// #include <string.h> /* strerror */
// #include <inttypes.h> /* PRIu32 */
// #include <sys/stat.h> /* chmod */
#include <unistd.h> /* exec */
#include <cstring> /* strerror */
#include <cmath> /* nan */
// #include <errno.h>
// #include <fcntl.h> /* O_RDONLY */

/* std::... */
#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <sstream>
#include <memory> //unique_ptr

static const double d_nan = nan("");
static const double d_inf = 1.0l/0.0l;

// main options
namespace opt {
    bool   B = false;

    bool   V = false;
    int    Vsize = 4; // =0: plot dots; >0: plot circles
    double Vcirc = 0.; // circle size in Oxy scale

    bool   S = false;
    double Smin = d_nan;
    double Smax = d_nan;
    double Sstep = d_nan;

    bool   G = false;
    double Gmax = 0;

    bool   P = false;
    double Pmin = -1.5;
    double Pmax = +1.0;

    // other options
    char   ref_xy = 'o';
    char   ref_S =  'o';
    char   ref_P =  's';

    int width = 1280;
    int height = 720;
    int gray = false;
    int colorbox = false;
    int timelabel = true;
    int holder = false;
    int spring = false;
    
    int ttree_bottom_nodes = false;
    int ttree_near_nodes = false;
    int ttree_find_node = false;

    int dry_run = false;

    rect_t rect = {0.};
    mesh_t mesh_hi = {0., 640, 0};
    mesh_t mesh_lo = {0., 256, 0};

    const char* input = NULL;
    const char* target = NULL;
}

template<typename ... Args>
std::string strfmt(const char* format, Args ... args)
{
    size_t size = snprintf( nullptr, 0, format, args ... ) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format, args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}


int main(int argc, char **argv)
{
    opt::parse(argc, argv);

    Space S;
    S.Load(opt::input);
    Exear *e = new Exear(opt::target);

    {
        std::stringstream main_sh;
        /* By default gnuplot writes the plot to the stdout
        this tiny bash script protects the user from damaging tty
        and executes gnuplot only if user redirected stdout to file or pipe */
        // main_sh << "#!/bin/bash -x" << std::endl;
        main_sh << "test -t 1";
        main_sh << " && echo \"You are trying to write the plot to stdout. It may be harmful.\"";
        main_sh << " && echo \"Use: $0 > plot.png\"";
        main_sh << " && echo \"Or   $0 | display\"";
        main_sh << " && exit 1" << std::endl;
        main_sh << "exec ./main.gp" << std::endl;
        e->append("main.sh", main_sh.str(), 0755);
    }

    /* This is the gnuplot script itself */
    std::stringstream main_gp;
    main_gp << "#!/usr/bin/env gnuplot" << std::endl;
    main_gp << "set terminal pngcairo enhanced"
    << strfmt( "  size %d, %d", opt::width, opt::height);
    main_gp << "  font \"FreeSerif, 26\"" << std::endl;
    main_gp << "unset output" << std::endl;
    main_gp << "unset key" << std::endl;
    main_gp << "set border 0" << std::endl;
    main_gp << "set lmargin 0" << std::endl;
    main_gp << "set rmargin 0" << std::endl;
    main_gp << "set bmargin 0" << std::endl;
    main_gp << "set tmargin 0" << std::endl;
    main_gp << "set zeroaxis" << std::endl;
    main_gp << "unset xtics" << std::endl;
    main_gp << "unset ytics" << std::endl
    << strfmt( "set xrange [%lg:%lg]", opt::rect.xmin, opt::rect.xmax) << std::endl
    << strfmt( "set yrange [%lg:%lg]", opt::rect.ymin, opt::rect.ymax) << std::endl;
    if (opt::colorbox) {
        main_gp << "set colorbox horizontal user origin .1,.02 size .8,.04" << std::endl;
    } else {
        main_gp << "unset colorbox" << std::endl;
    }

    if (opt::gray) {
        main_gp << "set object 1 rectangle";
        main_gp << " from screen 0,0 to screen 1,1";
        main_gp << " fillcolor rgb \"#808080\"";
        main_gp << " behind" << std::endl;
        main_gp << "color(x) = (x>0) ? 0xffffff : 0x000000" << std::endl;
    } else {
        main_gp << "color(x) = (x>0) ? 0xff0000 : 0x0000ff" << std::endl;
    }

    if (opt::timelabel) {
        main_gp << strfmt("set label \"t = %lf\" at graph 0.05, 0.90 front", S.Time) << std::endl;
    }

    std::stringstream plot_cmd;
    #define DELIMITER (!plot_cmd.tellp()?"plot \\\n    ":", \\\n    ")

    if (opt::G) {
        std::stringstream bin;
        XVorticity vrt = {S,
            opt::rect.xmin, opt::rect.ymin,
            opt::mesh_hi.dxdy,
            opt::mesh_hi.xres+1,
            opt::mesh_hi.yres+1,
            2};
        vrt.evaluate();
        bin << vrt;
        e->append("map_vorticity", bin.str());

        if (opt::gray) {
            main_gp << "set palette defined (-1 \"#000000\", 1 \"#ffffff\")" << std::endl;
        } else {
            main_gp << "set palette defined (-1 \"#0000ff\", 0 \"#ffffff\", 1 \"#ff0000\")" << std::endl;
        }
        if (opt::Gmax > 0) {
            main_gp << strfmt( "set cbrange [%lg:%lg]\n", -opt::Gmax, opt::Gmax);
        } else {
            main_gp << "set cbrange []\n" << std::endl;
        }

        plot_cmd << DELIMITER;
        plot_cmd << "'map_vorticity'";
        plot_cmd << " binary matrix";
        plot_cmd << " u 1:2:3";
        plot_cmd << " with image";
    }

    if (opt::V) {
        std::stringstream bin;
        for (auto& lobj: S.VortexList) {
            float f[3] = {float(lobj.r.x), float(lobj.r.y), float(lobj.g)};
            bin.write(reinterpret_cast<const char*>(f), sizeof(f));
        }

        e->append("vortex_list", bin.str());

        plot_cmd << DELIMITER;
        plot_cmd << "'vortex_list'";
        plot_cmd << " binary format='%3float'";
        if (opt::Vsize > 0) {
            plot_cmd << strfmt(  " u 1:2:(%lf):(color($3))", opt::Vcirc)
            << " with circles lc rgb variable fs transparent solid 0.3 noborder";
        } else {
            plot_cmd << " u 1:2:(color($3))"
            << " with dots lc rgb variable";
        }
    }

    if (opt::S) {
        std::stringstream bin_streamfunction;
        XStreamfunction psi = {S,
            opt::rect.xmin, opt::rect.ymin,
            opt::mesh_lo.dxdy,
            opt::mesh_lo.xres+1,
            opt::mesh_lo.yres+1,
            2, opt::ref_S};
        psi.evaluate();
        bin_streamfunction << psi;
        e->append("map_streamfunction", bin_streamfunction.str());

        // main_gp << "set palette defined (-1 \"#000000\", 1 \"#ffffff\")" << std::endl;
        // main_gp << "set cbrange []\n" << std::endl;

        // plot_cmd << DELIMITER;
        // plot_cmd << "'map_streamfunction'";
        // plot_cmd << " binary matrix";
        // plot_cmd << " with image";

        if (!(opt::Sstep>0)) {
            opt::Smin = psi.min();
            opt::Smax = psi.max();
            opt::Sstep = (opt::Smax - opt::Smin)/33.;
        }

        std::stringstream bin_streamline;
        bin_streamline << XIsoline(psi, opt::Smin, opt::Smax, opt::Sstep);
        e->append("streamlines", bin_streamline.str());

        plot_cmd << DELIMITER;
        plot_cmd << "'streamlines'";
        plot_cmd << " binary format='%2float'";
        plot_cmd << " with lines lw 1 lc rgb 'black'";
    }

    for (const std::shared_ptr<TBody>& lbody: S.BodyList) {
        if (opt::B) {
            plot_cmd << DELIMITER;
            plot_cmd << "'" << lbody->label << "'";
            plot_cmd << " binary format='%2float'";
            plot_cmd << " with filledcurve lc rgb '#adadad' lw 2 fs solid border -1";
        }

        std::stringstream bin;
        for (const TAtt& latt: lbody->alist) {
            TVec p = latt.corner;
            float f[2] = {float(p.x), float(p.y)};
            bin.write(reinterpret_cast<const char*>(f), sizeof(f));
        }
        { /* close the curve */
            TVec p = lbody->alist.front().corner;
            float f[2] = {float(p.x), float(p.y)};
            bin.write(reinterpret_cast<const char*>(f), sizeof(f));
        }

        e->append(lbody->label.c_str(), bin.str());
    }

    if (opt::holder || opt::spring) {
        std::stringstream body_holders;
        for (const std::shared_ptr<TBody>& lbody: S.BodyList) {
            TVec p = lbody->holder.r;
            body_holders << p.x << " " << p.y;
            body_holders << " ";
            p = lbody->get_axis();
            body_holders << p.x << " " << p.y;
            body_holders << std::endl;
        }
        e->append("body_holders", body_holders.str());

        if (opt::holder) {
            plot_cmd << DELIMITER;
            plot_cmd << "'body_holders'";
            plot_cmd << strfmt(  " u 1:2:(1.5*%lf)", opt::Vcirc);
            plot_cmd << " w circles lc rgb 'black' fs solid noborder";
        }
        if (opt::spring) {
            plot_cmd << DELIMITER;
            plot_cmd << "'body_holders'";
            plot_cmd << strfmt(  " u 3:4:(1.5*%lf)", opt::Vcirc);
            plot_cmd << " w circles lc rgb 'black' fs solid noborder";
            plot_cmd << DELIMITER;
            plot_cmd << "'body_holders'";
            plot_cmd << " u 1:2:($3-$1):($4-$2)";
            plot_cmd << " w vectors nohead lc rgb 'black' lw 1.5";
        }
    }

    if (opt::ttree_bottom_nodes
        || opt::ttree_near_nodes
        || opt::ttree_find_node
    ) {
        double dl = S.AverageSegmentLength();
        double min_node_size = dl>0 ? dl*10 : 0;
        double max_node_size = dl>0 ? dl*20 : d_inf;
        TSortedTree tree = {&S, 8, min_node_size, max_node_size};
        tree.build();

        if (opt::ttree_bottom_nodes) {
            std::stringstream bin_bottom_nodes;
            for (TSortedNode* lbn: tree.getBottomNodes())
            {
                float f[4] = {0.};
                f[0] = lbn->x;
                f[1] = lbn->y;
                f[2] = lbn->w/2;
                f[3] = lbn->h/2;
                bin_bottom_nodes.write(reinterpret_cast<const char*>(f), sizeof(f));
            }
            e->append("ttree_bottom_nodes", bin_bottom_nodes.str());

            plot_cmd << DELIMITER;
            plot_cmd << "'ttree_bottom_nodes'";
            plot_cmd << " binary format='%4float'";
            plot_cmd << " with boxxy fill empty lc rgb 'black'";
        }

        if (opt::ttree_near_nodes) {
            std::stringstream bin_near_nodes;
            for (TSortedNode* lbn: tree.getBottomNodes())
            {
                float f[4] = {0.};
                for (TSortedNode* lnn: *lbn->NearNodes)
                {
                    f[0] = lnn->x;
                    f[1] = lnn->y;
                    f[2] = lbn->x - lnn->x;
                    f[3] = lbn->y - lnn->y;
                    bin_near_nodes.write(reinterpret_cast<const char*>(f), sizeof(f));
                }
            }
            e->append("ttree_near_nodes", bin_near_nodes.str());

            plot_cmd << DELIMITER;
            plot_cmd << "'ttree_near_nodes'";
            plot_cmd << " binary format='%4float'";
            plot_cmd << " with vectors nohead lc rgb 'black'";
        }        

        if (opt::ttree_find_node) {
            std::stringstream bin_find_node;
            for (int yj=0; yj<opt::mesh_lo.yres; yj+=4) {
                for (int xi=0; xi<opt::mesh_lo.xres; xi+=4) {
                    TVec p = TVec(opt::rect.xmin, opt::rect.ymin) + opt::mesh_lo.dxdy*TVec(xi, yj);
                    const TSortedNode& bnode = *tree.findNode(p);
                    float f[4] = {0.};
                    f[0] = p.x;
                    f[1] = p.y;
                    f[2] = bnode.x - p.x;
                    f[3] = bnode.y - p.y;
                    bin_find_node.write(reinterpret_cast<const char*>(f), sizeof(f));
                }
            }
            e->append("ttree_find_node", bin_find_node.str());

            plot_cmd << DELIMITER;
            plot_cmd << "'ttree_find_node'";
            plot_cmd << " binary format='%4float'";
            plot_cmd << " with vectors nohead lc rgb 'black'";
        }
    }
    #undef DELIMITER

    main_gp << plot_cmd.str() << std::endl;
    e->append("main.gp", main_gp.str(), 0755);

    {
        std::stringstream info;
        for (int i=0; i<argc; i++) {
            info << argv[i] << std::endl;
        }
        e->append("#argv", info.str());
    }

    delete e;
    if (!opt::dry_run) {
        int ret = execl(opt::target, "", (char*)NULL);
        if (ret) {
            fprintf(stderr, "execlp failed: %s (%d)\n", strerror(errno), errno);
        }
    }

    return 0;
}
