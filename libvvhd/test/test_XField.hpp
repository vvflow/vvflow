#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "expect.hpp"
#include "XField.hpp"
#include "TVec.hpp"

#include <stdexcept>
#include <sstream>
#include <cstring>

float f_inf = std::numeric_limits<float>::infinity();
float f_nan = std::numeric_limits<float>::quiet_NaN();

class YField: public XField {
public:
    // YField(
    //     double xmin, double ymin,
    //     double dxdy,
    //     int xres, int yres
    // ): XField(xmin, ymin, dxdy, xres, yres) {};
    // YField(const std::string& str): XField(str) {};
    void evaluate() {
        for (int yj=0; yj<yres; yj++)
        {
            for (int xi=0; xi<xres; xi++)
            {
                TVec p = {xmin + xi*dxdy, ymin + yj*dxdy};
                map[yj*xres+xi] = p.abs2();
            }
        }
        evaluated = true;
    };

public:
    using XField::XField;
    using XField::xmin;
    using XField::ymin;
    using XField::dxdy;
    using XField::xres;
    using XField::yres;
    using XField::map;
    using XField::evaluated;
};


class XFieldSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( XFieldSuite );
    CPPUNIT_TEST( TestDefaultConstructor );
    CPPUNIT_TEST( TestStringConstructor );
    CPPUNIT_TEST( TestEvaluate );
    CPPUNIT_TEST_SUITE_END();
protected:
    void TestDefaultConstructor();
    void TestStringConstructor();
    void TestEvaluate();
};

void XFieldSuite::TestDefaultConstructor()
{
    EXPECT_THROW_MSG(XField f(0, 0, 0, 1, 2);, std::invalid_argument, "XField(): xres must be > 1");
    EXPECT_THROW_MSG(XField f(0, 0, 0, 2, 1);, std::invalid_argument, "XField(): yres must be > 1");
    EXPECT_THROW_MSG(XField f(0, 0, 0, 2, 2);, std::invalid_argument, "XField(): dxdy must be positive");
    EXPECT_THROW_MSG(XField f(0, 0, -1, 2, 2);, std::invalid_argument, "XField(): dxdy must be positive");
    EXPECT_THROW_MSG(XField f(0, 0, f_inf, 2, 2);, std::invalid_argument, "XField(): dxdy must be finite");
    EXPECT_THROW_MSG(XField f(0, 0, f_nan, 2, 2);, std::invalid_argument, "XField(): dxdy must be finite");

    YField field(1, 2, 3, 4, 5);
    CPPUNIT_ASSERT_EQUAL(1.f, field.xmin);
    CPPUNIT_ASSERT_EQUAL(2.f, field.ymin);
    CPPUNIT_ASSERT_EQUAL(3.f, field.dxdy);
    CPPUNIT_ASSERT_EQUAL(4, field.xres);
    CPPUNIT_ASSERT_EQUAL(5, field.yres);
    CPPUNIT_ASSERT_EQUAL(false, field.evaluated);
}

void XFieldSuite::TestEvaluate()
{
    YField field = {-1, -1, 1, 3, 3};
    field.evaluate();
    // N3  -1 0 1
    //     --------
    // +1 | 2 1 2
    //  0 | 1 0 1
    // -1 | 2 1 2

    CPPUNIT_ASSERT_EQUAL(2.f, field.at(0, 0));
    CPPUNIT_ASSERT_EQUAL(1.f, field.at(0, 1));
    CPPUNIT_ASSERT_EQUAL(0.f, field.at(1, 1));

    CPPUNIT_ASSERT_EQUAL(0.f, field.min());
    CPPUNIT_ASSERT_EQUAL(2.f, field.max());

    // zero values are ignored
    CPPUNIT_ASSERT_EQUAL(1.f, field.percentile(0.49));
    CPPUNIT_ASSERT_EQUAL(2.f, field.percentile(0.51));
}

std::string pack(std::vector<float> v) {
    std::stringstream bin;
    bin.write(reinterpret_cast<const char*>(v.data()), sizeof(float)*v.size());
    return bin.str();
}

void XFieldSuite::TestStringConstructor()
{
    std::cout << std::endl;
    YField f1 = {1, 2, 1, 3, 4};
    f1.evaluate();

    std::stringstream ss;
    ss << f1;
    const std::string str = ss.str();
    CPPUNIT_ASSERT_EQUAL(sizeof(float)*(3+1)*(4+1), str.size());
    YField f2(str);
    CPPUNIT_ASSERT_NO_THROW(ss << f2);
    CPPUNIT_ASSERT_EQUAL(f1.xmin, f2.xmin);
    CPPUNIT_ASSERT_EQUAL(f1.ymin, f2.ymin);
    CPPUNIT_ASSERT_EQUAL(f1.dxdy, f2.dxdy);
    CPPUNIT_ASSERT_EQUAL(f1.xres, f2.xres);
    CPPUNIT_ASSERT_EQUAL(f1.yres, f2.yres);

    if (memcmp(f1.map, f2.map, f1.xres*f2.yres*sizeof(float)) != 0) {
        CPPUNIT_FAIL("XField maps not equal");
    }

    EXPECT_THROW_MSG(
        XField f(std::string(1, 'x'));,
        std::invalid_argument, "XField(str): odd string");
    EXPECT_THROW_MSG(
        XField f(std::string(4*1, 'x'));,
        std::invalid_argument, "XField(str): insufficient length");

    EXPECT_THROW_MSG(
        XField f(pack({-1., 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");
    EXPECT_THROW_MSG(
        XField f(pack({1.0, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");
    EXPECT_THROW_MSG(
        XField f(pack({7.5, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");
    EXPECT_THROW_MSG(
        XField f(pack({f_inf, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");
    EXPECT_THROW_MSG(
        XField f(pack({f_nan, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");

    EXPECT_THROW_MSG(
        XField f(pack({3.0, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): can't factorize matrix");
    EXPECT_THROW_MSG(
        XField f(pack({2.0, 1, 2,   1, 0, 0,   3, 0, 0}));,
        std::invalid_argument, "XField(str): non-uniform grid");
}

