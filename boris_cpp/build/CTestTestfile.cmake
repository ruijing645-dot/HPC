# CMake generated Testfile for 
# Source directory: /home/ruijing/M2/HPC/boris_cpp
# Build directory: /home/ruijing/M2/HPC/boris_cpp/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[test_01_boris_pusher]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_01_boris_pusher.py")
set_tests_properties([=[test_01_boris_pusher]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/boris_cpp/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;35;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
add_test([=[test_02_grid_field_push]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_02_grid_field_push.py")
set_tests_properties([=[test_02_grid_field_push]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/boris_cpp/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;35;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
add_test([=[test_03_cic]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_03_cic.py")
set_tests_properties([=[test_03_cic]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/boris_cpp/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;35;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
add_test([=[test_04_prediction_first]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_04_prediction_first.py")
set_tests_properties([=[test_04_prediction_first]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/boris_cpp/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;35;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
add_test([=[test_05_prediction_second]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_05_prediction_second.py")
set_tests_properties([=[test_05_prediction_second]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/boris_cpp/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;35;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
add_test([=[test_06_correction]=] "/home/ruijing/.venv/bin/python3" "/home/ruijing/M2/HPC/boris_cpp/tests/test_06_correction.py")
set_tests_properties([=[test_06_correction]=] PROPERTIES  ENVIRONMENT "BORIS_MODULE_DIR=/home/ruijing/M2/HPC/boris_cpp/build" _BACKTRACE_TRIPLES "/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;35;add_test;/home/ruijing/M2/HPC/boris_cpp/CMakeLists.txt;0;")
