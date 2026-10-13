# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
# cspell: words DGCOV DGCOVR

cmake_minimum_required(VERSION 3.13)
project(azure-code-coverage-test LANGUAGES CXX)

set(BUILD_CODE_COVERAGE ON)
include("${CMAKE_CURRENT_LIST_DIR}/../../AzureCodeCoverage.cmake")
append_code_coverage_for_current_project()

if(NOT GCOVR_PATH)
    message(FATAL_ERROR "gcovr is required for the coverage regression test.")
endif()

find_package(Threads REQUIRED)
add_executable(coverage-test coverage_test.cpp)
target_compile_features(coverage-test PRIVATE cxx_std_14)
target_link_libraries(coverage-test PRIVATE Threads::Threads)

enable_testing()
add_test(NAME atomic-coverage
    COMMAND ${CMAKE_COMMAND}
        -DTEST_EXECUTABLE=$<TARGET_FILE:coverage-test>
        -DGCOV=${GCOV}
        -DGCOVR=${GCOVR_PATH}
        -DTEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DTEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/check-coverage.cmake
)
set_tests_properties(atomic-coverage PROPERTIES TIMEOUT 60)
