# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# We need to know an absolute path to our repo root to do things like referencing ./LICENSE.txt file.
set(AZ_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

message("ENV VARS FOR VCPKG:")
message("PATH: $ENV{VCPKG_INSTALLATION_ROOT}")
message("TRIPLET: $ENV{VCPKG_DEFAULT_TRIPLET}")
message("CACHE: $ENV{VCPKG_BINARY_SOURCES}")

macro(az_vcpkg_integrate)
  # vcpkg Integration
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

macro(az_vcpkg_portfile_prep targetName fileName contentToRemove)
  # with sdk/<lib>/vcpkg/<fileName>
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/${fileName}" fileContents)

  # Windows -> Unix line endings
  string(FIND fileContents "\r\n" crLfPos)

  if (crLfPos GREATER -1)
    string(REPLACE "\r\n" "\n" fileContents ${fileContents})
  endif()

  # remove comment header
  string(REPLACE "${contentToRemove}" "" fileContents ${fileContents})

  # undo Windows -> Unix line endings (if applicable)
  if (crLfPos GREATER -1)
    string(REPLACE "\n" "\r\n" fileContents ${fileContents})
  endif()
  unset(crLfPos)

  # output to an intermediate location
  file (WRITE "${CMAKE_BINARY_DIR}/vcpkg_prep/${targetName}/${fileName}" ${fileContents})
  unset(fileContents)

  # Produce the files to help with the vcpkg release.
  # Go to the /out/build/<cfg>/vcpkg directory, and copy (merge) "ports" folder to the vcpkg repo.
  # Then, update the portfile.cmake file SHA512 from "1" to the actual hash (a good way to do it is to uninstall a package,
  # clean vcpkg/downloads, vcpkg/buildtrees, run "vcpkg install <pkg>", and get the SHA from the error message).
  configure_file(
    "${CMAKE_BINARY_DIR}/vcpkg_prep/${targetName}/${fileName}"
    "${CMAKE_BINARY_DIR}/vcpkg/ports/${targetName}-cpp/${fileName}"
    @ONLY
  )
endmacro()

macro(az_vcpkg_export targetName macroNamePart dllImportExportHeaderPath)
  foreach(vcpkgFile "vcpkg.json" "portfile.cmake")
    az_vcpkg_portfile_prep(
      "${targetName}"
      "${vcpkgFile}"
      "# Copyright (c) Microsoft Corporation. All rights reserved.\n# SPDX-License-Identifier: MIT\n\n"
    )
  endforeach()

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

  # If building a Windows DLL, patch the dll_import_export.hpp
  if(WIN32 AND BUILD_SHARED_LIBS)
    add_compile_definitions(AZ_${macroNamePart}_BEING_BUILT)
    target_compile_definitions(${targetName} PUBLIC AZ_${macroNamePart}_DLL)

    set(AZ_${macroNamePart}_DLL_INSTALLED_AS_PACKAGE "*/ + 1 /*")
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/inc/${dllImportExportHeaderPath}"
      "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${dllImportExportHeaderPath}"
      @ONLY
    )
    unset(AZ_${macroNamePart}_DLL_INSTALLED_AS_PACKAGE)

    get_filename_component(dllImportExportHeaderDir ${dllImportExportHeaderPath} DIRECTORY)
    install(
        FILES "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${dllImportExportHeaderPath}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${dllImportExportHeaderDir}"
    )
    unset(dllImportExportHeaderDir)
  endif()

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
      VERSION ${AZ_LIBRARY_VERSION} # the version that we extracted from package_version.hpp
      COMPATIBILITY SameMajorVersion
    )

  # Install package config and version files.
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
