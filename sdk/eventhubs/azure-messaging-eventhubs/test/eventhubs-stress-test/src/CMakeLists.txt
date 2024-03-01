# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# cspell: words otlp

cmake_minimum_required(VERSION 3.13)

project(azure-messaging-eventhubs-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC_USE_STATIC_CRT AND MSVC)
  # 1. More about static/shared CRT:
  # https://docs.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160
  #
  # 2. MSVC_USE_STATIC_CRT build flag approach is used/inspired by libcurl
  # (https://github.com/curl/curl/blob/master/CMakeLists.txt) and some other projects.
  #
  # 3. GTest would emit the following warning:
  # warning LNK4098: defaultlib 'LIBCMTD' conflicts with use of other libs; use /NODEFAULTLIB:library
  # AddGoogleTest.cmake uses gtest_force_shared_crt
  # (see https://github.com/google/googletest/blob/master/googletest/README.md),
  # which respects linker settings that we set below, and our settings below are all in sync.
  #
  # 4. Sometimes, the following approach is recommended instead:
  # +-----------------------------------------------------------------------------------+
  # | # Use the static runtime libraries when building statically                       |
  # | # for consistency with the vcpkg arch-windows-static triplets:                    |
  # | cmake_policy(SET CMP0091 NEW)                                                     |
  # | # see https://cmake.org/cmake/help/v3.15/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html |
  # | if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)                                       |
  # |   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")         |
  # | endif()                                                                           |
  # +-----------------------------------------------------------------------------------+
  # However, it only works when cmake installed is 3.15+;
  # we have to require a minimum of 3.13.
  #
  # 5. We "replace with empty string" (i.e. remove) first, then add, so that '/MT'
  # will be present (and present once) even if '/MD' was not.

  message(STATUS "Configuring Static Runtime Library.")
  if(${CMAKE_CXX_FLAGS} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  endif()

  if(${CMAKE_CXX_FLAGS_RELEASE} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  endif()

  if(${CMAKE_CXX_FLAGS_RELWITHDEBINFO} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
  endif()

  if(${CMAKE_CXX_FLAGS_MINSIZEREL} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
  endif()
  
  if(${CMAKE_CXX_FLAGS_DEBUG} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  endif()

  # Now make the same adjustments for .C files as was done for .CPP files.
  if(${CMAKE_C_FLAGS} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
  endif()
  
  if(${CMAKE_C_FLAGS_RELEASE} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  endif()

  if(${CMAKE_C_FLAGS_RELWITHDEBINFO} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT")
  endif()

  if(${CMAKE_C_FLAGS_MINSIZEREL} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
  endif()
  
  if(${CMAKE_C_FLAGS_DEBUG} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MDd" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  endif()
endif()

set(WITH_STL ON)

find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(protobuf)
find_package(nlohmann_json)
find_package(CURL)

set(INCLUDE_FILES 
  inc/eventhubs_stress_scenarios.hpp
  scenarios/inc/event_sender.hpp
  scenarios/inc/opentelemetry_helpers.hpp
  scenarios/inc/batch_stress_tests.hpp
  )


set(SOURCE_FILES
  eventhubs_stress_test.cpp
  scenarios/src/opentelemetry_helpers.cpp
  scenarios/src/batch_stress_tests.cpp
  )


add_executable(
  azure-messaging-eventhubs-stress-test
  ${SOURCE_FILES} ${INCLUDE_FILES}
)

# Include the headers from the project.
target_include_directories(
  azure-messaging-eventhubs-stress-test
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/scenarios/inc>
)

target_link_libraries(azure-messaging-eventhubs-stress-test 
PRIVATE
azure-messaging-eventhubs
azure-identity
opentelemetry-cpp::ostream_span_exporter
opentelemetry-cpp::in_memory_span_exporter
opentelemetry-cpp::otlp_http_exporter
opentelemetry-cpp::otlp_http_log_record_exporter
opentelemetry-cpp::sdk
)

create_map_file(azure-messaging-eventhubs-stress-test azure-messaging-eventhubs-stress-test.map)
file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
