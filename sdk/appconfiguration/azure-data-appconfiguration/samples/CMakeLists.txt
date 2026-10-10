# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (app_config_sample LANGUAGES CXX)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(app_config_sample appconfig_basic_operation.cpp)
target_compile_features(app_config_sample PRIVATE cxx_std_14)
set_target_properties(app_config_sample PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(app_config_sample PRIVATE azure-data-appconfiguration azure-identity)
target_include_directories(app_config_sample PRIVATE .)
target_compile_definitions(app_config_sample PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(appconfiguration app_config_sample DISABLE_RUN)
