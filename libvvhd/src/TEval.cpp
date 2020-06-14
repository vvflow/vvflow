#include "TEval.hpp"

#include <stdexcept>
#include <cstring> // strcmp
#include <limits>
#include <cmath>
#include <lua.hpp>

#define L ((lua_State*)lua_state)

static const double d_nan = std::numeric_limits<double>::quiet_NaN();

int lua_erf(lua_State* _L)
{
    int isnum;
    double x = lua_tonumberx(_L, -1, &isnum);
    if (!isnum) {
        luaL_error(_L,
            "bad argument #1 to 'sin'"
            " (number expected, got %s)",
            lua_typename(_L, lua_type(_L, -1))
        );
    }

    lua_pushnumber(_L, erf(x));
    return 1;
}

TEval::TEval():
    lua_state(luaL_newstate()), expr(),
    cacheTime1(d_nan),
    cacheTime2(d_nan),
    cacheValue1(),
    cacheValue2()
{
    #define SETNUMBER(name, value) \
        lua_pushnumber(L, value); \
        lua_setglobal(L, name);

    const double pi = acos(-1);
    SETNUMBER("e",        exp(1));
    SETNUMBER("log2e",    1.L/log(2));
    SETNUMBER("log10e",   1.L/log(10));
    SETNUMBER("ln2",      log(2));
    SETNUMBER("ln10",     log(10));
    SETNUMBER("pi",       pi);
    SETNUMBER("pi_2",     pi/2.L);
    SETNUMBER("pi_4",     pi/4.L);
    SETNUMBER("1_pi",     1.L/pi);
    SETNUMBER("2_pi",     2.L/pi);
    SETNUMBER("2_sqrtpi", 2.L/sqrt(pi));
    SETNUMBER("sqrt2",    sqrt(2.L));
    SETNUMBER("sqrt1_2",  sqrt(0.5L));
    #undef SETNUMBER

    luaL_openlibs(L);
    luaL_loadstring(L, R"(
        sin = math.sin
        exp = math.exp
        log = math.log
        sqrt = math.sqrt
        sin = math.sin
        cos = math.cos
        tan = math.tan
        asin = math.asin
        acos = math.acos
        atan = math.atan
        sinh = math.sinh
        cosh = math.cosh
        tanh = math.tanh
        abs = math.abs
        step = function(x)
            if x ~= x then return x
            elseif x >= 0 then return 1
            else return 0
            end
        end
        delta = function(x)
            if x ~= x then return x
            elseif x == 0 then return 1/0
            else return 0
            end
        end
        nandelta = function(x)
            if x ~= x then return x
            elseif x == 0 then return 0/0
            else return 0
            end
        end
    )");
    lua_pcall(L, 0, 0, 0);

    lua_pushcfunction(L, lua_erf);
    lua_setglobal(L, "erf");
}

TEval::~TEval()
{
    lua_close(L);
}

TEval::TEval(const TEval& copy): TEval()
{
    *this = std::string(copy);
}


TEval::TEval(const std::string& str): TEval()
{
    *this = str;
}

TEval& TEval::operator=(const TEval& copy)
{
    return *this = std::string(copy);
}

int loadstring(lua_State* _L, const std::string& str)
{
    return luaL_loadbuffer(_L, str.c_str(), str.size(), "@TEval");
}

TEval& TEval::operator=(const std::string& str)
{
    if (str.empty()) {
        expr.clear();
        lua_pushnil(L);
        lua_setglobal(L, "_evaluator");
        return *this;
    }

    int err;
    err = loadstring(L, "return " + str);
    if (err) {
        lua_pop(L, 1);
        err = loadstring(L, str);
    }
    if (err) {
        std::string e = lua_tostring(L, -1);
        lua_pop(L, 1);
        throw std::invalid_argument(e);
    }

    lua_pushvalue(L, -1);
    lua_pushnumber(L, 0);
    lua_setglobal(L, "t");
    err = lua_pcall(L, 0, 1, 0);
    if (err) {
        std::string e = lua_tostring(L, -1);
        lua_pop(L, 2);
        throw std::invalid_argument(e);
    } else if (!lua_isnumber(L, -1)) {
        lua_pop(L, 2);
        throw std::invalid_argument("invalid return type");
    } else {
        lua_pop(L, 1);
    }

    expr = str;
    cacheTime1 = cacheTime2 = d_nan;
    lua_setglobal(L, "_evaluator");
    return *this;
}

double TEval::eval(double t) const
{
    if (expr.empty())
        return 0;

    double ret = 0;
#pragma omp critical
    {
        if (t == cacheTime2) {
            ret = cacheValue2;
        } else if (t == cacheTime1) {
            ret = cacheValue1;
        } else {
            lua_pushnumber(L, t);
            lua_setglobal(L, "t");
            int err;

            lua_getglobal(L, "_evaluator");
            err = lua_pcall(L, 0, 1, 0);
            if (err) {
                std::string e = lua_tostring(L, -1);
                lua_pop(L, 1);
                throw std::invalid_argument(e);
            }
            int isnum;
            ret = lua_tonumberx(L, -1, &isnum);
            lua_pop(L, 1);
            if (!isnum) {
                throw std::invalid_argument("invalid return type");
            }

            cacheTime2 = cacheTime1;
            cacheValue2 = cacheValue1;
            cacheTime1 = t;
            cacheValue1 = ret;
        }
    }

    return ret;
}
