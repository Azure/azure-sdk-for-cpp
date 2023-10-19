#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists for testtools. It does nothing, except loads other folders

include("../configs/azure_c_shared_utilityFunctions.cmake")

usePermissiveRulesForSamplesAndTests()

if(NOT ${use_installed_dependencies})
    if ((NOT TARGET ctest) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/ctest/CMakeLists.txt))
        add_subdirectory(ctest)
    endif()
    if ((NOT TARGET testrunnerswitcher) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/testrunner/CMakeLists.txt))
        add_subdirectory(testrunner)
    endif()
endif()

add_subdirectory(sal)
add_subdirectory(micromock)
