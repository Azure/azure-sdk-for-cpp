# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.22)

project (azure-messaging LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

message(STATUS "Detecting compiler.")
cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)
cmake_host_system_information(RESULT OS_RELEASE QUERY OS_RELEASE)
cmake_host_system_information(RESULT PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)
cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)

message(STATUS "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}")
message(STATUS "OS Name: ${OS_NAME}")
message(STATUS "OS Release: ${OS_RELEASE}")
message(STATUS "PRETTY_NAME: ${PRETTY_NAME}")
message(STATUS "DISTRO: ${DISTRO}")
message(STATUS "DISTRO_ID"=${DISTRO_ID})
message(STATUS "DISTRO_VERSION_ID"= ${DISTRO_VERSION_ID})

if (CMAKE_CXX_COMPILER MATCHES "clang")
  if($ENV{CXX} MATCHES "-1[12]")
    message(STATUS "Disabling test discovery in EventHubs library for CLANG-11 and CLANG-12 builds because the eventhubs tests segfault for unknown reasons.")
    set(DISABLE_TEST_DISCOVERY ON CACHE BOOL "Disable test discovery for library.")
  endif()
endif()

add_subdirectory(azure-messaging-eventhubs)
add_subdirectory(azure-messaging-eventhubs-checkpointstore-blob)
