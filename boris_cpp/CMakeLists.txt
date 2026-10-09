cmake_minimum_required(VERSION 3.20)
project(boris_cpp LANGUAGES CXX)

# Let editors consume the exact include paths resolved by CMake.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE pybind11_result
)
if(NOT pybind11_result EQUAL 0)
    message(FATAL_ERROR "pybind11 is required: ${Python3_EXECUTABLE} -m pip install pybind11")
endif()

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(boris_cpp src/bindings.cpp)
target_compile_features(boris_cpp PRIVATE cxx_std_20)
target_include_directories(boris_cpp PRIVATE include)

enable_testing()
add_test(NAME python_boris_tests COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_boris.py)
set_tests_properties(python_boris_tests PROPERTIES
    ENVIRONMENT "BORIS_MODULE_DIR=$<TARGET_FILE_DIR:boris_cpp>"
)
