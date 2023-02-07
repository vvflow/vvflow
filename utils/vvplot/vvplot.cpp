#include "./optparse.hpp"
#include "./gnuplotter.hpp"

/* vvhd */
#include "TSpace.hpp"
#include "TSortedTree.hpp"
#include "XPressure.hpp"
#include "XVorticity.hpp"
#include "XStreamfunction.hpp"
#include "XVelocity.hpp"
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
#include <fstream>
#include <sstream>

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

    char   U = '\0';
    double Umin = d_nan;
    double Umax = d_nan;

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

    rect_t rect = {0.};
    mesh_t mesh_hi = {0., 640, 0};
    mesh_t mesh_lo = {0., 256, 0};
    double eps_mult = 2.;

    const char* input;
    const char* target;
    const char* load_field;
    int ifmt = ffmt::unknown;
    int ofmt = ffmt::png;
}

int main_hdf(int argc, char **argv)
{
    Space S;
    S.load(opt::input);

    Gnuplotter gp;

    /* This is the gnuplot script itself */
    gp << "#!/usr/bin/env gnuplot" << std::endl;
    gp << "# Generated by vvplot:" << std::endl << "#";
    for (int i=0; i<argc; i++) {
        gp << " " << argv[i];
    }
    gp << "\n" << std::endl;

    if (opt::ref_xy == 'o') {
        // do nothing
    } else if (opt::ref_xy == 'f') {
        opt::rect.xmin += S.inf_marker.x;
        opt::rect.xmax += S.inf_marker.x;
        opt::rect.ymin += S.inf_marker.y;
        opt::rect.ymax += S.inf_marker.y;
    } else if (opt::ref_xy == 'b') {
        TVec pos = S.BodyList.front()->get_axis();
        opt::rect.xmin += pos.x;
        opt::rect.xmax += pos.x;
        opt::rect.ymin += pos.y;
        opt::rect.ymax += pos.y;
    } else if (opt::ref_xy == 'x') {
        TVec pos = S.BodyList.front()->get_axis();
        opt::rect.xmin += pos.x;
        opt::rect.xmax += pos.x;
    } else if (opt::ref_xy == 'y') {
        TVec pos = S.BodyList.front()->get_axis();
        opt::rect.ymin += pos.y;
        opt::rect.ymax += pos.y;
    }

    gp << "set terminal pngcairo enhanced"
    << strfmt( "  size %d, %d", opt::width, opt::height);
    gp << "  font \"FreeSerif, 26\"" << std::endl;
    gp << "unset output" << std::endl;
    gp << "unset key" << std::endl;
    gp << "set border 0" << std::endl;
    gp << "set lmargin 0" << std::endl;
    gp << "set rmargin 0" << std::endl;
    gp << "set bmargin 0" << std::endl;
    gp << "set tmargin 0" << std::endl;
    gp << "set zeroaxis" << std::endl;
    gp << "unset xtics" << std::endl;
    gp << "unset ytics" << std::endl
    << strfmt( "set xrange [%lg:%lg]", opt::rect.xmin, opt::rect.xmax) << std::endl
    << strfmt( "set yrange [%lg:%lg]", opt::rect.ymin, opt::rect.ymax) << std::endl;
    if (opt::colorbox) {
        gp << "set format cb \"\"" << std::endl;
        gp << "set colorbox horizontal user";
        gp << " origin screen 0.15, character 0.4";
        gp << " size   screen 0.70, character 1.0" << std::endl;
        gp << "set label 61 \"\"";
        gp << " at screen 0.14, character 0.9 right front" << std::endl;
        gp << "set label 62 \"\"";
        gp << " at screen 0.86, character 0.9 left front" << std::endl;
    } else {
        gp << "unset colorbox" << std::endl;
        gp << "set label 61 \"\" at screen 0, -1" << std::endl;
        gp << "set label 62 \"\" at screen 1, -1" << std::endl;
    }

    if (opt::gray) {
        gp << "set object 1 rectangle";
        gp << " from screen 0,0 to screen 1,1";
        gp << " fillcolor rgb \"#808080\"";
        gp << " behind" << std::endl;
        gp << "color(x) = (x>0) ? 0xffffff : 0x000000" << std::endl;
    } else {
        gp << "color(x) = (x>0) ? 0xff0000 : 0x0000ff" << std::endl;
    }

    if (opt::timelabel) {
        gp << strfmt("set label \"t = %lf\" at graph 0.05, 0.90 front", double(S.time)) << std::endl;
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
        };
        vrt.eps_mult = opt::eps_mult;
        vrt.evaluate();
        bin << vrt;
        gp.add("map_vorticity", bin.str());

        if (opt::gray) {
            gp << "set palette defined (-1 \"#000000\", 1 \"#ffffff\")" << std::endl;
        } else {
            gp << "set palette defined (-1 \"#0000ff\", 0 \"#ffffff\", 1 \"#ff0000\")" << std::endl;
        }
        if (opt::Gmax == 0) {
            float pmin = vrt.percentile(0.02);
            float pmax = vrt.percentile(0.98);
            opt::Gmax = std::max(-pmin, pmax);
        }
        gp << strfmt( "set cbrange [%lg:%lg]\n", -opt::Gmax, opt::Gmax);
        gp << strfmt( "set label 61 \"%.1lf\"\n", -opt::Gmax);
        gp << strfmt( "set label 62 \"%.1lf\"\n", opt::Gmax);

        plot_cmd << DELIMITER;
        plot_cmd << "'map_vorticity'";
        plot_cmd << " binary matrix";
        plot_cmd << " u 1:2:3";
        plot_cmd << " with image";
    }

    if (opt::P) {
        std::stringstream bin;
        XPressure prs = {S,
            opt::rect.xmin, opt::rect.ymin,
            opt::mesh_hi.dxdy,
            opt::mesh_hi.xres+1,
            opt::mesh_hi.yres+1,
        };
        prs.eps_mult = opt::eps_mult;
        prs.ref_frame = opt::ref_P;
        prs.evaluate();
        bin << prs;
        gp.add("map_pressure", bin.str());

        if (opt::gray) {
            gp << "set palette defined (" <<
                "-1.5 \"#000000\", " <<
                "0.0 \"#c8c8c8\", " <<
                "0.5 \"#f0f0f0\", " <<
                "1.0 \"#ffffff\"" <<
            ")" << std::endl;
        } else {
            gp << "set palette defined ("
                "-1.5 \"#0829d5\", " <<
                "-0.5 \"#1ffcff\", " <<
                "0.0 \"#2ef62e\", " <<
                "0.3 \"yellow\", " <<
                "1.0 \"red\"" <<
            ")" << std::endl;
        }

        gp << strfmt( "set cbrange [%lg:%lg]\n", opt::Pmin, opt::Pmax);
        gp << strfmt( "set label 61 \"%.1lf\"\n", opt::Pmin);
        gp << strfmt( "set label 62 \"%.1lf\"\n", opt::Pmax);

        plot_cmd << DELIMITER;
        plot_cmd << "'map_pressure'";
        plot_cmd << " binary matrix";
        plot_cmd << " u 1:2:3";
        plot_cmd << " with image";

        std::stringstream bin_isopressure;
        bin_isopressure << XIsoline(prs, opt::Pmin, opt::Pmax, (opt::Pmax - opt::Pmin)/33.);
        gp.add("isopressure", bin_isopressure.str());

        plot_cmd << DELIMITER;
        plot_cmd << "'isopressure'";
        plot_cmd << " binary format='%2float'";
        plot_cmd << " with lines lw 1 lc rgb '#9999cc'";
    }

    if (opt::U) {
        XField *field = nullptr;

        if (opt::load_field) {
            std::stringstream fbuf;
            std::ifstream file(opt::load_field, std::ios::binary);
            fbuf << file.rdbuf();
            field = new XField(fbuf.str());
        } else {
            XVelocity *ufield = new XVelocity(
                S,
                opt::rect.xmin,
                opt::rect.ymin,
                opt::mesh_hi.dxdy,
                opt::mesh_hi.xres+1,
                opt::mesh_hi.yres+1
            );

            ufield->mode = opt::U;
            ufield->ref_frame = opt::ref_S;
            ufield->evaluate();
            field = ufield;
        }

        std::string field_name = "map_velocity_" + std::string(1, opt::U);
        std::stringstream bin;
        bin << *field;
        gp.add(field_name, bin.str());

        if (!std::isfinite(opt::Umin)) opt::Umin = field->min();
        if (!std::isfinite(opt::Umax)) opt::Umax = field->max();

        gp << "set palette defined (" <<
            opt::Umin/1 << " \"#0829d5\", " <<
            opt::Umin/3 << " \"#1ffcff\", " <<
                    0.0 << " \"#2ef62e\", " <<
            opt::Umax/3 << " \"yellow\", " <<
            opt::Umax/1 << " \"red\"" <<
        ")" << std::endl;

        gp << strfmt( "set cbrange [%lg:%lg]\n", opt::Umin, opt::Umax);
        gp << strfmt( "set label 61 \"%.1lf\"\n", opt::Umin);
        gp << strfmt( "set label 62 \"%.1lf\"\n", opt::Umax);

        plot_cmd << DELIMITER;
        plot_cmd << "'" << field_name << "'";
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

        gp.add("vortex_list", bin.str());

        plot_cmd << DELIMITER;
        plot_cmd << "'vortex_list'";
        plot_cmd << " binary format='%3float'";
        plot_cmd << " u 1:2:(color($3))";
        plot_cmd << " with dots lc rgb variable";
        if (opt::Vsize > 0) {
            plot_cmd << DELIMITER;
            plot_cmd << "''";
            plot_cmd << " binary format='%3float'";
            plot_cmd << strfmt(  " u 1:2:(%lf):(color($3))", opt::Vcirc);
            plot_cmd << " with circles lc rgb variable fs transparent solid 0.3 noborder";
        }
    }

    if (opt::S) {
        XField *field = nullptr;

        if (opt::load_field) {
            std::stringstream fbuf;
            std::ifstream file(opt::load_field, std::ios::binary);
            fbuf << file.rdbuf();
            field = new XField(fbuf.str());
        } else {
            XStreamfunction *sfield = new XStreamfunction(
                S,
                opt::rect.xmin,
                opt::rect.ymin,
                opt::mesh_lo.dxdy,
                opt::mesh_lo.xres+1,
                opt::mesh_lo.yres+1
            );
            sfield->eps_mult = opt::eps_mult;
            sfield->ref_frame = opt::ref_S;
            sfield->evaluate();
            field = sfield;
        }

        std::stringstream bin_streamfunction;
        bin_streamfunction << *field;

        gp.add("map_streamfunction", bin_streamfunction.str());

        // gp << "set palette defined (-1 \"#000000\", 1 \"#ffffff\")" << std::endl;
        // gp << "set cbrange []\n" << std::endl;

        // plot_cmd << DELIMITER;
        // plot_cmd << "'map_streamfunction'";
        // plot_cmd << " binary matrix";
        // plot_cmd << " with image";

        if (!(opt::Sstep>0)) {
            opt::Smin = field->min();
            opt::Smax = field->max();
            opt::Sstep = (opt::Smax - opt::Smin)/33.;
        }

        std::stringstream bin_streamline;
        bin_streamline << XIsoline(*field, opt::Smin, opt::Smax, opt::Sstep);
        gp.add("streamlines", bin_streamline.str());

        plot_cmd << DELIMITER;
        plot_cmd << "'streamlines'";
        plot_cmd << " binary format='%2float'";
        plot_cmd << " with lines lw 1.5 lc rgb 'black'";
    }

    for (const std::shared_ptr<TBody>& lbody: S.BodyList) {
        const TBody& b = *lbody;
        std::string blabel = b.label;
        if (blabel.empty()){
            blabel = S.get_body_name(&b);
        }

        if (opt::B) {
            plot_cmd << DELIMITER;
            plot_cmd << "'" << blabel << "'";
            plot_cmd << " binary format='%2float'";
            plot_cmd << " with filledcurve lc rgb '#adadad' lw 2 fs solid border -1";
        }

        std::stringstream bin;
        for (const TAtt& latt: b.alist) {
            TVec p = latt.corner;
            float f[2] = {float(p.x), float(p.y)};
            bin.write(reinterpret_cast<const char*>(f), sizeof(f));
        }
        { /* close the curve */
            TVec p = b.alist.front().corner;
            float f[2] = {float(p.x), float(p.y)};
            bin.write(reinterpret_cast<const char*>(f), sizeof(f));
        }

        gp.add(blabel, bin.str());
    }

    if (opt::holder || opt::spring) {
        std::stringstream body_holders;
        std::stringstream body_springs;
        for (const std::shared_ptr<TBody>& lbody: S.BodyList) {
            TVec p_h = lbody->holder.r;
            body_holders << p_h.x << " " << p_h.y;
            body_holders << std::endl;

            TVec p_s = lbody->get_axis();
            body_springs << p_s.x << " " << p_s.y << " ";
            body_springs << p_h.x - p_s.x << " " << p_h.y - p_s.y;
            body_springs << std::endl;
        }
        gp.add("body_holders", body_holders.str());
        gp.add("body_springs", body_springs.str());

        if (opt::holder) {
            plot_cmd << DELIMITER;
            plot_cmd << "'body_holders'";
            plot_cmd << strfmt(  " u 1:2:(1.5*%lf)", opt::Vcirc);
            plot_cmd << " w circles lc rgb 'black' fs solid noborder";
        }
        if (opt::spring) {
            plot_cmd << DELIMITER;
            plot_cmd << "'body_springs'";
            plot_cmd << strfmt(  " u 1:2:(1.5*%lf)", opt::Vcirc);
            plot_cmd << " w circles lc rgb 'black' fs solid noborder";
            plot_cmd << DELIMITER;
            plot_cmd << "''";
            plot_cmd << " u 1:2:3:4";
            plot_cmd << " w vectors";
            plot_cmd << strfmt(" head size %lf,135", opt::Vcirc*3);
            plot_cmd << " lc rgb 'black' lw 1.5";
        }
    }

    if (opt::ttree_bottom_nodes
        || opt::ttree_near_nodes
        || opt::ttree_find_node
    ) {
        double dl = S.average_segment_length();
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
            gp.add("ttree_bottom_nodes", bin_bottom_nodes.str());

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
            gp.add("ttree_near_nodes", bin_near_nodes.str());

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
            gp.add("ttree_find_node", bin_find_node.str());

            plot_cmd << DELIMITER;
            plot_cmd << "'ttree_find_node'";
            plot_cmd << " binary format='%4float'";
            plot_cmd << " with vectors nohead lc rgb 'black'";
        }
    }
    #undef DELIMITER

    gp << plot_cmd.str() << std::endl;
    if (opt::ofmt == ffmt::tar) {
        gp.save(opt::target);
    } else if (opt::ofmt == ffmt::png) {
        gp.exec(opt::target);
    }

    // if (!opt::dry_run) {
    //     int ret = execl(opt::target, "", (char*)NULL);
    //     if (ret) {
    //         fprintf(stderr, "execlp failed: %s (%d)\n", strerror(errno), errno);
    //     }
    // }

    return 0;
}

int main_tar(int argc, char **argv)
{
    Gnuplotter gp;
    gp.load(opt::input);

    const std::string* map_streamfunction = gp.get("map_streamfunction");
    const std::string* streamlines = gp.get("streamlines");

    if (map_streamfunction && streamlines == nullptr) {
        std::cout << "Replotting streamlines" << std::endl;
        XField psi(*map_streamfunction);
        if (!(opt::Sstep>0)) {
            opt::Smin = psi.min();
            opt::Smax = psi.max();
            opt::Sstep = (opt::Smax - opt::Smin)/33.;
        }

        std::stringstream bin_streamline;
        bin_streamline << XIsoline(psi, opt::Smin, opt::Smax, opt::Sstep);
        gp.add("streamlines", bin_streamline.str());
    }

    gp.exec(opt::target);
    return 0;
}

int main(int argc, char **argv)
{
    opt::parse(argc, argv);

    if (opt::ifmt == ffmt::hdf) {
        return main_hdf(argc, argv);
    } else {
        return main_tar(argc, argv);
    }
}
