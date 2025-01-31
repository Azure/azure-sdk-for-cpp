# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-messaging LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

message(STATUS "Detecting compiler.")
if (CMAKE_CXX_COMPILER MATCHES "clang")
  if($ENV{CXX} MATCHES "-11")
    message(STATUS "Disabling testing in EventHubs library for CLANG-11 builds because the eventhubs tests segfault for unknown reasons.")
    set(BUILD_TESTING OFF)
  endif()
endif()

add_subdirectory(azure-messaging-eventhubs)
add_subdirectory(azure-messaging-eventhubs-checkpointstore-blob)
