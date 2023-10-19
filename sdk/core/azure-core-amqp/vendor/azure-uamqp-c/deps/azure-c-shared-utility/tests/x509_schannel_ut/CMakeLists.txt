#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName x509_schannel_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
../../adapters/x509_schannel.c
)

set(${theseTestsName}_h_files
)

#/*the following linker option will make x509_schannel.obj : warning LNK4217: locally defined symbol _CryptDecodeObjectEx@32 imported in function _x509_schannel_create go away*/
#/*the warning appears because CryptDecodeObjectEx is defined with declspec(dllexport) and the mocked functions in the unittests do not (cannot) adhere to that specification*/
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4217")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /ignore:4217")

#the below line strips advapi32.lib from CMAKE_C_STANDARD_LIBRARIES and from CMAKE_CXX_STANDARD_LIBRARIES

string(REPLACE "advapi32.lib" "" CMAKE_C_STANDARD_LIBRARIES ${CMAKE_C_STANDARD_LIBRARIES})
string(REPLACE "advapi32.lib" "" CMAKE_CXX_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES})

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
