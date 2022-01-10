#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Azure::azure-security-attestation" for configuration "Debug"
set_property(TARGET Azure::azure-security-attestation APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Azure::azure-security-attestation PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/azure-security-attestation.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS Azure::azure-security-attestation )
list(APPEND _IMPORT_CHECK_FILES_FOR_Azure::azure-security-attestation "${_IMPORT_PREFIX}/lib/azure-security-attestation.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
