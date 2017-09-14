#ifndef _OPTPARSE_H_
#define _OPTPARSE_H_

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
    extern double Grange;

    extern bool   P;
    extern double Pmin;
    extern double Pmax;
    // other options
    // extern char   ref_xy;
    // extern char   ref_S;
    // extern char   ref_P;

    extern int width;
    extern int height;
    extern int gray;
    extern int colorbox;
    extern int timelabel;
    extern int holder;
    extern int spring;

    extern int dry_run;

    extern rect_t rect;
    extern mesh_t mesh_hi;
    extern mesh_t mesh_lo;

    extern const char* input;
    extern const char* target;
}

#endif