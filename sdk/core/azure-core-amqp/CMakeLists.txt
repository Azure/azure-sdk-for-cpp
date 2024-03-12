# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-core-amqp LANGUAGES CXX)

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

find_package(Threads REQUIRED)
if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(VENDOR_UAMQP ON CACHE BOOL "Use vendored uamqp" FORCE)
if (VENDOR_UAMQP)
  message(STATUS "Using vendored uamqp")
  set(use_installed_dependencies ON CACHE BOOL "Use vcpkg dependencies." FORCE)
  set(skip_samples ON CACHE BOOL "Skip building samples" FORCE)
  set(build_as_object_library ON CACHE BOOL "Produce object library" FORCE)
  set(atomic_refcount ON CACHE BOOL "Use atomic refcount" FORCE)

  add_subdirectory(vendor/azure-uamqp-c SYSTEM)

  # uAMQP specific compiler settings. Primarily used to disable warnings in the uAMQP codebase.
  if (MSVC)
    target_compile_definitions(uamqp PRIVATE _CRT_SECURE_NO_WARNINGS)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(uamqp PUBLIC -Wno-extra-semi -Wno-gnu-zero-variadic-macro-arguments -Wno-cast-qual -Wno-format-pedantic -Wno-c11-extensions)
  else()
    target_compile_options(uamqp PUBLIC -Wno-pedantic -Wno-implicit-fallthrough -Wno-strict-aliasing)
  endif()
else()
  message(STATUS "Using vcpkg uamqp")
  find_package(uamqp CONFIG REQUIRED)
endif()

find_package(umock_c CONFIG REQUIRED)
find_package(azure_macro_utils_c CONFIG REQUIRED)
find_package(azure_c_shared_utility CONFIG REQUIRED)


set (AZURE_CORE_AMQP_HEADER
    inc/azure/core/amqp.hpp
    inc/azure/core/amqp/dll_import_export.hpp
    inc/azure/core/amqp/internal/amqp_settle_mode.hpp
    inc/azure/core/amqp/internal/cancellable.hpp
    inc/azure/core/amqp/internal/claims_based_security.hpp
    inc/azure/core/amqp/internal/common/async_operation_queue.hpp
    inc/azure/core/amqp/internal/common/completion_operation.hpp
    inc/azure/core/amqp/internal/common/global_state.hpp
    inc/azure/core/amqp/internal/connection.hpp
    inc/azure/core/amqp/internal/connection_string_credential.hpp
    inc/azure/core/amqp/internal/doxygen_pragma.hpp
    inc/azure/core/amqp/internal/endpoint.hpp
    inc/azure/core/amqp/internal/link.hpp
    inc/azure/core/amqp/internal/management.hpp
    inc/azure/core/amqp/internal/message_receiver.hpp
    inc/azure/core/amqp/internal/message_sender.hpp
    inc/azure/core/amqp/internal/models/amqp_error.hpp
    inc/azure/core/amqp/internal/models/amqp_protocol.hpp
    inc/azure/core/amqp/internal/models/message_source.hpp
    inc/azure/core/amqp/internal/models/message_target.hpp
    inc/azure/core/amqp/internal/models/messaging_values.hpp
    inc/azure/core/amqp/internal/models/performatives/amqp_detach.hpp
    inc/azure/core/amqp/internal/models/performatives/amqp_transfer.hpp
    inc/azure/core/amqp/internal/network/amqp_header_detect_transport.hpp
    inc/azure/core/amqp/internal/network/sasl_transport.hpp
    inc/azure/core/amqp/internal/network/socket_listener.hpp
    inc/azure/core/amqp/internal/network/socket_transport.hpp
    inc/azure/core/amqp/internal/network/tls_transport.hpp
    inc/azure/core/amqp/internal/network/transport.hpp
    inc/azure/core/amqp/internal/session.hpp
    inc/azure/core/amqp/models/amqp_header.hpp
    inc/azure/core/amqp/models/amqp_message.hpp
    inc/azure/core/amqp/models/amqp_properties.hpp
    inc/azure/core/amqp/models/amqp_value.hpp
    inc/azure/core/amqp/rtti.hpp
)

set(AZURE_CORE_AMQP_SOURCE
    src/amqp/cancellable.cpp
    src/amqp/claim_based_security.cpp
    src/amqp/connection.cpp
    src/amqp/connection_string_credential.cpp
    src/amqp/link.cpp
    src/amqp/management.cpp
    src/amqp/message_receiver.cpp
    src/amqp/message_sender.cpp
    src/amqp/private/claims_based_security_impl.hpp
    src/amqp/private/connection_impl.hpp
    src/amqp/private/link_impl.hpp
    src/amqp/private/management_impl.hpp
    src/amqp/private/message_receiver_impl.hpp
    src/amqp/private/message_sender_impl.hpp
    src/amqp/private/session_impl.hpp
    src/amqp/private/unique_handle.hpp
    src/amqp/session.cpp
    src/common/global_state.cpp
    src/models/amqp_detach.cpp
    src/models/amqp_error.cpp
    src/models/amqp_header.cpp
    src/models/amqp_message.cpp
    src/models/amqp_properties.cpp
    src/models/amqp_transfer.cpp
    src/models/amqp_value.cpp
    src/models/message_source.cpp
    src/models/message_target.cpp
    src/models/messaging_values.cpp
    src/models/private/error_impl.hpp
    src/models/private/header_impl.hpp
    src/models/private/message_impl.hpp
    src/models/private/performatives/detach_impl.hpp
    src/models/private/performatives/transfer_impl.hpp
    src/models/private/properties_impl.hpp
    src/models/private/source_impl.hpp
    src/models/private/target_impl.hpp
    src/models/private/value_impl.hpp
    src/network/amqp_header_transport.cpp
    src/network/private/transport_impl.hpp
    src/network/private/transport_impl.hpp
    src/network/sasl_transport.cpp
    src/network/socket_listener.cpp
    src/network/socket_transport.cpp
    src/network/tls_transport.cpp
    src/network/transport.cpp
    src/private/package_version.hpp
)


add_library(azure-core-amqp ${AZURE_CORE_AMQP_SOURCE} ${AZURE_CORE_AMQP_HEADER} $<TARGET_OBJECTS:uamqp>)

if (VENDOR_UAMQP)
target_include_directories(azure-core-amqp SYSTEM PRIVATE ${UAMQP_INC_FOLDER})
endif()

target_include_directories(
  azure-core-amqp
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

		
# make sure that users can consume the project as a library.
add_library(Azure::azure-core-amqp ALIAS azure-core-amqp)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core azure-core-amqp azure-core-amqp-tests "tests?/*;samples?/*")
create_code_coverage(core azure-core-amqp-uamqp azure-core-amqp-uamqp-tests "tests?/*;samples?/*")

# cspell:words aziotsharedutil
# uAMQP's headers require the manual addition of umock_c, azure_macro_utils_c, and aziotsharedutil to the target link libraries. 
target_link_libraries(azure-core-amqp PRIVATE
    umock_c
    azure_macro_utils_c
    aziotsharedutil
    PUBLIC Azure::azure-core)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-core-amqp ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
  azure-core-amqp
  CORE_AMQP
  azure/core/amqp/dll_import_export.hpp
)

az_rtti_setup(
  azure-core-amqp
  CORE_AMQP
  azure/core/amqp/rtti.hpp
)

if(BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(_azure_TESTING_BUILD)
  
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test)
endif()

if(BUILD_SAMPLES)
  add_compile_definitions(SAMPLES_BUILD)
  add_subdirectory (samples)
endif()
