# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# cspell: words rustup ncrypt secur

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

set(USE_RUST_AMQP ON CACHE BOOL "Enable Rust AMQP stack" FORCE)

if (DISABLE_RUST_IN_BUILD)
  message(STATUS "Disabling Rust based functionality in build.")
  set(USE_RUST_AMQP OFF CACHE BOOL "Enable Rust AMQP stack" FORCE)
endif()


if (USE_RUST_AMQP)
  message(STATUS "Using RUST based AMQP stack")
  set(VENDOR_UAMQP OFF CACHE BOOL "Use vendored uAMQP" FORCE)
  set(USE_UAMQP OFF CACHE BOOL "Enable uAMQP AMQP stack" FORCE)
else()
  set(USE_UAMQP ON CACHE BOOL "Enable uAMQP AMQP stack" FORCE)
  set(VENDOR_UAMQP ON CACHE BOOL "Use vendored uamqp" FORCE)
endif()

if (USE_UAMQP)
  message(STATUS "Using vendored uamqp")
  set(use_installed_dependencies ON CACHE BOOL "Use vcpkg dependencies." FORCE)
  set(skip_samples ON CACHE BOOL "Skip building samples" FORCE)
  set(build_as_object_library ON CACHE BOOL "Produce object library" FORCE)
  set(atomic_refcount ON CACHE BOOL "Use atomic refcount" FORCE)

  add_subdirectory(src/impl/uamqp/vendor/azure-uamqp-c SYSTEM)

  # uAMQP specific compiler settings. Primarily used to disable warnings in the uAMQP codebase.
  if (MSVC)
    target_compile_definitions(uamqp PRIVATE _CRT_SECURE_NO_WARNINGS)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(uamqp PUBLIC -Wno-extra-semi -Wno-gnu-zero-variadic-macro-arguments -Wno-cast-qual -Wno-format-pedantic -Wno-c11-extensions)
  else()
    target_compile_options(uamqp PUBLIC -Wno-pedantic -Wno-implicit-fallthrough -Wno-strict-aliasing)
  endif()

  find_package(umock_c CONFIG REQUIRED)
  find_package(azure_macro_utils_c CONFIG REQUIRED)
  find_package(azure_c_shared_utility CONFIG REQUIRED)
endif()

if (USE_RUST_AMQP)

#  include (${CMAKE_SOURCE_DIR}/cmake-modules/acquire_rust.cmake)
#  install_rustup()

  include(FetchContent)

    # If the cmake build is x86, add the x86 target to the rust build
    message("VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
  if (${VCPKG_TARGET_TRIPLET} MATCHES "x86.*")
    message("Configure rust for x86 builds.")

    if (${VCPKG_TARGET_TRIPLET} MATCHES ".*-windows.*")
      message("Configure rust for Windows x86 builds.")

      # Run the rustup command to install the toolchain
      execute_process(COMMAND rustup toolchain install stable-i686-pc-windows-msvc)

      set(Rust_TOOLCHAIN "stable-i686-pc-windows-msvc")
      set(Rust_CARGO_TARGET "i686-pc-windows-msvc")
    else()
      message("Configure rust for Linux x86 builds.")
      execute_process(COMMAND rustup toolchain install stable-i686-unknown-linux-gnu)
      set(Rust_CARGO_TARGET "i686-unknown-linux-gnu")
    endif()
  else()
    message("Configure rust for 64 builds.")
  endif()


  FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.5.1 # Optionally specify a commit hash, version tag or branch here
  )
  FetchContent_MakeAvailable(Corrosion)

if (NOT APPLE)
  if(LINUX OR UNIX)
    message(STATUS "Adding OpenSSL to target link libraries")
    include(FindOpenSSL)

    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
      if (${_variableName} MATCHES "OPENSSL")
        message(STATUS "Found variable: ${_variableName} = ${${_variableName}}")
      endif()
    endforeach()

    message(STATUS "OPENSSL_FOUND: ${OPENSSL_FOUND}")
    message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
    message(STATUS "OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")
    message(STATUS "OPENSSL_SSL_LIBRARY: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "OPENSSL_DIR: ${OPENSSL_DIR}")
    message(STATUS "OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")
    message(STATUS "OPENSSL_VERSION: ${OPENSSL_VERSION}")
    message(STATUS "OPENSSL_VERSION_STRING: ${OPENSSL_VERSION_STRING}")
  endif()
endif()


 
  # Import targets defined in a package or workspace manifest `Cargo.toml` file
  corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/rust_amqp/rust_amqp/rust_wrapper/Cargo.toml)

  # Set the build environment variables for the rust_wrapper crate to use our targetted OpenSSL version.
  if(NOT APPLE)
    if (LINUX OR UNIX)
      # corrosion_set_env_vars(rust_wrapper
      #   OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
      #   OPENSSL_LIBS=${OPENSSL_LIBRARIES}
      # )
      corrosion_set_env_vars(rust_wrapper
        OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
        OPENSSL_DIR=${OPENSSL_INCLUDE_DIR}/..
      )
      endif()
  endif()
endif()


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
    src/amqp/claim_based_security.cpp
    src/amqp/connection.cpp
    src/amqp/connection_string_credential.cpp
    src/amqp/management.cpp
    src/amqp/message_receiver.cpp
    src/amqp/message_sender.cpp
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
    src/private/package_version.hpp
)

