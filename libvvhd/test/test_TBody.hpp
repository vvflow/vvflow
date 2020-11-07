#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "TBody.hpp"

//  _________
// |j        |
// |   ___.  |
// |  |d  |  .
// |  |___|  |
// |         |
// |_________|

class TBodySuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( TBodySuite );
    CPPUNIT_TEST( TestThinBody );
    CPPUNIT_TEST( TestAttached );
    CPPUNIT_TEST( TestCopyConstructor );
    CPPUNIT_TEST_SUITE_END();

protected:
    std::shared_ptr<TBody> dummy;
    std::shared_ptr<TBody> jimmy;
    std::shared_ptr<TBody> thin;

    void TestThinBody();
    void TestAttached();
    void TestCopyConstructor();

public:
    TBodySuite(): dummy(), jimmy(), thin() {}

    void setUp()
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

        thin = std::make_shared<TBody>();
        thin->label = "thin";
        thin->alist.emplace_back(0.0, 1.0);
        thin->alist.emplace_back(0.5, 1.0);
        thin->alist.emplace_back(1.0, 1.0);
        thin->doUpdateSegments();
        thin->doFillProperties();
    }

    void tearDown()
    {
        dummy.reset();
        jimmy.reset();
        thin.reset();
    }
};

void TBodySuite::TestThinBody()
{
    CPPUNIT_ASSERT_EQUAL(thin->get_area(), 0.0);
    CPPUNIT_ASSERT_EQUAL(thin->get_moi_cofm(), 0.0);
    CPPUNIT_ASSERT_EQUAL(thin->get_moi_axis(), 0.0);
    CPPUNIT_ASSERT_EQUAL(thin->get_slen(), 2.0);
    TVec cofm = thin->get_cofm();
    CPPUNIT_ASSERT_EQUAL(cofm.x, 0.5);
    CPPUNIT_ASSERT_EQUAL(cofm.y, 1.0);
}

void TBodySuite::TestAttached()
{
    // won't compile: use of deleted function 'TAtt::TAtt()'
    // TAtt att;

    CPPUNIT_ASSERT_EQUAL(1U, TAtt(0., 0., true).slip);
    CPPUNIT_ASSERT_EQUAL(0U, TAtt(0., 0., false).slip);
    CPPUNIT_ASSERT_EQUAL(0U, TAtt(0., 0.).slip);
    CPPUNIT_ASSERT_EQUAL(1U, TAtt(TVec(0., 0.), true).slip);
    CPPUNIT_ASSERT_EQUAL(0U, TAtt(TVec(0., 0.), false).slip);
    CPPUNIT_ASSERT_EQUAL(0U, TAtt(TVec(0., 0.)).slip);

    TAtt a1 = TAtt(0., 0., true);
    a1.eq_no = 7;
    TAtt a2 = a1;
    TAtt a3 = TAtt(0., 0.); a3 = a1;
    TAtt a4 = std::move(a1);
    TAtt a5 = TAtt(0., 0.); a5 = std::move(a1);
    CPPUNIT_ASSERT_EQUAL(7U, a1.eq_no);
    CPPUNIT_ASSERT_EQUAL(7U, a2.eq_no);
    CPPUNIT_ASSERT_EQUAL(7U, a3.eq_no);
    CPPUNIT_ASSERT_EQUAL(7U, a4.eq_no);
    CPPUNIT_ASSERT_EQUAL(7U, a5.eq_no);
}

static void checkDummy(std::shared_ptr<TBody> dummy)
{
    if (!dummy) {
        CPPUNIT_FAIL(" invalid dummy");
    }
    CPPUNIT_ASSERT_EQUAL((size_t)4, dummy->size());
    CPPUNIT_ASSERT_EQUAL(false, dummy->isInsideValid());
    CPPUNIT_ASSERT_EQUAL(8.0, dummy->get_slen());
    CPPUNIT_ASSERT_EQUAL(4.0, dummy->get_area());
    CPPUNIT_ASSERT_EQUAL(0.0, dummy->get_cofm().abs2());
    CPPUNIT_ASSERT_EQUAL(16.0/6.0, dummy->get_moi_cofm());
    CPPUNIT_ASSERT((dummy->get_axis()-TVec(1., 1.)).iszero());
    CPPUNIT_ASSERT_EQUAL(32, dummy->eq_forces_no);
    CPPUNIT_ASSERT(dummy->root_body.expired());
    CPPUNIT_ASSERT_EQUAL(
        &dummy->alist.front(),
        dummy->isPointInvalid(TVec(0.9, 0.0))
    );
    CPPUNIT_ASSERT_EQUAL(
        (TAtt*)nullptr,
        dummy->isPointInvalid(TVec(1.1, 0.0))
    );
}

static void checkJimmy(std::shared_ptr<TBody> jimmy)
{
    if (!jimmy) {
        CPPUNIT_FAIL(" invalid jimmy");
    }
    CPPUNIT_ASSERT_EQUAL((size_t)4, jimmy->size());
    CPPUNIT_ASSERT_EQUAL(true, jimmy->isInsideValid());
    CPPUNIT_ASSERT_EQUAL(16.0, jimmy->get_slen());
    CPPUNIT_ASSERT_EQUAL(-16.0, jimmy->get_area());
    CPPUNIT_ASSERT_EQUAL(0.0, jimmy->get_cofm().abs2());
    CPPUNIT_ASSERT_EQUAL(-256.0/6.0, jimmy->get_moi_cofm());
    CPPUNIT_ASSERT((jimmy->get_axis()-TVec(2., 0.)).iszero());
    CPPUNIT_ASSERT_EQUAL(69, jimmy->eq_forces_no);
    checkDummy(jimmy->root_body.lock());
    CPPUNIT_ASSERT_EQUAL(
        (TAtt*)nullptr,
        jimmy->isPointInvalid(TVec(1.9, 0.0))
    );
    CPPUNIT_ASSERT_EQUAL(
        &jimmy->alist.front(),
        jimmy->isPointInvalid(TVec(2.1, 0.0))
    );
}

void TBodySuite::TestCopyConstructor()
{
    std::shared_ptr<TBody> ndummy = std::make_shared<TBody>(*dummy);
    std::shared_ptr<TBody>& njimmy = dummy;
    *njimmy = *jimmy;
    njimmy->root_body = ndummy;
    checkDummy(ndummy);
    checkJimmy(njimmy);

    // won't compile:
    // use of deleted function 'TBody::TBody(TBody&&)'
    // use of deleted function 'TBody& TBody::operator=(TBody&&)'
    // TBody b = std::move(*dummy);
    // b = std::move(*dummy);
}
