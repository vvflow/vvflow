# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit")
  file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit-build"
  "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix"
  "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/tmp"
  "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit-stamp"
  "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src"
  "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_make/cppunit-prefix/src/cppunit-stamp${cfgdir}") # cfgdir has leading slash
endif()
