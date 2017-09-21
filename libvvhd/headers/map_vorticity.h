#pragma once
#include <core.h>
#include <sstream>

// void map_vorticity(Space* S, std::stringstream &bin, rect_t &rect, mesh_t &mesh);

class MapVorticity : private Space {
public:
    MapVorticity();
    MapVorticity(const Space &S);

    void calc(double xmin, double ymin, double dxdy, int xres, int yres);

    //  N   y1  y2  yN
    // x1  z11 z12 z1N
    // x2  z21 z22 z2N
    // xM  zM1 zM2 zMN
    friend std::ostream& operator<< (std::ostream& os, const MapVorticity& p) { return os << p.x << " \t" << p.y; }

private:
    double xmin, ymin;
    int    xres, yres;
    double dxdy;
    vector<float> map;
}
