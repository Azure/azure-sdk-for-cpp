# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# The option is added again just like from the main CMakeList
# to enable discovering the option directly from each project.
option(BUILD_RTTI "Build libraries with run-time type information." ON)

macro(az_rtti_setup targetName packagePart rttiHeaderPath)
  if (BUILD_RTTI)
    target_compile_definitions(${targetName} PUBLIC AZ_RTTI)
    
    # Patch azure_rtti for installation with RTTI enabled.
    set(AZ_${packagePart}_RTTI "*/ + 1 /*")
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/inc/${rttiHeaderPath}"
      "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${rttiHeaderPath}"
      @ONLY
    )
    unset(AZ_${packagePart}_RTTI)

    get_filename_component(rttiHeaderDir ${rttiHeaderPath} DIRECTORY)
    install(
        FILES "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/${rttiHeaderPath}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${rttiHeaderDir}"
    )
    unset(rttiHeaderDir)

  else()
    message(STATUS "Building library ${targetName} without RTTI")
    if(MSVC)
      target_compile_options(${targetName} PUBLIC /GR-)
    else()
      target_compile_options(${targetName} PUBLIC -fno-rtti)
    endif()
  endif()
endmacro()