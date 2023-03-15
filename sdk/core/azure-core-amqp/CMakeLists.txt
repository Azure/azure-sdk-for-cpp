# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

# Azure core is compatible with CMake 3.12
cmake_minimum_required (VERSION 3.12)
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

find_package(uamqp CONFIG REQUIRED)
find_package(umock_c)
find_package(azure_macro_utils_c CONFIG REQUIRED)
find_package(azure_c_shared_utility CONFIG REQUIRED)

set (AZURE_CORE_AMQP_HEADER
    inc/azure/core/amqp/cancellable.hpp
    inc/azure/core/amqp/claims_based_security.hpp
    inc/azure/core/amqp/common/async_operation_queue.hpp
    inc/azure/core/amqp/common/completion_operation.hpp
    inc/azure/core/amqp/common/global_state.hpp
    inc/azure/core/amqp/connection.hpp
    inc/azure/core/amqp/connection_string_credential.hpp
    inc/azure/core/amqp/link.hpp
    inc/azure/core/amqp/message_receiver.hpp
    inc/azure/core/amqp/message_sender.hpp
    inc/azure/core/amqp/models/amqp_header.hpp
    inc/azure/core/amqp/models/amqp_message.hpp
    inc/azure/core/amqp/models/amqp_properties.hpp
    inc/azure/core/amqp/models/amqp_value.hpp
    inc/azure/core/amqp/models/message_source.hpp
    inc/azure/core/amqp/models/message_target.hpp
    inc/azure/core/amqp/models/messaging_values.hpp
    inc/azure/core/amqp/models/transfer_instance.hpp
    inc/azure/core/amqp/session.hpp
)

set(AZURE_CORE_AMQP_SOURCE
	src/amqp/cancellable.cpp
	src/amqp/claim_based_security.cpp
	src/amqp/connection.cpp
	src/amqp/connection_string_credential.cpp
	src/amqp/link.cpp
	src/amqp/message_receiver.cpp
	src/amqp/message_sender.cpp
	src/amqp/session.cpp
	src/common/global_state.cpp
	src/models/amqp_header.cpp
	src/models/amqp_message.cpp
	src/models/amqp_properties.cpp
	src/models/amqp_value.cpp
	src/models/message_source.cpp
	src/models/message_target.cpp
	src/models/messaging_values.cpp
	src/models/TransferInstance.cpp
	src/network/amqp_header_transport.cpp
	src/network/sasl_transport.cpp
	src/network/socket_listener.cpp
	src/network/socket_transport.cpp
	src/network/tls_transport.cpp
	src/network/transport.cpp 
)

add_library(azure-core-amqp ${AZURE_CORE_AMQP_SOURCE} ${AZURE_CORE_AMQP_HEADER})

target_include_directories(
  azure-core-amqp
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)
		
# make sure that users can consume the project as a library.
add_library(Azure::azure-core-amqp ALIAS azure-core-amqp)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(amqp azure-core-amqp azure-core-amqp-tests "tests?/*;samples?/*")

target_link_libraries(azure-core-amqp PRIVATE uamqp umock_c azure_macro_utils_c PUBLIC Azure::azure-core)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET azure-core-amqp PROPERTY CXX_STANDARD 14)
endif()


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