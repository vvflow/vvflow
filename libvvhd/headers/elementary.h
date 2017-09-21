#ifndef ELEMENTARY_H_
#define ELEMENTARY_H_

#include <math.h>
#include <iostream>
// #include "shellscript.h"
#include "stdint.h"

// #include "TVec.hpp"
// #include "TObj.hpp"

typedef void* pointer;
typedef char* pchar;
const double C_PI =		3.14159265358979323846;
const double C_2PI =	2.*C_PI;
const double C_1_2PI =  1./(2.*C_PI);
const double C_1_4PI =	1./(4.*C_PI);
const double C_1_PI = 	1./C_PI;
const double C_2_PI = 	2./C_PI;

extern const char* libvvhd_gitrev;
extern const char* libvvhd_gitinfo;
extern const char* libvvhd_gitdiff;

/* COMMON STUFF **************************************************************/

inline double sign(double x) { return (x>0) ? 1 : ((x<0) ? -1 : 0); }
inline double sqr(double x) { return x*x; }
inline int sqr(int x) { return x*x; }
inline double max(double a, double b) { return(a>b)?a:b; }
inline double min(double a, double b) { return(a<b)?a:b; }
inline double max(double a, double b, double c) { return max(a, max(b, c)); }
inline double min(double a, double b, double c) { return min(a, min(b, c)); }
inline double max(double a, double b, double c, double d) { return max(a, max(b, max(c,d))); }
inline double min(double a, double b, double c, double d) { return min(a, min(b, min(c,d))); }


/* PARSING STRINGS ***********************************************************/
// returns 1 on success, 0 on failure
template<typename T> static bool parse(const char *text, T* result);

// template<> bool parse(const char *text, double* result)
// {
//     // can't save result
//     if (!result)
//         return false;

//     // empty string defaults to 0.0
//     if (text[0] == '\0')
//         return (*result = 0.0, true);

//     int len, ret;
//     ret = sscanf(text, "%lg%n", result, &len);
//     return ret==1 && text[len]=='\0';
// }

// template<> bool parse(const char *text, int* result)
// {
//     // can't save result
//     if (!result)
//         return false;

//     // empty string defaults to 0
//     if (text[0] == '\0')
//         return (*result = 0, true);

//     int len, ret;
//     ret = sscanf(text, "%d%n", result, &len);
//     return ret==1 && text[len]=='\0';
// }

// template<> bool parse(const char* text, TTime* result)
// {
//     // can't save result
//     if (!result)
//         return false;

//     // empty string defaults to 0
//     if (text[0] == '\0')
//     {
//         result->value = 0;
//         result->timescale = 1;
//         return true;
//     }

//     int len, ret;
//     ret = sscanf(text, "%d/%u%n", &result->value, &result->timescale, &len);
//     if (ret==2 && text[len]=='\0') {
//         return true;
//     } else {
//         double dbl;
//         ret = parse<double>(text, &dbl);
//         if (ret && isfinite(dbl)) {
//             *result = TTime::makeWithSecondsDecimal(dbl);
//             return true;
//         }
//         return false;
//     }
// }

// template<> bool parse(const char* text, TVec* result)
// {
//     // can't save result
//     if (!result)
//         return false;

//     // empty string defaults to 0
//     if (text[0] == '\0')
//     {
//         result->x = 0.;
//         result->y = 0.;
//         return true;
//     }

//     int len, ret;
//     ret = sscanf(text, "%lg, %lg%n", &result->x, &result->y, &len);
//     return (ret==2 && text[len]=='\0');
// }

// template<> bool parse(const char* text, TVec3D* result)
// {
//     // can't save result
//     if (!result)
//         return false;

//     // empty string defaults to 0
//     if (text[0] == '\0')
//     {
//         result->r.x = 0.;
//         result->r.y = 0.;
//         result->o = 0.;
//         return true;
//     }

//     int len, ret;
//     ret = sscanf(text, "%lg, %lg, %lg%n", &result->r.x, &result->r.y, &result->o, &len);
//     return (ret==3 && text[len]=='\0');
// }

#endif

