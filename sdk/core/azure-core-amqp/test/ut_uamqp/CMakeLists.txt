# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# AMQP Tests which require using the uAMQP library to force certain scenarios.

cmake_minimum_required (VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(azure-core-amqp-tests-uamqp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(AzureBuildTargetForCI)
include(GoogleTest)

#set(USE_INSTALLED_DEPENDENCIES ON CACHE BOOL "Use vcpkg dependencies." FORCE)
#set(skip_samples ON CACHE BOOL "Skip building samples" FORCE)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/azure-uamqp-c ${CMAKE_CURRENT_BINARY_DIR}/../../vendor/azure-uamqp-c SYSTEM)

#find_package(uamqp CONFIG REQUIRED)
find_package(umock_c)
find_package(azure_macro_utils_c CONFIG REQUIRED)
find_package(azure_c_shared_utility CONFIG REQUIRED)


# Unit Tests
add_executable(azure-core-amqp-uamqp-tests
  uamqp_error_tests.cpp
  uamqp_header_tests.cpp
  uamqp_insertion_tests.cpp
  uamqp_message_tests.cpp
  uamqp_properties_tests.cpp
  uamqp_performatives_tests.cpp
  uamqp_value_tests.cpp
)

target_compile_definitions(azure-core-amqp-uamqp-tests PRIVATE _azure_BUILDING_TESTS)

add_dependencies(azure-core-amqp-uamqp-tests Azure::azure-core-amqp uamqp)

# UAMQP Definitions and headers.
target_compile_definitions(azure-core-amqp-uamqp-tests PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(azure-core-amqp-uamqp-tests SYSTEM PRIVATE ${UAMQP_INC_FOLDER})


if (MSVC)
  # Disable warnings:
  # - C26495: Variable
  #             - 'testing::internal::Mutex::critical_section_'
  #             - 'testing::internal::Mutex::critical_section_init_phase_'
  #             - 'testing::internal::Mutex::owner_thread_id_'
  #             - 'testing::internal::Mutex::type_'
  #           is uninitialized. Always initialize member variables (type.6).
  # - C26812: The enum type
  #             - 'testing::internal::Mutex::StaticConstructorSelector'
  #             - 'testing::TestPartResult::Type'
  #           is unscoped. Prefer 'enum class' over 'enum' (Enum.3)
  # - C6326: Google comparisons
  target_compile_options(azure-core-amqp-uamqp-tests PUBLIC /wd26495 /wd26812 /wd6326 /wd28204 /wd28020 /wd6330 /wd4389)
endif()

target_include_directories(azure-core-amqp-uamqp-tests PRIVATE Azure::azure-core-amqp)

# Link test executable against gtest & gtest_main
target_link_libraries(azure-core-amqp-uamqp-tests PRIVATE GTest::gtest_main Azure::azure-core-amqp uamqp umock_c azure_macro_utils_c)

create_per_service_target_build(core azure-core-amqp-uamqp-tests)
create_map_file(azure-core-amqp-uamqp-tests azure-core-amqp-uamqp-tests.map)

# gtest_discover_tests will scan the test from azure-core-amqp-tests and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-core-amqp-uamqp-tests
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
     TEST_PREFIX azure-core-amqp.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600)
