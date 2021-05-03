# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# VCPKG integration.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Project set up
cmake_minimum_required(VERSION 3.13)
project(Application-using-keyvault-from-vcpkg LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
find_package(azure-security-keyvault-keys-cpp CONFIG REQUIRED)
find_package(azure-identity-cpp CONFIG REQUIRED)

add_executable (
     application
     src/main
)

# Link to Azure SDK
target_link_libraries(application
     PRIVATE
     Azure::azure-security-keyvault-keys
     Azure::azure-identity
)
