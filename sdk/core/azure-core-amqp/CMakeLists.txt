# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
project(azure-core-amqp)

# Include sub-projects.
add_subdirectory ("src")

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
# Add create_map_file function
include(CreateMapFile)



get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})


target_include_directories(
  azure-core-amqp
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

  az_vcpkg_export(
    azure-core-amqp
    CORE_AMQP
    azure/core/amqp/inc/dll_import_export.hpp
  )

  az_rtti_setup(
    azure-core-amqp
    CORE_AMQP
    azure/core/amqp/rtti.hpp
  )
  # coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
  create_code_coverage(amqp azure-core-amqp azure-core-amqp-tests "tests?/*;samples?/*")

if(BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(TESTING_BUILD)
  
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
endif()

if(BUILD_SAMPLES)
add_subdirectory ("samples")
endif()