# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (keyvault-secrets-test-app LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  keyvault-secrets-test-app
    test_app.cpp
    macro_guard.cpp
 )

 if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  # cspell:words VCRUNTIME
  set(AZ_UWP_VCRUNTIME_PATH "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC/14.29.30133/onecore/debug_nonredist/x86/Microsoft.VC142.DebugCRT/VCRUNTIME140D.dll")
  set(AZ_UWP_BUILD_CONFIG "Debug")
  if (EXISTS "${VCRUNTIME_PATH}")
    get_filename_component(AZ_UWP_VCRUNTIME_FILENAME "${VCRUNTIME_PATH}" NAME_WE)
    add_custom_command(
        TARGET azure-core-global-context-test
          POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E "copy_if_different \"${AZ_UWP_VCRUNTIME_PATH}\" ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/"
            COMMAND ${CMAKE_COMMAND} -E "rename \"${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}.dll ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}_APP.dll"
    )
    unset(AZ_UWP_VCRUNTIME_FILENAME)
  endif()
endif()

create_per_service_target_build(keyvault-secrets-test-app keyvault)

create_map_file(keyvault-secrets-test-app keyvault-secrets-test-app.map)

if (MSVC)
  target_compile_options(keyvault-secrets-test-app PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(keyvault-secrets-test-app PRIVATE azure-security-keyvault-secrets azure-identity)
