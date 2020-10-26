#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "TVec.hpp"

class TVecSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( TVecSuite );
    CPPUNIT_TEST( TestConstructor );
    CPPUNIT_TEST_SUITE_END();
protected:
    void TestConstructor();
};

// CPPUNIT_TEST_SUITE_REGISTRATION( TVecSuite );

void TVecSuite::TestConstructor()
{
    TVec v0;
    CPPUNIT_ASSERT_EQUAL(v0.x, 0.);
    CPPUNIT_ASSERT_EQUAL(v0.y, 0.);

    {
        TVec v1(1, 1);
        CPPUNIT_ASSERT_EQUAL(v1.x, 1.);
        CPPUNIT_ASSERT_EQUAL(v1.y, 1.);
    }

    /* copy constructor */ {
        TVec v2(v0);
        v2.x = 2;
        v2.y = 2;
        CPPUNIT_ASSERT_EQUAL(v2.x, 2.);
        CPPUNIT_ASSERT_EQUAL(v2.y, 2.);
        CPPUNIT_ASSERT_EQUAL(v0.x, 0.);
        CPPUNIT_ASSERT_EQUAL(v0.y, 0.);
    }

    /* copy assignment */ {
        TVec v3 = v0;
        v3.x = 3;
        v3.y = 3;
        CPPUNIT_ASSERT_EQUAL(v3.x, 3.);
        CPPUNIT_ASSERT_EQUAL(v3.y, 3.);
        CPPUNIT_ASSERT_EQUAL(v0.x, 0.);
        CPPUNIT_ASSERT_EQUAL(v0.y, 0.);
    }

    /* move constructor */ {
        TVec v4(std::move(v0));
        v4.x = 4;
        v4.y = 4;
        CPPUNIT_ASSERT_EQUAL(v4.x, 4.);
        CPPUNIT_ASSERT_EQUAL(v4.y, 4.);
        CPPUNIT_ASSERT_EQUAL(v0.x, 0.);
        CPPUNIT_ASSERT_EQUAL(v0.y, 0.);
    }

    /* move assignment */ {
        TVec v5 = std::move(v0);
        v5.x = 5;
        v5.y = 5;
        CPPUNIT_ASSERT_EQUAL(v5.x, 5.);
        CPPUNIT_ASSERT_EQUAL(v5.y, 5.);
        CPPUNIT_ASSERT_EQUAL(v0.x, 0.);
        CPPUNIT_ASSERT_EQUAL(v0.y, 0.);
    }
}
