# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.12)
project(azure-messaging-eventhubs LANGUAGES CXX)

# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(MESSAGING)
endif()


include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
# Add create_map_file function
include(CreateMapFile)

if(FETCH_SOURCE_DEPS)
    GetFolderList(KEYS)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.5.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.5.0" REQUIRED)
  endif()
endif()

set(
  AZURE_MESSAGING_EVENTHUBS_HEADER
  inc/azure/messaging/eventhubs.hpp
  inc/azure/messaging/eventhubs/amqp_message.hpp
  inc/azure/messaging/eventhubs/producer_client.hpp
  inc/azure/messaging/eventhubs/event_data_batch.hpp
  inc/azure/messaging/eventhubs/retry_operation.hpp
  inc/azure/messaging/eventhubs/consumer_client.hpp
)

set(
  AZURE_MESSAGING_EVENTHUBS_SOURCE
  src/private/package_version.hpp
  src/producer_client.cpp
  src/retry_operation.cpp
  src/consumer_client.cpp
)
add_library(
  azure-messaging-eventhubs
    ${AZURE_MESSAGING_EVENTHUBS_HEADER} 
    ${AZURE_MESSAGING_EVENTHUBS_SOURCE}
 )

target_include_directories(azure-messaging-eventhubs  PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-messaging-eventhubs PUBLIC Azure::azure-core Azure::azure-core-amqp)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(eventhubs azure-messaging-eventhubs azure-messaging-eventhubs-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-messaging-eventhubs ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
  add_compile_definitions(TESTING_BUILD_AMQP)
  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
endif()

az_vcpkg_export(
    azure-messaging-eventhubs
    MESSAGING_EVENTHUBS
    azure/messaging/eventhubs/dll_import_export.hpp
  )

az_rtti_setup(
  azure-messaging-eventhubs
  MESSAGING_EVENTHUBS
  azure/messaging/eventhubs/rtti.hpp
)
