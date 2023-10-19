#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

set(ctest_ut_c_files
    assert_failures_with_msg_tests.c
    assertfailurestests.c
    assertsuccesstests.c
    ctestunittests.c
    enum_define_tests.c
    simpletestsuiteonetest.c
    simpletestsuitetwotests.c
    testfunctioncleanuptests.c
    testfunctioninitializetests.c
    testsuiteinitializecleanuptests.c
    whentestsuiteinitializefailstests.c
    whentestfunctioninitializefailstests.c
    testfunctioncleanupfailstests.c
    testsuitecleanuptests.c
    testsuitecleanuptests2.c
)

if (MSVC)
set(ctest_ut_c_files
    ${ctest_ut_c_files}
    windows_types_tests.c
)
endif()

set(ctest_ut_cpp_files
    simpletestsuiteonetestcpp.cpp
)

set(ctest_ut_h_files
)

include_directories(${CTEST_INC_FOLDER})

IF(WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

add_executable(ctest_ut ${ctest_ut_c_files} ${ctest_ut_cpp_files} ${ctest_ut_h_files})

set_target_properties(ctest_ut
               PROPERTIES
               FOLDER "tests/ctest_ut")

target_link_libraries(ctest_ut ctest)

if(${run_unittests})
	add_test(NAME ctest_ut COMMAND ctest_ut)
endif()