if (USE_UAMQP)
set(AZURE_UAMQP_SOURCE
  src/amqp/link.cpp
  inc/azure/core/amqp/internal/link.hpp

  src/impl/uamqp/amqp/cancellable.cpp
  src/impl/uamqp/amqp/claim_based_security.cpp
  src/impl/uamqp/amqp/connection.cpp
  src/impl/uamqp/amqp/connection_string_credential.cpp
  src/impl/uamqp/amqp/link.cpp
  src/impl/uamqp/amqp/management.cpp
  src/impl/uamqp/amqp/message_receiver.cpp
  src/impl/uamqp/amqp/message_sender.cpp
  src/impl/uamqp/amqp/session.cpp
  src/impl/uamqp/amqp/private/claims_based_security_impl.hpp
  src/impl/uamqp/amqp/private/connection_impl.hpp
  src/impl/uamqp/amqp/private/link_impl.hpp
  src/impl/uamqp/amqp/private/management_impl.hpp
  src/impl/uamqp/amqp/private/message_receiver_impl.hpp
  src/impl/uamqp/amqp/private/message_sender_impl.hpp
  src/impl/uamqp/amqp/private/session_impl.hpp
  src/impl/uamqp/network/amqp_header_transport.cpp
  src/impl/uamqp/network/sasl_transport.cpp
  src/impl/uamqp/network/socket_listener.cpp
  src/impl/uamqp/network/socket_transport.cpp
  src/impl/uamqp/network/tls_transport.cpp
  src/impl/uamqp/network/transport.cpp
  src/impl/uamqp/network/private/transport_impl.hpp
)
endif()

if(USE_RUST_AMQP)
set(AZURE_RUST_AMQP_SOURCE
  src/impl/rust_amqp/amqp/claim_based_security.cpp
  src/impl/rust_amqp/amqp/connection.cpp
  src/impl/rust_amqp/amqp/connection_string_credential.cpp
  src/impl/rust_amqp/amqp/management.cpp
  src/impl/rust_amqp/amqp/message_receiver.cpp
  src/impl/rust_amqp/amqp/message_sender.cpp
  src/impl/rust_amqp/amqp/session.cpp
  src/impl/rust_amqp/amqp/private/claims_based_security_impl.hpp
  src/impl/rust_amqp/amqp/private/connection_impl.hpp
  src/impl/rust_amqp/amqp/private/management_impl.hpp
  src/impl/rust_amqp/amqp/private/message_receiver_impl.hpp
  src/impl/rust_amqp/amqp/private/message_sender_impl.hpp
  src/impl/rust_amqp/amqp/private/session_impl.hpp
)

endif()

if (USE_UAMQP)
add_library(azure-core-amqp ${AZURE_CORE_AMQP_SOURCE} ${AZURE_CORE_AMQP_HEADER} ${AZURE_UAMQP_SOURCE} $<TARGET_OBJECTS:uamqp>)
elseif(USE_RUST_AMQP)
add_library(azure-core-amqp ${AZURE_CORE_AMQP_SOURCE} ${AZURE_CORE_AMQP_HEADER} ${AZURE_RUST_AMQP_SOURCE})
endif()



if (USE_UAMQP)
target_include_directories(azure-core-amqp SYSTEM PRIVATE ${UAMQP_INC_FOLDER})
target_include_directories(azure-core-amqp PRIVATE src/impl/uamqp/amqp/private src/impl/uamqp/amqp/network ${UAMQP_INC_FOLDER})
endif()

if (USE_RUST_AMQP)
target_include_directories(azure-core-amqp PRIVATE src/impl/rust_amqp/amqp/private src/impl/rust_amqp/rust_amqp/rust_wrapper)
endif()

target_include_directories(
  azure-core-amqp
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

# make sure that users can consume the project as a library.
add_library(Azure::azure-core-amqp ALIAS azure-core-amqp)

if(USE_UAMQP)
#  target_compile_definitions(azure-core-amqp PRIVATE ENABLE_UAMQP)
  add_compile_definitions(ENABLE_UAMQP)
elseif(USE_RUST_AMQP)
#  target_compile_definitions(azure-core-amqp PRIVATE ENABLE_RUST_AMQP)
  add_compile_definitions(ENABLE_RUST_AMQP)
endif()

target_compile_definitions(azure-core-amqp PRIVATE _azure_BUILDING_SDK)
		
# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core azure-core-amqp azure-core-amqp-tests "tests?/*;samples?/*")
create_code_coverage(core azure-core-amqp-uamqp azure-core-amqp-uamqp-tests "tests?/*;samples?/*")

if (USE_UAMQP)
# cspell:words aziotsharedutil
# uAMQP's headers require the manual addition of umock_c, azure_macro_utils_c, and aziotsharedutil to the target link libraries. 
  target_link_libraries(azure-core-amqp PRIVATE
    umock_c
    azure_macro_utils_c
    aziotsharedutil
    PUBLIC Azure::azure-core)
elseif(USE_RUST_AMQP)
  target_link_libraries(azure-core-amqp 
    PRIVATE rust_wrapper-static
    PUBLIC Azure::azure-core)

  if (MSVC)
    message(STATUS "Assuming Windows crypto libraries")
    target_link_libraries(azure-core-amqp PRIVATE Secur32 ncrypt crypt32 RuntimeObject)
  elseif (APPLE)
    message(STATUS "Assuming Security framework on macOS")
    target_link_libraries(azure-core-amqp PRIVATE "-framework Security")
  elseif(LINUX OR UNIX)
    message(STATUS "Assuming Linux crypto libraries.")
    target_link_libraries(azure-core-amqp PRIVATE ${OPENSSL_LIBRARIES})
  else()
    message(STATUS "Unknown operating system, assuming Linux based operating system.")
#    target_link_libraries(azure-core-amqp PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  endif()
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-core-amqp PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
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
