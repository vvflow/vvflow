#include <cppunit/ui/text/TestRunner.h>
#include "test_TVec.hpp"
#include "test_TVec3D.hpp"

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( TVecSuite::suite() );
  runner.addTest( TVec3DSuite::suite() );
  runner.run();
  return 0;
}
