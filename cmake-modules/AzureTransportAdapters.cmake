## Copyright (c) Microsoft Corporation.
## Licensed under the MIT License.

##############                      TRANSPORT ADAPTER BUILD                     ######################
#  Default: If no option is explicitly added, libcurl will be used for POSIX and WinHTTP for Windows #
#  Windows: Both CURL and WINHTTP can be built to be used.                                           #
#  POSIX: Only CURL is acceptable. If WINHTTP is set, generate step will fail for user               #

if (BUILD_TRANSPORT_CUSTOM)
  message("Using the user-defined transport adapter. Make sure `AzureSdkGetCustomHttpTransport` is implemented and linked.")
  add_compile_definitions(BUILD_TRANSPORT_CUSTOM_ADAPTER)
endif()

#  Defines `BUILD_TRANSPORT_WINHTTP_ADAPTER` and `BUILD_CURL_HTTP_TRANSPORT_ADAPTER` for source code

# On Windows: Make sure to build WinHTTP either if it was user-requested or no transport was selected at all.
# On POSIX: Make sure to build Curl either if it was user-requested or no transport was selected at all.
if (NO_AUTOMATIC_TRANSPORT_BUILD)
  message("Automatic transport build option detection is disabled.")
endif()

if (WIN32 OR MINGW OR MSYS OR CYGWIN)
  if (BUILD_TRANSPORT_CURL)
    # Specified by user on CMake input Libcurl
    add_compile_definitions(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  endif()
  if (BUILD_TRANSPORT_WINHTTP OR (NOT BUILD_TRANSPORT_CURL AND NOT BUILD_TRANSPORT_CUSTOM))
    # WinHTTP selected by user on CMake input 
    # OR Nothing selected by CMake input (not libcurl or custom). Then set default for Windows.
    
    if (NOT BUILD_TRANSPORT_WINHTTP AND NOT BUILD_TRANSPORT_CUSTOM AND NOT NO_AUTOMATIC_TRANSPORT_BUILD)
      # No custom and No winHTTP. 
      message("No transport adapter was selected, using WinHTTP as the default option for Windows.")
    endif()
    
    if (BUILD_TRANSPORT_WINHTTP OR NOT NO_AUTOMATIC_TRANSPORT_BUILD)
      add_compile_definitions(BUILD_TRANSPORT_WINHTTP_ADAPTER)
    endif()
    
    if (NOT BUILD_TRANSPORT_WINHTTP AND NOT NO_AUTOMATIC_TRANSPORT_BUILD)
      # When user did not provide the input option, we need to turn it ON as it is used to include the src code
      SET(BUILD_TRANSPORT_WINHTTP ON)
    endif()

  endif()
elseif (UNIX)
  if (BUILD_TRANSPORT_WINHTTP)
    message(FATAL_ERROR "WinHTTP transport adapter is not supported for POSIX platforms.")
  endif()

  if (BUILD_TRANSPORT_CURL OR (NOT BUILD_TRANSPORT_CURL AND NOT BUILD_TRANSPORT_CUSTOM))

    if(NOT BUILD_TRANSPORT_CURL AND NOT NO_AUTOMATIC_TRANSPORT_BUILD)
      message("No transport adapter was selected, using libcurl as the default option for POSIX.")
    endif()

    if (BUILD_TRANSPORT_CURL OR NOT NO_AUTOMATIC_TRANSPORT_BUILD)
      add_compile_definitions(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
      SET(BUILD_TRANSPORT_CURL ON)
    endif()
  endif()

else()
  message(FATAL_ERROR "Unsupported platform.")
endif()
