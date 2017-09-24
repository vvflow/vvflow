#pragma once

#include "TEval.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

class TEvalTest : public ::testing::Test {};

TEST_F(TEvalTest, ConstructDefault)
{
    TEval eval;
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
    
    TEval eval("");
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
    TEval e2(eval);
    TEval e3; e3 = eval;
    TEval e4(std::move(eval));
    TEval e5; e5 = std::move(eval);
}
