#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./utils.h"
#include "../../libvvhd/test/expect.hpp"

class PidFileTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( PidFileTest );
    CPPUNIT_TEST( TestAll );
    CPPUNIT_TEST_SUITE_END();

protected:
    void TestAll();

public:
    void setUp() {
        auto tempdir = "/tmp/vvflow_test_results";
        mkdir(tempdir, 0777);
        CPPUNIT_ASSERT_EQUAL(chdir(tempdir), 0);
    }
    void tearDown() {}
};

int main( int argc, char **argv)
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( PidFileTest::suite() );
    bool ok = runner.run();
    return ok ? 0 : 1;
}

void PidFileTest::TestAll()
{
    CPPUNIT_ASSERT_NO_THROW({
        std::ofstream out(".test_pidfile.pid");
        out << "placeholder" << std::endl;
    });

    std::shared_ptr<PidFile> pf1 = std::make_shared<PidFile>("test_pidfile");

    std::string line;
    std::ifstream fa(".test_pidfile.pid");
    std::getline(fa, line);
    CPPUNIT_ASSERT_EQUAL(std::string(""), line);

    pf1->write();

    EXPECT_THROW_MSG(
        PidFile pf2("test_pidfile");,
        std::runtime_error,
        "Simulation already running: test_pidfile, pid: " + std::to_string(getpid())
    );

    pf1.reset();

    CPPUNIT_ASSERT_NO_THROW(
        PidFile pf3("test_pidfile");
    );

    errno = 0;
    struct stat pfstat;
    CPPUNIT_ASSERT_EQUAL(-1, stat(".test_pidfile.pid", &pfstat));
    CPPUNIT_ASSERT_EQUAL(ENOENT, errno);
}
