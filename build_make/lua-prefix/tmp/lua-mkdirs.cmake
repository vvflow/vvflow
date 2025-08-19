# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua")
  file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua-build"
  "/home/runner/work/vvflow/vvflow/build_make/lua-prefix"
  "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/tmp"
  "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua-stamp"
  "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src"
  "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/vvflow/vvflow/build_make/lua-prefix/src/lua-stamp${cfgdir}") # cfgdir has leading slash
endif()
