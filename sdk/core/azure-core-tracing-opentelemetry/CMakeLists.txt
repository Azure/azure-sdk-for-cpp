# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

# Azure core is compatible with CMake 3.12
cmake_minimum_required (VERSION 3.12)
project(azure-core-tracing-opentelemetry LANGUAGES CXX)

if (NOT VCPKG_TARGET_TRIPLET MATCHES "windows" OR VCPKG_TARGET_TRIPLET MATCHES "static" OR NOT WIN32)
    set(BUILD_AZURE_CORE_TRACING_OPENTELEMETRY ON)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
# Add create_map_file function
include(CreateMapFile)

if (BUILD_AZURE_CORE_TRACING_OPENTELEMETRY)
    find_package(Threads REQUIRED)

    if(NOT AZ_ALL_LIBRARIES)
      find_package(azure-core-cpp CONFIG QUIET)
      if(NOT azure-core-cpp_FOUND)
        find_package(azure-core-cpp REQUIRED)
      endif()
    endif()
    find_package(opentelemetry-cpp CONFIG REQUIRED)

    set(
      AZURE_CORE_OPENTELEMETRY_HEADER
        inc/azure/core/tracing/opentelemetry/dll_import_export.hpp
        inc/azure/core/tracing/opentelemetry/internal/apiview.hpp
        inc/azure/core/tracing/opentelemetry/opentelemetry.hpp
        inc/azure/core/tracing/opentelemetry/rtti.hpp
    )

    set(
      AZURE_CORE_OPENTELEMETRY_SOURCE
        src/opentelemetry.cpp
        src/opentelemetry_private.hpp
        src/private/package_version.hpp
    )

    add_library(azure-core-tracing-opentelemetry ${AZURE_CORE_OPENTELEMETRY_HEADER} ${AZURE_CORE_OPENTELEMETRY_SOURCE})

    target_include_directories(
      azure-core-tracing-opentelemetry
        PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
          $<INSTALL_INTERFACE:include>
    )

    target_compile_definitions(azure-core-tracing-opentelemetry PRIVATE _azure_BUILDING_SDK)

    # make sure that users can consume the project as a library.
    add_library(Azure::azure-core-tracing-opentelemetry ALIAS azure-core-tracing-opentelemetry)

    # coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
    create_code_coverage(core azure-core-tracing-opentelemetry azure-core-tracing-opentelemetry-test "tests?/*;samples?/*")

    target_link_libraries(azure-core-tracing-opentelemetry INTERFACE Threads::Threads)

    target_link_libraries(azure-core-tracing-opentelemetry PRIVATE azure-core
        opentelemetry-cpp::api
        opentelemetry-cpp::ext 
        opentelemetry-cpp::sdk 
        opentelemetry-cpp::trace)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-core-tracing-opentelemetry ${AZ_LIBRARY_VERSION})

if(BUILD_AZURE_CORE_TRACING_OPENTELEMETRY)
  set_target_properties(azure-core-tracing-opentelemetry PROPERTIES VERSION ${AZ_LIBRARY_VERSION})

  az_vcpkg_export(
    azure-core-tracing-opentelemetry
    CORE_TRACING_OPENTELEMETRY
    azure/core/tracing/opentelemetry/dll_import_export.hpp
  )

  az_rtti_setup(
    azure-core-tracing-opentelemetry
    CORE_TRACING_OPENTELEMETRY
    azure/core/tracing/opentelemetry/rtti.hpp
  )
endif()

if(BUILD_AZURE_CORE_TRACING_OPENTELEMETRY AND BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(_azure_TESTING_BUILD)
  
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
endif()
