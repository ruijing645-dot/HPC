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
set(BORIS_NOTEBOOK_TESTS
    test_01_boris_pusher.ipynb
    test_02_grid_field_push.ipynb
    test_03_cic.ipynb
    test_04_prediction_first.ipynb
    test_05_prediction_second.ipynb
    test_06_correction.ipynb
)
foreach(test_file IN LISTS BORIS_NOTEBOOK_TESTS)
    get_filename_component(test_name "${test_file}" NAME_WE)
    add_test(NAME "${test_name}" COMMAND ${Python3_EXECUTABLE} -m nbconvert
        --to notebook --execute --stdout "${CMAKE_CURRENT_SOURCE_DIR}/tests/${test_file}")
    set_tests_properties("${test_name}" PROPERTIES
        ENVIRONMENT "BORIS_MODULE_DIR=$<TARGET_FILE_DIR:boris_cpp>"
    )
endforeach()
