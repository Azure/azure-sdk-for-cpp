# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# The option is added again just like from the main CMakeList
# to enable discovering the option directly from each project.
option(BUILD_RTTI "Build libraries with run-time type information." ON)

macro(az_rtti_setup targetName rttiHeaderPath)
  if (BUILD_RTTI)
    target_compile_definitions(${targetName} PUBLIC AZURE_SDK_RTTI_ENABLED)
    
    # Patch azure_rtti for installation with RTTI enabled.
    set(AZ_BUILD_WITH_RTTI "*/ + 1 /*")
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/inc/${rttiHeaderPath}"
      "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${rttiHeaderPath}"
      @ONLY
    )
    unset(AZ_BUILD_WITH_RTTI)

    get_filename_component(dllImportExportHeaderDir ${rttiHeaderPath} DIRECTORY)
    install(
        FILES "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${rttiHeaderPath}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${dllImportExportHeaderDir}"
    )
    unset(dllImportExportHeaderDir)

  else()
    message(STATUS "Building library ${targetName} without RTTI")
    if(MSVC)
      target_compile_options(${targetName} PUBLIC /GR-)
    else()
      target_compile_options(${targetName} PUBLIC -fno-rtti)
    endif()
  endif()
endmacro()
