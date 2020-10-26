#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "TVec3D.hpp"

class TVec3DSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( TVec3DSuite );
    CPPUNIT_TEST( TestConstructor );
    CPPUNIT_TEST_SUITE_END();
protected:
    void TestConstructor();
};

// CPPUNIT_TEST_SUITE_REGISTRATION( TVec3DSuite );

void TVec3DSuite::TestConstructor()
{
    TVec3D v0;
    CPPUNIT_ASSERT_EQUAL(v0.r.x, 0.);
    CPPUNIT_ASSERT_EQUAL(v0.r.y, 0.);
    CPPUNIT_ASSERT_EQUAL(v0.o,   0.);

    {
        TVec3D v1(1.1, 1.2, 1.3);
        CPPUNIT_ASSERT_EQUAL(v1.r.x, 1.1);
        CPPUNIT_ASSERT_EQUAL(v1.r.y, 1.2);
        CPPUNIT_ASSERT_EQUAL(v1.o,   1.3);
    }

    /* copy constructor */ {
        TVec3D v2(v0);
        v2.r.x = 2.1;
        v2.r.y = 2.2;
        v2.o = 2.3;
        CPPUNIT_ASSERT_EQUAL(v2.r.x, 2.1);
        CPPUNIT_ASSERT_EQUAL(v2.r.y, 2.2);
        CPPUNIT_ASSERT_EQUAL(v2.o,   2.3);
        CPPUNIT_ASSERT_EQUAL(v0.r.x, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.r.y, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.o,   0.0);
    }

    /* copy assignment */ {
        TVec3D v3 = v0;
        v3.r.x = 3.1;
        v3.r.y = 3.2;
        v3.o = 3.3;
        CPPUNIT_ASSERT_EQUAL(v3.r.x, 3.1);
        CPPUNIT_ASSERT_EQUAL(v3.r.y, 3.2);
        CPPUNIT_ASSERT_EQUAL(v3.o,   3.3);
        CPPUNIT_ASSERT_EQUAL(v0.r.x, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.r.y, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.o,   0.0);
    }

    /* move constructor */ {
        TVec3D v4(std::move(v0));
        v4.r.x = 4.1;
        v4.r.y = 4.2;
        v4.o = 4.3;
        CPPUNIT_ASSERT_EQUAL(v4.r.x, 4.1);
        CPPUNIT_ASSERT_EQUAL(v4.r.y, 4.2);
        CPPUNIT_ASSERT_EQUAL(v4.o,   4.3);
        CPPUNIT_ASSERT_EQUAL(v0.r.x, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.r.y, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.o,   0.0);
    }

    /* move assignment */ {
        TVec3D v5 = std::move(v0);
        v5.r.x = 5.1;
        v5.r.y = 5.2;
        v5.o = 5.3;
        CPPUNIT_ASSERT_EQUAL(v5.r.x, 5.1);
        CPPUNIT_ASSERT_EQUAL(v5.r.y, 5.2);
        CPPUNIT_ASSERT_EQUAL(v5.o,   5.3);
        CPPUNIT_ASSERT_EQUAL(v0.r.x, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.r.y, 0.0);
        CPPUNIT_ASSERT_EQUAL(v0.o,   0.0);
    }
}
