#include <cppunit/ui/text/TestRunner.h>
#include "test_TVec.hpp"
#include "test_TEval.hpp"
#include "test_TBody.hpp"
#include "test_TVec3D.hpp"
#include "test_XField.hpp"

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( TVecSuite::suite() );
  runner.addTest( TEvalSuite::suite() );
  runner.addTest( TBodySuite::suite() );
  runner.addTest( TVec3DSuite::suite() );
  runner.addTest( XFieldSuite::suite() );
  bool ok = runner.run();
  return ok ? 0 : 1;
}
