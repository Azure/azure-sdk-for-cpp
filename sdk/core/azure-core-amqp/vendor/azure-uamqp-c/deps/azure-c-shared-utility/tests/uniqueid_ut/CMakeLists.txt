#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName uniqueid_ut)

if(WIN32)
    set(uniqueid_ut_test_file ${theseTestsName}_win32)
    generate_cppunittest_wrapper(${uniqueid_ut_test_file})
else()
    set(uniqueid_ut_test_file ${theseTestsName}_linux)
    generate_cppunittest_wrapper(${uniqueid_ut_test_file})
endif()


set(${uniqueid_ut_test_file}_c_files
${UNIQUEID_C_FILE}
)

set(${uniqueid_ut_test_file}_h_files
)

if(APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(UUID REQUIRED uuid)
endif()

if(WIN32)
    build_c_test_artifacts(${uniqueid_ut_test_file} ON "tests/azure_c_shared_utility_tests" ADDITIONAL_LIBS rpcrt4.lib)
elseif(UNIX)
    if(APPLE)
        build_c_test_artifacts(${uniqueid_ut_test_file} ON "tests/azure_c_shared_utility_tests" ADDITIONAL_LIBS -L${UUID_LIBRARY_DIRS} ${UUID_LIBRARIES})
    else()
        build_c_test_artifacts(${uniqueid_ut_test_file} ON "tests/azure_c_shared_utility_tests" ADDITIONAL_LIBS uuid)
    endif()
endif()
