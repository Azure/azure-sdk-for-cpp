# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

az_build_shared_lib_test(azure-data-appconfiguration DATA_APPCONFIGURATION)

project (app_config_sample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(app_config_sample appconfig_basic_operation.cpp)
target_link_libraries(app_config_sample PRIVATE azure-data-appconfiguration azure-identity)
target_include_directories(app_config_sample PRIVATE .)
target_compile_definitions(app_config_sample PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(appconfiguration app_config_sample DISABLE_RUN)
