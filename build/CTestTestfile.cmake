# CMake generated Testfile for 
# Source directory: /home/ruijing/M2/HPC/boris_cpp
# Build directory: /home/ruijing/M2/HPC/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[python_boris_tests]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_boris.py")
set_tests_properties([=[python_boris_tests]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;25;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
