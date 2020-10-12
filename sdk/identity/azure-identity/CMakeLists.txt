# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

set (AZURE_IDENTITY_HEADER
    inc/azure/identity/client_secret_credential.hpp
    inc/azure/identity/environment_credential.hpp
)

set (AZURE_IDENTITY_SOURCE
    src/client_secret_credential.cpp
    src/environment_credential.cpp
)

add_library(azure-identity ${AZURE_IDENTITY_HEADER} ${AZURE_IDENTITY_SOURCE})
target_include_directories(azure-identity PUBLIC inc)
target_link_libraries(azure-identity azure-core)

# TODO: Do not read from version.txt, use version.hpp and replace this file
# command with:
# get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/identity/<package-name>/version.hpp")
# Access the variable from this function using ${AZ_LIBRARY_VERSION}
# Look at the CMakeLists.txt for template package for example usage
file(READ version.txt AZURE_IDENTITY_PACKAGE_VERSION)
target_compile_definitions(azure-identity PUBLIC AZURE_IDENTITY_PACKAGE_VERSION="${AZURE_IDENTITY_PACKAGE_VERSION}")
message("Azure Identity Package Version ${AZURE_IDENTITY_PACKAGE_VERSION}")
add_library(azure::identity ALIAS azure-identity)

generate_documentation(azure-identity ${AZURE_IDENTITY_PACKAGE_VERSION})
