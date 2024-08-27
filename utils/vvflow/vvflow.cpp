#include "./optparse.hpp"
#include "./lua_space.h"
#include "./lua_tvec.h"
#include "./lua_tobj.h"
#include "./lua_tvec3d.h"
#include "./lua_tbody.h"
#include "./lua_teval.h"
#include "./lua_objlist.h"
#include "./lua_bodylist.h"
#include "./gen_body.h"
#include "./getset.h"

#include "MEpsilonFast.hpp"
#include "MConvectiveFast.hpp"
#include "MDiffusiveFast.hpp"
#include "MStepdata.hpp"
#include "MFlowmove.hpp"
#include "elementary.h"

#include <iostream>
#include <fstream>
#include <cstdio>

#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <ctime>

#define H5_USE_18_API
#include <hdf5.h>

#include "sensors.cpp"
#include "omp.h"
#define dbg(a) a
//#define dbg(a) cerr << "Doing " << #a << "... " << flush; a; cerr << "done\n";
#define seek(f) while (fgetc(f)!='\n'){}

using std::cerr;
using std::endl;
using std::shared_ptr;

int stackDump(lua_State *L)
{
    int i;
    int top = lua_gettop(L); /* depth of the stack */
    for (i = 1; i <= top; i++) { /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING: { /* strings */
            printf("#%d str  '%s'\n", i, lua_tostring(L, i));
            break;
        }
        case LUA_TBOOLEAN: { /* Booleans */
            printf("#%d bool %s\n", i, lua_toboolean(L, i) ? "true" : "false");
            break;
        }
        case LUA_TNUMBER: { /* numbers */
            printf("#%d num  %g\n", i, lua_tonumber(L, i));
            break;
        }
        case LUA_TUSERDATA: {
            const char *typ;
            if (luaL_getmetafield(L, i, "__name")) {
                typ = lua_tostring(L, -1);
                lua_pop(L, 1);
            }
            else
                typ = "userdata";
            printf("#%d udata %s\n", i, typ);
            break;
        }
        default: { /* other values */
            printf("#%d %s\n", i, lua_typename(L, t));
            break;
        }
        }
    }
    printf("\n"); /* end the listing */
    return 0;
}

int simulate (lua_State *L);

int luaopen_vvd (lua_State *L) {

    lua_pushnumber(L, std::numeric_limits<double>::infinity()); // push 1
    lua_setglobal(L, "inf"); // pop 1
    lua_pushnumber(L, std::numeric_limits<double>::quiet_NaN()); // push 1
    lua_setglobal(L, "nan"); // pop 1

    lua_pushcfunction(L, luavvd_load_body); // push 1
    lua_setglobal(L, "load_body"); // pop 1
    lua_pushcfunction(L, luavvd_gen_cylinder); // push 1
    lua_setglobal(L, "gen_cylinder"); // pop 1
    lua_pushcfunction(L, luavvd_gen_semicyl); // push 1
    lua_setglobal(L, "gen_semicyl"); // pop 1
    lua_pushcfunction(L, luavvd_gen_ellipse); // push 1
    lua_setglobal(L, "gen_ellipse"); // pop 1
    lua_pushcfunction(L, luavvd_gen_plate); // push 1
    lua_setglobal(L, "gen_plate"); // pop 1
    lua_pushcfunction(L, luavvd_gen_parallelogram); // push 1
    lua_setglobal(L, "gen_parallelogram"); // pop 1
    lua_pushcfunction(L, luavvd_gen_chamber_gpj); // push 1
    lua_setglobal(L, "gen_chamber_gpj"); // pop 1
    lua_pushcfunction(L, luavvd_gen_chamber_box); // push 1
    lua_setglobal(L, "gen_chamber_box"); // pop 1
    lua_pushcfunction(L, luavvd_gen_savonius); // push 1
    lua_setglobal(L, "gen_savonius"); // pop 1

    lua_pushcfunction(L, simulate);
    lua_setglobal(L, "simulate");

    luaopen_space(L);
    luaopen_tvec(L);
    luaopen_tobj(L);
    luaopen_tvec3d(L);
    luaopen_teval(L);
    luaopen_tbody(L);
    luaopen_objlist(L);
    luaopen_bodylist(L);

    return 0;
}

