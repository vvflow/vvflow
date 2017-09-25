#pragma once

#include "TEval.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

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
