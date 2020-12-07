# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# We need to know an absolute path to our repo root to do things like referencing ./LICENSE.txt file.
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

macro(az_vcpkg_export targetName)
  # Produce the files to help with the VcPkg release.
  # Go to the /out/build/<cfg>/vcpkg directory, and copy (merge) "ports" folder to the vcpkg repo.
  # Then, update portfile.cmake's SHA512 from "1" to the actual hash (a good way to do it is to uninstall a package,
  # clean vcpkg/downloads, vcpkg/buildtrees, run "vcpkg install <pkg>", and get the SHA from the error message).
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/CONTROL" "${CMAKE_BINARY_DIR}/vcpkg/ports/${targetName}-cpp/CONTROL" @ONLY)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/portfile.cmake" "${CMAKE_BINARY_DIR}/vcpkg/ports/${targetName}-cpp/portfile.cmake" @ONLY)

  # Standard names for folders such as "bin", "lib", "include". We could hardcode, but some other libs use it too (curl).
  include(GNUInstallDirs)

  # When installing, copy our "inc" directory (headers) to "include" directory at the install location.
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/inc/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  # Copy license as "copyright" (vcpkg dictates naming and location).
  install(FILES "${AZ_ROOT_DIR}/LICENSE.txt" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${targetName}-cpp" RENAME "copyright")

  # Indicate where to install targets. Mirrors what other ports do.
  install(
    TARGETS "${targetName}"
      EXPORT "${targetName}-cppTargets"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # DLLs (if produced by build) go to "/bin"
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # static .lib files
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # .lib files for DLL build
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # headers
    )

  # Export the targets file itself.
  install(
    EXPORT "${targetName}-cppTargets"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${targetName}-cpp"
      NAMESPACE Azure:: # Not the C++ namespace, but a namespace in terms of cmake.
      FILE "${targetName}-cppTargets.cmake"
    )

  # configure_package_config_file(), write_basic_package_version_file()
  include(CMakePackageConfigHelpers)

  # Produce package config file.
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/Config.cmake.in"
    "${targetName}-cppConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${targetName}-cpp"
    PATH_VARS
      CMAKE_INSTALL_LIBDIR)

  # Produce version file.
  write_basic_package_version_file(
    "${targetName}-cppConfigVersion.cmake"
      VERSION ${AZ_LIBRARY_VERSION} # the version that we extracted from version.hpp
      COMPATIBILITY SameMajorVersion
    )

  # Install package cofig and version files.
  install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${targetName}-cppConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${targetName}-cppConfigVersion.cmake"
      DESTINATION
        "${CMAKE_INSTALL_DATAROOTDIR}/${targetName}-cpp" # to shares/<our_pkg>
    )

  # Export all the installs above as package.
  export(PACKAGE "${targetName}-cpp")
endmacro()
