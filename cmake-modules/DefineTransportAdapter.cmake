## Copyright (c) Microsoft Corporation. All rights reserved.
## SPDX-License-Identifier: MIT

#############                      TRANSPORT ADAPTER BUILD                     #####################
#  Default: If no option is explicitly added, curl will be used for POSIX and WIN HTTP for Windows #
#  Windows: Both CURL and WIN_HTTP can be built to be used.                                        #
#  POSIX: Only CURL is acceptable. If WIN_HTTP is set, generate step will fail for user            #

#  Defines `BUILD_TRANSPORT_WINHTTP_ADAPTER` and `BUILD_CURL_HTTP_TRANSPORT_ADAPTER` for source code

if (WIN32 OR MINGW OR MSYS OR CYGWIN)
  add_compile_definitions(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  add_compile_definitions(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  SET(BUILD_TRANSPORT_CURL ON)
  SET(BUILD_TRANSPORT_WINHTTP ON)
elseif(UNIX)
    if(BUILD_TRANSPORT_WINHTTP)
        message(FATAL_ERROR "Win HTTP transport adapter is not supported for Unix platforms")
    endif()
    # regardless if CURL transport is set or not, it is always added for UNIX as default
    add_compile_definitions(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    SET(BUILD_TRANSPORT_CURL ON)
else()
    message(FATAL_ERROR "Unsupported platform")
endif ()
