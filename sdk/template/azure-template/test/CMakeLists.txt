# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-template-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_DLLEXPORT_ALL_SYMBOLS ON)
add_compile_definitions(AZ_TEMPLATE_BEING_BUILT)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-template-test
     ut/template_test.cpp
     )

target_link_libraries(azure-template-test PRIVATE Azure::azure-template gtest_main)

if (MSVC)
    target_compile_options(azure-template-test PUBLIC /wd6326 /wd26495 /wd26812)

    # Since we could be built as either DLL or a static library, when we are built as static library,
    # linker does generate a bunch of warnings related to the use of dllimport attribute,
    # basically, suggesting that its use is unneccessary. We know that.
    # LNK4217: symbol 'symbol' defined in 'filename_1.obj' is imported by 'filename_2.obj' in function 'function'
    # LNK4286: symbol 'symbol' defined in 'filename_1.obj' is imported by 'filename_2.obj'
    target_link_options(azure-template-test PRIVATE /ignore:4217 /ignore:4286)
endif()

gtest_discover_tests(azure-template-test TEST_PREFIX azure-template.)

