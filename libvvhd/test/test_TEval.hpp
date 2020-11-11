#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "TEval.hpp"
#include "expect.hpp"

#include <stdexcept>
#include <limits>
#include <map>

class TEvalSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( TEvalSuite );
    CPPUNIT_TEST( TestDefaultConstructor );
    CPPUNIT_TEST( TestStringConstructor );
    CPPUNIT_TEST( TestEvaluateConstants );
    CPPUNIT_TEST( TestEvaluateFunctions );
    CPPUNIT_TEST_SUITE_END();
protected:
    void TestDefaultConstructor();
    void TestStringConstructor();
    void TestEvaluateConstants();
    void TestEvaluateFunctions();
};

void TEvalSuite::TestDefaultConstructor()
{
    TEval eval;
    // CPPUNIT_ASSERT_EQUAL(v0.x, 0.);
    CPPUNIT_ASSERT_EQUAL(std::string(eval), std::string(""));
    CPPUNIT_ASSERT_EQUAL(eval.eval(-1), 0.);
    CPPUNIT_ASSERT_EQUAL(eval.eval(0), 0.);
    CPPUNIT_ASSERT_EQUAL(eval.eval(1), 0.);
    CPPUNIT_ASSERT_EQUAL(eval.eval(100), 0.);
}

void TEvalSuite::TestStringConstructor()
{
    // malformed expression
    CPPUNIT_ASSERT_THROW(TEval eval(")");, std::invalid_argument);
    // invalid expression
    CPPUNIT_ASSERT_THROW(TEval eval("a+1");, std::invalid_argument);
    CPPUNIT_ASSERT_THROW(TEval eval("t(1)");, std::invalid_argument);
    CPPUNIT_ASSERT_THROW(TEval eval("x(1)");, std::invalid_argument);

    TEval eval("t-t");
    eval = "t+1";
    // invalid expression should not spoil evaluator
    CPPUNIT_ASSERT_THROW(eval = "b+1";, std::invalid_argument);
    CPPUNIT_ASSERT_EQUAL(std::string(eval), std::string("t+1"));
    CPPUNIT_ASSERT_EQUAL(eval.eval(1), 2.0);
    // valid expression should drop cache
    CPPUNIT_ASSERT_NO_THROW(eval = "2+t";);
    CPPUNIT_ASSERT_EQUAL(std::string(eval), std::string("2+t"));
    CPPUNIT_ASSERT_EQUAL(eval.eval(1), 3.0);

    // copy and move compile
    TEval e1("sin(t)");
    TEval e2(e1);
    CPPUNIT_ASSERT_EQUAL(e2.eval(0.1), std::sin(0.1));

    TEval e3; e3 = e1;
    CPPUNIT_ASSERT_EQUAL(e3.eval(0.2), std::sin(0.2));

    TEval e4(std::move(e1));
    CPPUNIT_ASSERT_EQUAL(e4.eval(0.3), std::sin(0.3));

    TEval e5; e5 = std::move(e1);
    CPPUNIT_ASSERT_EQUAL(e5.eval(0.4), std::sin(0.4));
}

void TEvalSuite::TestEvaluateConstants()
{
    // Test the tests
    EXPECT_EQUAL(
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN()
    );
    CPPUNIT_ASSERT_THROW(EXPECT_EQUAL(
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity()
    ), CPPUNIT_NS::Exception);

    const double pi = acos(-1);

    #define CHECK(x, y) EXPECT_EQUAL(TEval(x).eval(0), y)
    CHECK("e", exp(1));
    CHECK("log2e", 1./log(2));
    CHECK("log10e", 1./log(10));
    CHECK("ln2", log(2));
    CHECK("ln10", log(10));
    CHECK("pi", pi);
    CHECK("pi_2", pi/2.);
    CHECK("pi_4", pi/4.);
    CHECK("sqrt2", sqrt(2.));
    CHECK("sqrt1_2", sqrt(0.5));
    #undef CHECK
}

static void CheckEvaluation(std::string expr, double fn(double))
{
    #define CHECK(x) EXPECT_EQUAL(TEval(expr).eval(x), fn(x))

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

void TEvalSuite::TestEvaluateFunctions()
{
    CheckEvaluation("exp(t)", exp);
    CheckEvaluation("log(t)", log);
    CheckEvaluation("sin(t)", sin);
    CheckEvaluation("cos(t)", cos);
    CheckEvaluation("tan(t)", tan);
    CheckEvaluation("erf(t)", erf);
    CheckEvaluation("abs(t)", fabs);
    CheckEvaluation("sqrt(t)", sqrt);
    CheckEvaluation("asin(t)", asin);
    CheckEvaluation("acos(t)", acos);
    CheckEvaluation("atan(t)", atan);
    CheckEvaluation("sinh(t)", sinh);
    CheckEvaluation("cosh(t)", cosh);
    CheckEvaluation("tanh(t)", tanh);

    CheckEvaluation("step(t)", [](double x) -> double {
        if (std::isnan(x)) return x;
        return x>=0 ? 1 : 0;
    });
    CheckEvaluation("delta(t)", [](double x) -> double {
        if (std::isnan(x)) return x;
        return x==0 ? std::numeric_limits<double>::infinity() : 0;
    });
    CheckEvaluation("nandelta(t)", [](double x) -> double {
        if (std::isnan(x)) return x;
        return x==0 ? std::numeric_limits<double>::quiet_NaN() : 0;
    });
}
