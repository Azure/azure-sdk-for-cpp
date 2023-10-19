#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(MACRO_UTILS_INC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/include" CACHE INTERNAL "this is what needs to be included if using macro utils" FORCE)

if("${CMAKE_VERSION}" VERSION_GREATER 3.0.2)
    include("${CMAKE_CURRENT_LIST_DIR}/azure_macro_utils_cTargets.cmake")

    get_target_property(AZURE_MACRO_UTILS_C_INCLUDES azure_macro_utils_c INTERFACE_INCLUDE_DIRECTORIES)

    set(AZURE_MACRO_UTILS_C_INCLUDES ${AZURE_MACRO_UTILS_C_INCLUDES} CACHE INTERNAL "")
else()
    message(STATUS "This version of CMake does not support interface targets. To use Azure Macro Utils, simply add \"MACRO_UTILS_INC_FOLDER\" to your include directories as specified in the instructions on the GitHub README.")
endif()