# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# cspell: ignore DISTRO

cmake_minimum_required (VERSION 3.13)

project (azure-messaging LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if (LINUX OR UNIX)
message(STATUS "Detecting Linux distribution.")
  execute_process(COMMAND lsb_release -ir OUTPUT_VARIABLE LINUX_DISTRO_INFO)

  message(STATUS "LINUX_INFO=${LINUX_DISTRO_INFO}")

  if (CMAKE_CXX_COMPILER MATCHES "clang")
    if(LINUX_DISTRO_INFO MATCHES "Ubuntu")
      if (LINUX_DISTRO_INFO MATCHES "20.04")
        message(STATUS "Disabling test discovery in EventHubs library for Ubuntu 20.04 builds because the eventhubs tests segfault for unknown reasons.")
        set(DISABLE_TEST_DISCOVERY ON CACHE BOOL "Disable test discovery for library.")
      endif()
    endif()
  endif()
endif()


add_subdirectory(azure-messaging-eventhubs)
add_subdirectory(azure-messaging-eventhubs-checkpointstore-blob)
