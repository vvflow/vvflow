#pragma once
#include <vector>
#include <limits>
#include "TVec.hpp"

lua_Number get_param(
    lua_State *L,
    const char* param,
    const char* range_err = "",
    lua_Number min = -std::numeric_limits<double>::infinity(),
    lua_Number max = +std::numeric_limits<double>::infinity(),
    lua_Number dflt = std::numeric_limits<double>::quiet_NaN()
);

int luavvd_gen_cylinder(lua_State *L);
int luavvd_gen_plate(lua_State *L);
int luavvd_gen_gis(lua_State *L);

void gen_seg_N(std::vector<TAtt>& alist, TVec p1, TVec p2, size_t N, uint32_t slip = 0);
void gen_seg_dl(std::vector<TAtt>& alist, TVec p1, TVec p2, double dl, uint32_t slip = 0);
void gen_arc_N(std::vector<TAtt>& alist, TVec c, double R, double a1, double a2, size_t N, uint32_t slip = 0);
void gen_arc_dl(std::vector<TAtt>& alist, TVec c, double R, double a1, double a2, double dl, uint32_t slip = 0);
