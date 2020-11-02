# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

if (NOT azure-core IN_LIST AZ_LOCAL_LIBS)
    find_package(azure-core-cpp CONFIG)
    if(NOT azure-core-cpp_FOUND)
      find_package(azure-core-cpp REQUIRED)
    endif()
endif()

set (AZURE_IDENTITY_HEADER
    inc/azure/identity/client_secret_credential.hpp
    inc/azure/identity/environment_credential.hpp
    inc/azure/identity/version.hpp
)

set (AZURE_IDENTITY_SOURCE
    src/client_secret_credential.cpp
    src/environment_credential.cpp
    src/version.cpp
)

# Uncomment once identity have tests
# CodeCoverage will validate requirements
# include(CodeCoverage)

add_library(azure-identity ${AZURE_IDENTITY_HEADER} ${AZURE_IDENTITY_SOURCE})
target_include_directories(azure-identity PUBLIC inc Azure::Core)
target_link_libraries(azure-identity Azure::Core)

# Uncomment once identity have tests
# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
# create_code_coverage(identity azure-identity "azure-identity-test")

add_library(Azure::Identity ALIAS azure-identity)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/identity/version.hpp")
generate_documentation(azure-identity ${AZ_LIBRARY_VERSION})

list(APPEND AZ_LOCAL_LIBS azure-identity)
set(AZ_LOCAL_LIBS ${AZ_LOCAL_LIBS} PARENT_SCOPE)