// main options
namespace opt {
    int save_profile = 0;
    int show_progress = 0;

    const char* input;
    const char* sensors_file = NULL;

    int argc;
    char* const* argv;
}

int main(int argc, char* const* argv)
{
    opt::parse(argc, argv);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_vvd(L);
    int ret = 0;

    lua_newtable(L);
    for (int i=0; i<opt::argc; i++) {
        lua_pushstring(L, opt::argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "arg");

    if (H5Fis_hdf5(opt::input)) {
        lua_getglobal(L, "S");
        Space **ptr = (Space**)lua_touserdata(L, -1);
        lua_pop(L, 1);

        if (!ptr) {
            throw std::runtime_error("Internal error 0x180");
        }
        Space &S = **ptr;
        S.load(opt::input);
        ret = luaL_dostring(L, "simulate()");
    } else {
        ret = luaL_dofile(L, opt::input);
    }

    if (ret) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return 8;
    }

    return 0;
}

int simulate (lua_State *L) {
    lua_getglobal(L, "S");
    Space **ptr = (Space**)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (!ptr) {
        throw std::runtime_error("Internal error 0x180");
    }
    Space &S = **ptr;

    // error checking
    #define RAISE(STR) { cerr << "vvflow ERROR: " << STR << endl; return -1; }
    if (S.re <= 0) RAISE("invalid value re<=0");
    #undef RAISE
    bool is_viscous = (S.re != std::numeric_limits<double>::infinity());

    char f_stepdata[256]; snprintf(f_stepdata, 256, "stepdata_%s.h5", S.caption.c_str());
    char f_results[240]; snprintf(f_results, 240, "results_%s", S.caption.c_str());
    char f_sensors_output[256]; snprintf(f_sensors_output, 256, "velocity_%s", S.caption.c_str());
    mkdir(f_results, 0777);

    Stepdata stepdata = {&S, f_stepdata, !!opt::save_profile};

    double dl = S.average_segment_length();
    double min_node_size = dl>0 ? dl*5 : 0;
    double max_node_size = dl>0 ? dl*100 : std::numeric_limits<double>::max();
    TSortedTree tr = {&S, 8, min_node_size, max_node_size};
    MConvectiveFast convective = {&S, &tr};
    MEpsilonFast epsilon = {&S, &tr};
    MDiffusiveFast diffusive = {&S, &tr};
    MFlowmove flowmove = {&S};
    sensors sens(&S, &convective, opt::sensors_file, f_sensors_output);
    #ifdef OVERRIDEMOI
        S.BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
    #endif

    const void* collision = nullptr;
    while (S.time < S.finish + S.dt/2)
    {
        if (S.BodyList.size())
        {
            dbg(tr.build());

            // решаем слау
            if (collision != nullptr) {
                convective.calc_circulation(&collision);
            }
            convective.calc_circulation(&collision);

            dbg(tr.destroy());
        }

        dbg(flowmove.heat_shed());

        if (S.time.divisibleBy(S.dt_save)  && (double(S.time) >= 0))
        {
            char tmp_filename[256];
            snprintf(tmp_filename, 256, "%s/%%06d.h5", f_results);
            S.save(tmp_filename);
            stepdata.flush();
        }

        dbg(flowmove.vortex_shed());
        dbg(flowmove.streak_shed());

        dbg(S.calc_forces());
        stepdata.write();
        S.zero_forces();

        dbg(tr.build());
        dbg(epsilon.CalcEpsilonFast(/*merge=*/is_viscous));
        dbg(convective.process_all_lists());
        dbg(sens.output());
        if (is_viscous)
        {
            dbg(diffusive.process_vort_list());
            dbg(diffusive.process_heat_list());
        }
        dbg(tr.destroy());

        dbg(flowmove.move_and_clean(true, &collision));
            #ifdef OVERRIDEMOI
                    S.BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
            #endif
        dbg(flowmove.heat_crop());
        S.time = TTime::add(S.time, S.dt);

        if (opt::show_progress)
            fprintf(stderr, "\rt=%-10g \tN=%-10zd", double(S.time), S.VortexList.size());
    }

    if (opt::show_progress)
        fprintf(stderr, "\n");

    return 0;
}
