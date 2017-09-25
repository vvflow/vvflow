#pragma once

#include "TBody.hpp"

#include <gtest/gtest.h>
// #include <stdexcept>

// using std::shared_ptr;
// using std::make_shared;

class TBodyTest : public ::testing::Test {
protected:
    std::shared_ptr<TBody> dummy;
    std::shared_ptr<TBody> jimmy;

    void SetUp()
    {
        dummy = std::make_shared<TBody>();
        dummy->label = "Dummy";
        dummy->alist.emplace_back(+1, +1); // 4    1
        dummy->alist.emplace_back(+1, -1); // ^    .
        dummy->alist.emplace_back(-1, -1); // '    v
        dummy->alist.emplace_back(-1, +1); // 3 <- 2
        dummy->doUpdateSegments();
        dummy->doFillProperties();

        jimmy = std::make_shared<TBody>();
        jimmy->label = "Jimmy";
        jimmy->alist.emplace_back(+2, -2); // 3 <- 2
        jimmy->alist.emplace_back(+2, +2); // .    ^
        jimmy->alist.emplace_back(-2, +2); // v    '
        jimmy->alist.emplace_back(-2, -2); // 4    1
        jimmy->doUpdateSegments();
        jimmy->doFillProperties();
    }
    void TearDown()
    {
        dummy.reset();
        jimmy.reset();
    }
};

TEST_F(TBodyTest, TAttTest)
{
    // won't compile: use of deleted function 'TAtt::TAtt()'
    // TAtt att;

    EXPECT_EQ(true, TAtt(0., 0., true).slip);
    EXPECT_EQ(false, TAtt(0., 0., false).slip);
    EXPECT_EQ(false, TAtt(0., 0.).slip);
    EXPECT_EQ(true, TAtt(TVec(0., 0.), true).slip);
    EXPECT_EQ(false, TAtt(TVec(0., 0.), false).slip);
    EXPECT_EQ(false, TAtt(TVec(0., 0.)).slip);

    TAtt a1 = TAtt(0., 0., true);
    a1.eq_no = 7;
    TAtt a2 = a1;
    TAtt a3 = std::move(a1);
    EXPECT_EQ(7, a1.eq_no);
    EXPECT_EQ(7, a2.eq_no);
    EXPECT_EQ(7, a3.eq_no);
}

TEST_F(TBodyTest, PropertiesDummy)
{
    EXPECT_EQ(4, dummy->size());
    EXPECT_EQ(false, dummy->isInsideValid());
    EXPECT_EQ(8.0, dummy->get_slen());
    EXPECT_EQ(4.0, dummy->get_area());
    EXPECT_EQ(0.0, dummy->get_cofm().abs2());
    EXPECT_EQ(16.0/6.0, dummy->get_moi_cofm());
    EXPECT_EQ(
        &dummy->alist.front(),
        dummy->isPointInvalid(TVec(0.9, 0.0))
    );
    EXPECT_EQ(
        nullptr,
        dummy->isPointInvalid(TVec(1.1, 0.0))
    );
}

TEST_F(TBodyTest, PropertiesJimmy)
{
    EXPECT_EQ(4, jimmy->size());
    EXPECT_EQ(true, jimmy->isInsideValid());
    EXPECT_EQ(16.0, jimmy->get_slen());
    EXPECT_EQ(-16.0, jimmy->get_area());
    EXPECT_EQ(0.0, jimmy->get_cofm().abs2());
    EXPECT_EQ(-256.0/6.0, jimmy->get_moi_cofm());
    EXPECT_EQ(
        nullptr,
        jimmy->isPointInvalid(TVec(1.9, 0.0))
    );
    EXPECT_EQ(
        &jimmy->alist.front(),
        jimmy->isPointInvalid(TVec(2.1, 0.0))
    );
}
