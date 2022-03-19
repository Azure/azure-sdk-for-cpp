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
project(smoketest-vcpkg LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  smoketest-vcpkg
    src/main
)

# Link to Azure SDK
target_link_libraries(smoketest-vcpkg
     PRIVATE
     Azure::azure-security-keyvault-keys
     Azure::azure-security-keyvault-certificates
     Azure::azure-security-keyvault-secrets
     Azure::azure-identity
     get-env-helper
)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
create_per_service_target_build(smoketest-vcpkg keyvault true)
