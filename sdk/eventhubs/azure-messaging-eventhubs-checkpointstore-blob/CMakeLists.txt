# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-messaging-eventhubs-checkpointstore-blob LANGUAGES CXX)

# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(EVENTHUBS_CHECKPOINTSTORE_BLOB)
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
    GetFolderList(EVENTHUBS)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()

  find_package(azure-messaging-eventhubs-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-messaging-eventhubs-cpp REQUIRED)
  endif()

  find_package(azure-storage-blobs-cpp CONFIG QUIET)
  if(NOT azure-storage-blobs-cpp_FOUND)
    find_package(azure-storage-blobs-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_MESSAGING_EVENTHUBS_BLOB_CHECKPOINT_HEADER
    inc/azure/messaging/eventhubs/checkpointstore_blob/blob_checkpoint_store.hpp
    inc/azure/messaging/eventhubs/checkpointstore_blob/dll_import_export.hpp
    inc/azure/messaging/eventhubs/checkpointstore_blob/rtti.hpp
)

set(
  AZURE_MESSAGING_EVENTHUBS_BLOB_CHECKPOINT_SOURCE
    src/blob_checkpoint_store.cpp
    src/private/package_version.hpp
)

add_library(
  azure-messaging-eventhubs-checkpointstore-blob
    ${AZURE_MESSAGING_EVENTHUBS_BLOB_CHECKPOINT_HEADER} ${AZURE_MESSAGING_EVENTHUBS_BLOB_CHECKPOINT_SOURCE}
)
target_compile_definitions(azure-messaging-eventhubs-checkpointstore-blob PRIVATE _azure_BUILDING_SDK)
create_per_service_target_build(eventhubs azure-messaging-eventhubs-checkpointstore-blob)
add_library(Azure::azure-messaging-eventhubs-checkpointstore-blob ALIAS azure-messaging-eventhubs-checkpointstore-blob)

target_include_directories(
  azure-messaging-eventhubs-checkpointstore-blob
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-messaging-eventhubs-checkpointstore-blob 
    PUBLIC Azure::azure-core Azure::azure-messaging-eventhubs Azure::azure-storage-blobs
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(eventhubs azure-messaging-eventhubs-checkpointstore-blob azure-messaging-eventhubs-blobcheckpointstore-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-messaging-eventhubs-checkpointstore-blob PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
#generate_documentation(azure-messaging-eventhubs-checkpointstore-blob ${AZ_LIBRARY_VERSION})
generate_documentation(azure-messaging-eventhubs-checkpointstore-blob ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

az_vcpkg_export(
    azure-messaging-eventhubs-checkpointstore-blob
    MESSAGING_EVENTHUBS_CHECKPOINTSTORE_BLOB
    azure/messaging/eventhubs/checkpointstore_blob/dll_import_export.hpp
  )

az_rtti_setup(
  azure-messaging-eventhubs-checkpointstore-blob
  MESSAGING_EVENTHUBS_CHECKPOINTSTORE_BLOB
  azure/messaging/eventhubs/checkpointstore_blob/rtti.hpp
)

unset(FETCH_SOURCE_DEPS CACHE)
