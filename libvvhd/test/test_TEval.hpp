#pragma once

#include "TEval.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <map>

class TEvalTest : public ::testing::Test {};

TEST_F(TEvalTest, ConstructDefault)
{
    TEval eval;
    EXPECT_EQ("", std::string(eval));
    EXPECT_EQ(0, eval.eval(-1));
    EXPECT_EQ(0, eval.eval(0));
    EXPECT_EQ(0, eval.eval(1));
    EXPECT_EQ(0, eval.eval(100));
}

TEST_F(TEvalTest, ConstructWithString)
{
    // malformed expression
    EXPECT_THROW(TEval eval(")");, std::invalid_argument);
    // invalid expression
    EXPECT_THROW(TEval eval("a+1");, std::invalid_argument);
    EXPECT_THROW(TEval eval("t(1)");, std::invalid_argument);
    EXPECT_THROW(TEval eval("x(1)");, std::invalid_argument);

    TEval eval("t-t");
    eval = "t+1";
    // invalid expression should not spoil evaluator
    EXPECT_THROW(eval = "b+1";, std::invalid_argument);
    EXPECT_EQ("(t+1)", std::string(eval));
    EXPECT_EQ(2, eval.eval(1));
    // valid expression should drop cache
    EXPECT_NO_THROW(eval = "2+t";);
    EXPECT_EQ("(2+t)", std::string(eval));
    EXPECT_EQ(3, eval.eval(1));

    // copy and move compile
    TEval e1("sin(t)");
    TEval e2(e1);
    EXPECT_EQ(e2.eval(0.1), std::sin(0.1));

    TEval e3; e3 = e1;
    EXPECT_EQ(e3.eval(0.2), std::sin(0.2));

    TEval e4(std::move(e1));
    EXPECT_EQ(e4.eval(0.3), std::sin(0.3));

    TEval e5; e5 = std::move(e1);
    EXPECT_EQ(e5.eval(0.4), std::sin(0.4));
}

TEST_F(TEvalTest, EvaluateConstants)
{
    const double pi = acos(-1);

    #define CHECK(x, y) EXPECT_THAT( \
        TEval(x).eval(0), \
        ::testing::NanSensitiveDoubleEq(y) \
    )
    CHECK("e", exp(1));
    CHECK("log2e", 1.L/log(2));
    CHECK("log10e", 1.L/log(10));
    CHECK("ln2", log(2));
    CHECK("ln10", log(10));
    CHECK("pi", pi);
    CHECK("pi_2", pi/2.L);
    CHECK("pi_4", pi/4.L);
    CHECK("1_pi", 1.L/pi);
    CHECK("2_pi", 2.L/pi);
    CHECK("2_sqrtpi", 2.L/sqrt(pi));
    CHECK("sqrt2", sqrt(2.L));
    CHECK("sqrt1_2", sqrt(0.5L));
    #undef CHECK
}

TEST_F(TEvalTest, EvaluateFunctions)
{
    std::map<const char*, double(*)(double)> functions = {
        {"exp(t)", exp},
        {"log(t)", log},
        {"log(t)", log},
        {"sqrt(t)", sqrt},
        {"sin(t)", sin},
        {"cos(t)", cos},
        {"tan(t)", tan},
        {"asin(t)", asin},
        {"acos(t)", acos},
        {"atan(t)", atan},
        {"sinh(t)", sinh},
        {"cosh(t)", cosh},
        {"tanh(t)", tanh},

        {"abs(t)", abs},
        {"step(t)", [](double x) -> double {
            if (isnan(x)) return x;
            return x>=0 ? 1 : 0;
        }},
        {"delta(t)", [](double x) -> double {
            if (isnan(x)) return x;
            return x==0 ? std::numeric_limits<double>::infinity() : 0;
        }},
        {"nandelta(t)", [](double x) -> double {
            if (isnan(x)) return x;
            return x==0 ? std::numeric_limits<double>::quiet_NaN() : 0;
        }},
        {"erf(t)", erf}
    };

    for (const auto& p: functions) {
        #define CHECK(x) EXPECT_THAT( \
            TEval(p.first).eval(x), \
            ::testing::NanSensitiveDoubleEq(p.second(x)) \
        )

        CHECK(0.0);
        CHECK(1.0);
        CHECK(2.0);
        CHECK(0.5);
        CHECK(-1.0);
        CHECK(std::numeric_limits<double>::min()); // DBL_MIN ~1e-308
        CHECK(std::numeric_limits<double>::max()); // DBL_MAX ~1e+308
        CHECK(std::numeric_limits<double>::lowest()); // -DBL_MAX
        CHECK(std::numeric_limits<double>::epsilon());
        CHECK(std::numeric_limits<double>::infinity());
        CHECK(std::numeric_limits<double>::quiet_NaN());

        #undef CHECK
    }
}
