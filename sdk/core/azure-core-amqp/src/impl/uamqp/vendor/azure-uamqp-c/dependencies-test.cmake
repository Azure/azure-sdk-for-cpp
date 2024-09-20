#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if(${use_installed_dependencies})
    #These need to be set for the functions included by azure-c-shared-utility
    set(SHARED_UTIL_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-shared-utility/src")
    set(SHARED_UTIL_FOLDER "${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-shared-utility")
    set(SHARED_UTIL_ADAPTER_FOLDER "${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-shared-utility/adapters")
    set_platform_files("${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-shared-utility")
    find_package(umock_c REQUIRED CONFIG)
endif()