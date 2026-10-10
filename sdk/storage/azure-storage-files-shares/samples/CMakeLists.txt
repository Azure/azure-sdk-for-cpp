# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(file-share-getting-started file_share_getting_started.cpp)
target_compile_features(file-share-getting-started PRIVATE cxx_std_14)
set_target_properties(file-share-getting-started PROPERTIES CXX_STANDARD_REQUIRED ON)
create_per_service_target_build_for_sample(storage file-share-getting-started)
target_link_libraries(file-share-getting-started PRIVATE azure-storage-files-shares get-env-helper)
target_compile_definitions(file-share-getting-started PRIVATE _azure_BUILDING_SAMPLES)
