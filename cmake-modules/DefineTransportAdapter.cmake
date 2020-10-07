## Copyright (c) Microsoft Corporation. All rights reserved.
## SPDX-License-Identifier: MIT

#############                      TRANSPORT ADAPTER BUILD                     #####################
#  Default: If no option is explicitly added, curl will be used for POSIX and WIN HTTP for WIndows #
#  Windows: Both CURL and WIN_HTTP can be build to be used.                                        #
#  POSIX: Only CURL is acceptable. If WIN_HTTP is set, generate step will fail for user            #

#  Defines `BUILD_WIN_HTTP_TRANSPORT_ADAPTER` and `BUILD_CURL_HTTP_TRANSPORT_ADAPTER` for source code

if (WIN32 OR MINGW OR MSYS OR CYGWIN)
  if(BUILD_CURL_TRANSPORT)
    add_compile_definitions(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  endif()
  # If explicitly adding WIN_HTTP or did not add CURL
  if(BUILD_WIN_HTTP_TRANSPORT OR NOT BUILD_CURL_TRANSPORT)
    add_compile_definitions(BUILD_WIN_HTTP_TRANSPORT_ADAPTER)
    SET(BUILD_WIN_HTTP_TRANSPORT ON)
  endif()
elseif(UNIX)
    if(BUILD_WIN_HTTP_TRANSPORT)
        message(FATAL_ERROR "Win HTTP transport adapter is not supported for Unix platforms")
    endif()
    # regardless if CURL transport is set or not, it is always added for UNIX as default
    add_compile_definitions(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    SET(BUILD_CURL_TRANSPORT ON)
else()
    message(FATAL_ERROR "Unsupported platform")
endif ()
