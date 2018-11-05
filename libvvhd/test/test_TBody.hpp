#pragma once

#include "TBody.hpp"

#include <gtest/gtest.h>
// #include <stdexcept>

// using std::shared_ptr;
// using std::make_shared;


//  _________
// |j        |
// |   ___.  |
// |  |d  |  .
// |  |___|  |
// |         |
// |_________|

class TBodyTest : public ::testing::Test {
public:
    TBodyTest():
        dummy(),
        jimmy() {}
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
        dummy->holder.r = TVec(1., 0.);
        dummy->dpos.r =   TVec(0., 1.);
        dummy->eq_forces_no = 32;

        jimmy = std::make_shared<TBody>();
        jimmy->label = "Jimmy";
        jimmy->alist.emplace_back(+2, -2); // 3 <- 2
        jimmy->alist.emplace_back(+2, +2); // .    ^
        jimmy->alist.emplace_back(-2, +2); // v    '
        jimmy->alist.emplace_back(-2, -2); // 4    1
        jimmy->doUpdateSegments();
        jimmy->doFillProperties();
        jimmy->holder.r = TVec(-1., 0.);
        jimmy->dpos.r =   TVec( 3., 0.);
        jimmy->root_body = dummy;
        jimmy->eq_forces_no = 69;
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
    TAtt a3 = TAtt(0., 0.); a3 = a1;
    TAtt a4 = std::move(a1);
    TAtt a5 = TAtt(0., 0.); a5 = std::move(a1);
    EXPECT_EQ(7U, a2.eq_no);
    EXPECT_EQ(7U, a3.eq_no);
    EXPECT_EQ(7U, a1.eq_no);
    EXPECT_EQ(7U, a4.eq_no);
    EXPECT_EQ(7U, a5.eq_no);
}

::testing::AssertionResult checkDummy(std::shared_ptr<TBody> dummy)
{
    if (!dummy) {
        return ::testing::AssertionFailure() << " invalid dummy";
    }
    EXPECT_EQ(4U, dummy->size());
    EXPECT_EQ(false, dummy->isInsideValid());
    EXPECT_EQ(8.0, dummy->get_slen());
    EXPECT_EQ(4.0, dummy->get_area());
    EXPECT_EQ(0.0, dummy->get_cofm().abs2());
    EXPECT_EQ(16.0/6.0, dummy->get_moi_cofm());
    EXPECT_TRUE((dummy->get_axis()-TVec(1., 1.)).iszero());
    EXPECT_EQ(32, dummy->eq_forces_no);
    EXPECT_TRUE(dummy->root_body.expired());
    EXPECT_EQ(
        &dummy->alist.front(),
        dummy->isPointInvalid(TVec(0.9, 0.0))
    );
    EXPECT_EQ(
        nullptr,
        dummy->isPointInvalid(TVec(1.1, 0.0))
    );
    return ::testing::AssertionSuccess();
}

::testing::AssertionResult checkJimmy(std::shared_ptr<TBody> jimmy)
{
    if (!jimmy) {
        return ::testing::AssertionFailure() << " invalid jimmy";
    }
    EXPECT_EQ(4U, jimmy->size());
    EXPECT_EQ(true, jimmy->isInsideValid());
    EXPECT_EQ(16.0, jimmy->get_slen());
    EXPECT_EQ(-16.0, jimmy->get_area());
    EXPECT_EQ(0.0, jimmy->get_cofm().abs2());
    EXPECT_EQ(-256.0/6.0, jimmy->get_moi_cofm());
    EXPECT_TRUE((jimmy->get_axis()-TVec(2., 0.)).iszero());
    EXPECT_EQ(69, jimmy->eq_forces_no);
    EXPECT_TRUE(checkDummy(jimmy->root_body.lock()));
    EXPECT_EQ(
        nullptr,
        jimmy->isPointInvalid(TVec(1.9, 0.0))
    );
    EXPECT_EQ(
        &jimmy->alist.front(),
        jimmy->isPointInvalid(TVec(2.1, 0.0))
    );
    return ::testing::AssertionSuccess();
}

TEST_F(TBodyTest, ConstructorCopy)
{
    std::shared_ptr<TBody> ndummy = std::make_shared<TBody>(*dummy);
    std::shared_ptr<TBody>& njimmy = dummy;
    *njimmy = *jimmy;
    njimmy->root_body = ndummy;
    EXPECT_TRUE(checkDummy(ndummy));
    EXPECT_TRUE(checkJimmy(njimmy));

    // won't compile:
    // use of deleted function 'TBody::TBody(TBody&&)'
    // use of deleted function 'TBody& TBody::operator=(TBody&&)'
    // TBody b = std::move(*dummy);
    // b = std::move(*dummy);
}
