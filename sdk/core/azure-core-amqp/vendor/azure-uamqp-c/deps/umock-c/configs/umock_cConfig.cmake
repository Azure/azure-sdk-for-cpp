#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if (DEFINED run_e2e_tests)
    set(original_run_e2e_tests ${run_e2e_tests})
else()
    set(original_run_e2e_tests OFF)
endif()

if (DEFINED run_int_tests)
    set(original_run_int_tests ${run_int_tests})
else()
    set(original_run_int_tests OFF)
endif()

if (DEFINED run_unittests)
    set(original_run_unittests ${run_unittests})
else()
    set(original_run_unittests OFF)
endif()


set(run_e2e_tests OFF)
set(run_int_tests OFF)
set(run_unittests OFF)

if(${original_run_int_tests} OR ${original_run_unittests})
    if(NOT ctest_FOUND)
        find_package(ctest REQUIRED CONFIG)
    endif()
    if(NOT testrunnerswitcher_FOUND)
        find_package(testrunnerswitcher REQUIRED CONFIG)
    endif()
endif()

set(run_e2e_tests ${original_run_e2e_tests})
set(run_int_tests ${original_run_int_tests})
set(run_unittests ${original_run_unittests})

include("${CMAKE_CURRENT_LIST_DIR}/umock_cTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/umock_cFunctions.cmake")

get_target_property(UMOCK_C_INCLUDES umock_c INTERFACE_INCLUDE_DIRECTORIES)

set(UMOCK_C_INCLUDES ${UMOCK_C_INCLUDES} CACHE INTERNAL "")
set(UMOCK_C_INC_FOLDER ${UMOCK_C_INCLUDES} CACHE INTERNAL "")

