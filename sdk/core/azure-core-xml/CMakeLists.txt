# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-core-xml LANGUAGES CXX)

include(AzureTransportAdapters)
include(AzureBuildTargetForCI)
include(AzureVersion)
include(AzureCodeCoverage)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
# Add create_map_file function
include(CreateMapFile)


if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_CORE_XML_HEADER
    inc/azure/core/xml.hpp
    inc/azure/core/xml/dll_import_export.hpp
    inc/azure/core/xml/rtti.hpp
    inc/azure/core/xml/xml_wrapper.hpp
)

set(
  AZURE_CORE_XML_SOURCE
    src/private/package_version.hpp
    src/xml_wrapper.cpp
)

add_library(azure-core-xml ${AZURE_CORE_XML_HEADER} ${AZURE_CORE_XML_SOURCE})
create_per_service_target_build(core-xml azure-core-xml)

target_include_directories(
  azure-core-xml
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-core-xml PUBLIC Azure::azure-core)

if(WIN32)
    target_link_libraries(azure-core-xml PRIVATE bcrypt webservices)
else()
    find_package(LibXml2 REQUIRED)
    target_include_directories(azure-core-xml SYSTEM PRIVATE ${LIBXML2_INCLUDE_DIRS})
    target_link_libraries(azure-core-xml PRIVATE ${LIBXML2_LIBRARIES})
endif()
# make sure that users can consume the project as a library.
add_library(Azure::azure-core-xml ALIAS azure-core-xml)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core-xml azure-core-xml azure-core-xml-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-core-xml ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-core-xml
    CORE_XML
    azure/core/xml/dll_import_export.hpp
  )

az_rtti_setup(
  azure-core-xml
  CORE_XML
  azure/core/xml/rtti.hpp
)
