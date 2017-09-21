#pragma once
#include <gtest/gtest.h>
#include "TEval.hpp"

class TEvalTest : public ::testing::Test
{
};

TEST_F(TEvalTest, getValue)
{
    EXPECT_EQ(TEval("t+1").getValue(1), 3);
}
