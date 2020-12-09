# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-template LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_version)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

set(
  AZURE_TEMPLATE_HEADER
    inc/azure/template/template_client.hpp
    inc/azure/template/version.hpp
    inc/azure/template.hpp
)

set(
  AZURE_TEMPLATE_SOURCE
    src/template_client.cpp
    src/version.cpp
)

add_library(azure-template ${AZURE_TEMPLATE_HEADER} ${AZURE_TEMPLATE_SOURCE})

target_include_directories(azure-template PUBLIC inc)
target_link_libraries(azure-identity PUBLIC azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::Template ALIAS azure-template)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(template azure-template azure-template-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/template/version.hpp")
generate_documentation(azure-template ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
    # tests
    add_subdirectory(test)
endif()
