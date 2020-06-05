#pragma once

#include "XField.hpp"
#include "TVec.hpp"
#include "expect.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <sstream>

float inf = std::numeric_limits<float>::infinity();
float NaN = std::numeric_limits<float>::quiet_NaN();

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


class XFieldTest : public ::testing::Test {};

TEST_F(XFieldTest, ConstructDefault)
{
    EXPECT_THROW_MSG(XField f(0, 0, 0, 1, 2);, std::invalid_argument, "XField(): xres must be > 1");
    EXPECT_THROW_MSG(XField f(0, 0, 0, 2, 1);, std::invalid_argument, "XField(): yres must be > 1");
    EXPECT_THROW_MSG(XField f(0, 0, 0, 2, 2);, std::invalid_argument, "XField(): dxdy must be positive");
    EXPECT_THROW_MSG(XField f(0, 0, -1, 2, 2);, std::invalid_argument, "XField(): dxdy must be positive");
    EXPECT_THROW_MSG(XField f(0, 0, inf, 2, 2);, std::invalid_argument, "XField(): dxdy must be finite");
    EXPECT_THROW_MSG(XField f(0, 0, NaN, 2, 2);, std::invalid_argument, "XField(): dxdy must be finite");

    YField field(1, 2, 3, 4, 5);
    EXPECT_EQ(1, field.xmin);
    EXPECT_EQ(2, field.ymin);
    EXPECT_EQ(3, field.dxdy);
    EXPECT_EQ(4, field.xres);
    EXPECT_EQ(5, field.yres);
    EXPECT_EQ(false, field.evaluated);
}

TEST_F(XFieldTest, Evaluate)
{
    YField field = {-1, -1, 1, 3, 3};
    field.evaluate();
    // N3  -1 0 1
    //     --------
    // +1 | 2 1 2
    //  0 | 1 0 1
    // -1 | 2 1 2

    EXPECT_EQ(2, field.at(0, 0));
    EXPECT_EQ(1, field.at(0, 1));
    EXPECT_EQ(0, field.at(1, 1));

    EXPECT_EQ(0, field.min());
    EXPECT_EQ(2, field.max());

    // zero values are ignored
    EXPECT_EQ(1, field.percentile(0.49));
    EXPECT_EQ(2, field.percentile(0.51));
}

std::string pack(std::vector<float> v) {
    std::stringstream bin;
    bin.write(reinterpret_cast<const char*>(v.data()), sizeof(float)*v.size());
    return bin.str();
}

TEST_F(XFieldTest, ConstructWithString)
{
    YField f1 = {1, 2, 1, 3, 4};
    f1.evaluate();

    std::stringstream ss;
    ss << f1;
    const std::string str = ss.str();
    EXPECT_EQ(sizeof(float)*(3+1)*(4+1), str.size());
    YField f2(str);
    EXPECT_NO_THROW(ss << f2);
    EXPECT_EQ(f1.xmin, f2.xmin);
    EXPECT_EQ(f1.ymin, f2.ymin);
    EXPECT_EQ(f1.dxdy, f2.dxdy);
    EXPECT_EQ(f1.xres, f2.xres);
    EXPECT_EQ(f1.yres, f2.yres);

    if (memcmp(f1.map, f2.map, f1.xres*f2.yres*sizeof(float)) != 0) {
        FAIL() << "XField maps not equal";
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
        XField f(pack({inf, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");
    EXPECT_THROW_MSG(
        XField f(pack({NaN, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): invalid data");

    EXPECT_THROW_MSG(
        XField f(pack({3.0, 1, 2,   1, 0, 0,   2, 0, 0}));,
        std::invalid_argument, "XField(str): can't factorize matrix");
    EXPECT_THROW_MSG(
        XField f(pack({2.0, 1, 2,   1, 0, 0,   3, 0, 0}));,
        std::invalid_argument, "XField(str): non-uniform grid");
}

