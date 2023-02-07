#pragma once

typedef struct {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
} rect_t;

typedef struct {
    double dxdy;
    int    xres;
    int    yres;
} mesh_t;

enum ffmt {
    unknown = 0,
    png,
    tar,
    hdf
};

namespace opt {
    void parse(int argc, char **argv);

    // main options
    extern bool   B;

    extern bool   V;
    extern int    Vsize;
    extern double Vcirc;

    extern bool   S;
    extern double Smin;
    extern double Smax;
    extern double Sstep;

    extern bool   G;
    extern double Gmax;

    extern bool   P;
    extern double Pmin;
    extern double Pmax;

    extern char   U; // '\0' | 'x' | 'y'
    extern double Umin;
    extern double Umax;

    // other options
    extern char   ref_xy;
    extern char   ref_S;
    extern char   ref_P;

    extern int width;
    extern int height;
    extern int gray;
    extern int colorbox;
    extern int timelabel;
    extern int holder;
    extern int spring;
    extern int ttree_bottom_nodes;
    extern int ttree_near_nodes;
    extern int ttree_find_node;
    extern double eps_mult;

    extern rect_t rect;
    extern mesh_t mesh_hi;
    extern mesh_t mesh_lo;

    extern const char* input;
    extern const char* target;
    extern const char* load_field;
    extern int ifmt;
    extern int ofmt;
}
