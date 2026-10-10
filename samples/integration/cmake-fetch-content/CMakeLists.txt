# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Project set up
cmake_minimum_required(VERSION 3.13)
project(Application-using-storage-blobs LANGUAGES CXX)

#
# Fetch content
#
include(FetchContent)
FetchContent_Declare(azuresdkforcpp
    # Set the SDK URL path and release tag
    GIT_REPOSITORY      https://github.com/Azure/azure-sdk-for-cpp.git
    GIT_TAG             azure-storage-files-datalake_12.0.0-beta.6)
FetchContent_GetProperties(azuresdkforcpp)
if(NOT azuresdkforcpp_POPULATED)
    FetchContent_Populate(azuresdkforcpp)
    add_subdirectory(${azuresdkforcpp_SOURCE_DIR} ${azuresdkforcpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable (
     application
     src/main.cpp
)
target_compile_features(application PRIVATE cxx_std_14)
set_target_properties(application PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(application PRIVATE _azure_BUILDING_SAMPLES)

# Link to Azure SDK
target_link_libraries(application Azure::azure-storage-blobs get-env-helper)
