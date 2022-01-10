#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Azure::azure-security-attestation" for configuration "RelWithDebInfo"
set_property(TARGET Azure::azure-security-attestation APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Azure::azure-security-attestation PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/azure-security-attestation.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS Azure::azure-security-attestation )
list(APPEND _IMPORT_CHECK_FILES_FOR_Azure::azure-security-attestation "${_IMPORT_PREFIX}/lib/azure-security-attestation.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
