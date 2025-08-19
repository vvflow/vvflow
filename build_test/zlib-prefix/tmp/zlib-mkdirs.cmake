# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib")
  file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib-build"
  "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix"
  "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/tmp"
  "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib-stamp"
  "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src"
  "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_test/zlib-prefix/src/zlib-stamp${cfgdir}") # cfgdir has leading slash
endif()
