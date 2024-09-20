# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(azure-core-amqp-tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(AzureBuildTargetForCI)
include(GoogleTest)

# Unit Tests

IF (USE_UAMQP)
set(UAMQP_ONLY_TESTS   
  transport_tests.cpp
)
ENDIF()

add_executable(azure-core-amqp-tests
  amqp_header_tests.cpp
  amqp_message_tests.cpp
  amqp_properties_tests.cpp
  amqp_value_tests.cpp
  async_operation_queue_tests.cpp
  azure_core_amqp_tests.cpp
  claim_based_security_tests.cpp
  connection_string_tests.cpp
  connection_tests.cpp
  link_tests.cpp
  management_tests.cpp
  message_sender_receiver.cpp
  message_source_target.cpp
  mock_amqp_server.hpp
  session_tests.cpp
  amqp_performative_tests.cpp
  ${UAMQP_ONLY_TESTS}
)

if (USE_UAMQP)
target_include_directories(azure-core-amqp-tests PRIVATE ../../src/amqp/impl/uamqp/amqp/private)
endif()
if (USE_RUST_AMQP)
target_include_directories(azure-core-amqp-tests PRIVATE ../../src/impl/rust_amqp/rust_amqp/rust_wrapper)
endif()

add_dependencies(azure-core-amqp-tests Azure::azure-core-amqp)

target_compile_definitions(azure-core-amqp-tests PRIVATE _azure_BUILDING_TESTS)

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
  target_compile_options(azure-core-amqp-tests PUBLIC /wd26495 /wd26812 /wd6326 /wd28204 /wd28020 /wd6330 /wd4389)
endif()

if (USE_UAMQP)
  target_compile_definitions(azure-core-amqp-tests PRIVATE ENABLE_UAMQP)
endif()


target_include_directories(azure-core-amqp-tests PRIVATE Azure::azure-core-amqp)
# Disable the localtime is unsafe warning.
target_compile_definitions(azure-core-amqp-tests PRIVATE _CRT_SECURE_NO_WARNINGS)

# Link test executable against gtest & gtest_main
target_link_libraries(azure-core-amqp-tests PRIVATE GTest::gtest_main Azure::azure-core-amqp)

create_per_service_target_build(core azure-core-amqp-tests)
create_map_file(azure-core-amqp-tests azure-core-amqp-tests.map)

# gtest_discover_tests will scan the test from azure-core-amqp-tests and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-core-amqp-tests
     TEST_PREFIX azure-core-amqp.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600)
