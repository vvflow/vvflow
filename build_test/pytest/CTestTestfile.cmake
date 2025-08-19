# CMake generated Testfile for 
# Source directory: /home/runner/work/vvflow/vvflow/pytest
# Build directory: /home/runner/work/vvflow/vvflow/build_test/pytest
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(pytests "pipenv" "run" "pytest" "/home/runner/work/vvflow/vvflow/pytest")
set_tests_properties(pytests PROPERTIES  ENVIRONMENT "PATH=/home/runner/work/vvflow/vvflow/build_test/utils/vvflow/:/home/runner/work/vvflow/vvflow/build_test/utils/vvplot/:/home/runner/work/vvflow/vvflow/build_test/utils/vvxtract/:/home/runner/work/vvflow/vvflow/utils/scripts:/snap/bin:/home/runner/.local/bin:/opt/pipx_bin:/home/runner/.cargo/bin:/home/runner/.config/composer/vendor/bin:/usr/local/.ghcup/bin:/home/runner/.dotnet/tools:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin" _BACKTRACE_TRIPLES "/home/runner/work/vvflow/vvflow/pytest/CMakeLists.txt;4;add_test;/home/runner/work/vvflow/vvflow/pytest/CMakeLists.txt;0;")
add_test(flake8 "pipenv" "run" "flake8" "/home/runner/work/vvflow/vvflow/pytest")
set_tests_properties(flake8 PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/vvflow/vvflow/pytest/CMakeLists.txt;5;add_test;/home/runner/work/vvflow/vvflow/pytest/CMakeLists.txt;0;")
add_test(black "pipenv" "run" "black" "/home/runner/work/vvflow/vvflow/pytest" "--check" "--diff")
set_tests_properties(black PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/vvflow/vvflow/pytest/CMakeLists.txt;6;add_test;/home/runner/work/vvflow/vvflow/pytest/CMakeLists.txt;0;")
