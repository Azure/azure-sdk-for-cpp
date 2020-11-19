# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set(AZ_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

macro(az_vcpkg_integrate)
  # VCPKG Integration
  if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
  elseif(DEFINED ENV{VCPKG_INSTALLATION_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
  endif()
  if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
  endif()
endmacro()

macro(az_vcpkg_export)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/CONTROL" "${CMAKE_BINARY_DIR}/vcpkg/ports/${TARGET_NAME}-cpp/CONTROL" @ONLY)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/portfile.cmake" "${CMAKE_BINARY_DIR}/vcpkg/ports/${TARGET_NAME}-cpp/portfile.cmake" @ONLY)

  include(GNUInstallDirs)

  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/inc/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES "${AZ_ROOT_DIR}/LICENSE.txt" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}-cpp" RENAME "copyright")

  install(
    TARGETS "${TARGET_NAME}"
      EXPORT "${TARGET_NAME}-cppTargets"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

  install(
    EXPORT "${TARGET_NAME}-cppTargets"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}-cpp"
      NAMESPACE Azure::
      FILE "${TARGET_NAME}-cppTargets.cmake"
    )

  include(CMakePackageConfigHelpers)

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/Config.cmake.in"
    "${TARGET_NAME}-cppConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}-cpp"
    PATH_VARS
      CMAKE_INSTALL_LIBDIR)

  write_basic_package_version_file(
    "${TARGET_NAME}-cppConfigVersion.cmake"
      VERSION ${AZ_LIBRARY_VERSION}
      COMPATIBILITY SameMajorVersion
    )

  install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-cppConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-cppConfigVersion.cmake"
      DESTINATION
        "${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}-cpp"
    )

  export(PACKAGE "${TARGET_NAME}-cpp")
endmacro()
