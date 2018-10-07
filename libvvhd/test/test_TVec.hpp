#pragma once

#include "TVec.hpp"

#include <gtest/gtest.h>

class TVecTest : public ::testing::Test {};

TEST_F(TVecTest, Construct)
{
    TVec v0;
    EXPECT_EQ(0, v0.x);
    EXPECT_EQ(0, v0.y);

    {
        TVec v1(1, 1);
        EXPECT_EQ(1, v1.x);
        EXPECT_EQ(1, v1.y);
    }

    /* copy constructor */ {
        TVec v2(v0);
        v2.x = 2;
        v2.y = 2;
        EXPECT_EQ(2, v2.x);
        EXPECT_EQ(2, v2.y);
        EXPECT_EQ(0, v0.x);
        EXPECT_EQ(0, v0.y);
    }

    /* copy assignment */ {
        TVec v3 = v0;
        v3.x = 3;
        v3.y = 3;
        EXPECT_EQ(3, v3.x);
        EXPECT_EQ(3, v3.y);
        EXPECT_EQ(0, v0.x);
        EXPECT_EQ(0, v0.y);
    }

    /* move constructor */ {
        TVec v4(std::move(v0));
        v4.x = 4;
        v4.y = 4;
        EXPECT_EQ(4, v4.x);
        EXPECT_EQ(4, v4.y);
        EXPECT_EQ(0, v0.x);
        EXPECT_EQ(0, v0.y);
    }

    /* move assignment */ {
        TVec v5 = std::move(v0);
        v5.x = 5;
        v5.y = 5;
        EXPECT_EQ(5, v5.x);
        EXPECT_EQ(5, v5.y);
        EXPECT_EQ(0, v0.x);
        EXPECT_EQ(0, v0.y);
    }
}
