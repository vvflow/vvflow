#pragma once

#include "TVec3D.hpp"

#include <gtest/gtest.h>

class TVec3DTest : public ::testing::Test {};

TEST_F(TVec3DTest, Construct)
{
    TVec3D v0;
    EXPECT_EQ(0, v0.r.x);
    EXPECT_EQ(0, v0.r.y);
    EXPECT_EQ(0, v0.o);

    {
        TVec3D v1(1.1, 1.2, 1.3);
        EXPECT_EQ(1.1, v1.r.x);
        EXPECT_EQ(1.2, v1.r.y);
        EXPECT_EQ(1.3, v1.o);
    }

    /* copy constructor */ {
        TVec3D v2(v0);
        v2.r.x = 2.1;
        v2.r.y = 2.2;
        v2.o = 2.3;
        EXPECT_EQ(2.1, v2.r.x);
        EXPECT_EQ(2.2, v2.r.y);
        EXPECT_EQ(2.3, v2.o);
        EXPECT_EQ(0, v0.r.x);
        EXPECT_EQ(0, v0.r.y);
        EXPECT_EQ(0, v0.o);
    }

    /* copy assignment */ {
        TVec3D v3 = v0;
        v3.r.x = 3.1;
        v3.r.y = 3.2;
        v3.o = 3.3;
        EXPECT_EQ(3.1, v3.r.x);
        EXPECT_EQ(3.2, v3.r.y);
        EXPECT_EQ(3.3, v3.o);
        EXPECT_EQ(0, v0.r.x);
        EXPECT_EQ(0, v0.r.y);
        EXPECT_EQ(0, v0.o);
    }

    /* move constructor */ {
        TVec3D v4(std::move(v0));
        v4.r.x = 4.1;
        v4.r.y = 4.2;
        v4.o = 4.3;
        EXPECT_EQ(4.1, v4.r.x);
        EXPECT_EQ(4.2, v4.r.y);
        EXPECT_EQ(4.3, v4.o);
        EXPECT_EQ(0, v0.r.x);
        EXPECT_EQ(0, v0.r.y);
        EXPECT_EQ(0, v0.o);
    }

    /* move assignment */ {
        TVec3D v5 = std::move(v0);
        v5.r.x = 5.1;
        v5.r.y = 5.2;
        v5.o = 5.3;
        EXPECT_EQ(5.1, v5.r.x);
        EXPECT_EQ(5.2, v5.r.y);
        EXPECT_EQ(5.3, v5.o);
        EXPECT_EQ(0, v0.r.x);
        EXPECT_EQ(0, v0.r.y);
        EXPECT_EQ(0, v0.o);
    }
}
