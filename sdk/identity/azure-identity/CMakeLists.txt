# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

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

add_library(azure-identity ${AZURE_IDENTITY_HEADER} ${AZURE_IDENTITY_SOURCE})
target_include_directories(azure-identity PUBLIC inc)
target_link_libraries(azure-identity azure-core)

add_library(azure::identity ALIAS azure-identity)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/identity/version.hpp")
generate_documentation(azure-identity ${AZ_LIBRARY_VERSION})
