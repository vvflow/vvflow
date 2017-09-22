#pragma once

constexpr double C_PI =     3.14159265358979323846;
constexpr double C_2PI =    2.*C_PI;
constexpr double C_1_2PI =  1./(2.*C_PI);
constexpr double C_1_4PI =  1./(4.*C_PI);
constexpr double C_1_PI =   1./C_PI;
constexpr double C_2_PI =   2./C_PI;

extern const char* libvvhd_gitrev;
extern const char* libvvhd_gitinfo;
extern const char* libvvhd_gitdiff;

/* COMMON STUFF **************************************************************/

// inline double sign(double x) { return (x>0) ? 1 : ((x<0) ? -1 : 0); }
template<typename T> T sqr(T x) { return x*x; }
