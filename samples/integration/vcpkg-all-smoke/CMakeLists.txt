# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# vcpkg integration.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Project set up
cmake_minimum_required(VERSION 3.13)
project(application-using-all-clients-from-vcpkg LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  application-using-all-clients-from-vcpkg
    src/main
)

# Link to Azure SDK
target_link_libraries(application-using-all-clients-from-vcpkg
     PRIVATE
     Azure::azure-security-keyvault-keys
     Azure::azure-identity
     get-env-helper
)
