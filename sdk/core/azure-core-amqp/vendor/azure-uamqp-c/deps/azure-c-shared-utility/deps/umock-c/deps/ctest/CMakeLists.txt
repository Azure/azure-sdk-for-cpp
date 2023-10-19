#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if(TARGET ctest)
    RETURN()
endif()

project(ctest)

set(CTEST_VERSION 1.1.19)

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build with -fPIC always
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
if ((MSVC) OR (UNIX) OR (LINUX))
# Enable coloring by default for Linux, *nix and Windows
option(use_coloring "use test coloring (default is ON)" ON)
else()
option(use_coloring "use test coloring (default is OFF)" OFF)
endif()
option(abort_on_fail "abort on test failure (default is OFF)" OFF)

include (CTest)
include (CheckIncludeFiles)

if(${use_coloring})
    add_definitions(-DUSE_COLORING)
endif()

if(${abort_on_fail})
    add_definitions(-DCTEST_ABORT_ON_FAIL)
endif()

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
  endif()
else()
  set (CMAKE_C_STANDARD 99)
endif()

if(MSVC)
    if (WINCE) # Be lax with WEC 2013 compiler
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
        # WEC 2013 uses older VS compiler. Build some files as C++ files to resolve C99 related compile issues
        SET_SOURCE_FILES_PROPERTIES(src/consolelogger.c src/xlogging.c src/map.c adapters/uniqueid_win32.c adapters/httpapi_wince.c src/tlsio_schannel.c src/x509_schannel.c PROPERTIES LANGUAGE CXX)
        add_definitions(-DWIN32) #WEC 2013 
    ELSE()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
  
    endif()
elseif(LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    if(NOT IN_OPENWRT)
        set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
    endif()
endif()

if (NOT TARGET azure_macro_utils_c) 
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c/CMakeLists.txt)
        add_subdirectory(deps/azure-macro-utils-c)
    else()
        message(FATAL_ERROR "Could not find azure_macro_utils_c")
    endif()
endif()

include_directories(${MACRO_UTILS_INC_FOLDER})

set(ctest_c_files
    ./src/ctest.c
)

set(ctest_h_files
    ./inc/ctest.h
)

if (MSVC)
set(ctest_c_files
    ${ctest_c_files}
    ./src/ctest_windows.c
)

set(ctest_h_files
    ${ctest_h_files}
    ./inc/ctest_windows.h
)
endif()

#these are the include folders
#the following "set" statetement exports across the project a global variable called CTEST_INC_FOLDER that expands to whatever needs to included when using ctest library
set(CTEST_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using ctest lib" FORCE)

include_directories(${CTEST_INC_FOLDER})

CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)

if ((NOT HAVE_STDINT_H) OR (NOT HAVE_STDBOOL_H))
    include_directories(${CTEST_INC_FOLDER}/aux_inc)
endif()

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

add_library(
ctest ${ctest_c_files} ${ctest_h_files}
)

target_include_directories(ctest
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(ctest
               PROPERTIES
               FOLDER "test_tools")

if (${run_unittests})
     add_subdirectory(tests)
endif()

# Set CMAKE_INSTALL_* if not defined
include(GNUInstallDirs)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Install Azure CTest
set(package_location "cmake")

install (TARGETS ctest EXPORT ctestTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/../bin
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install (FILES ${ctest_h_files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${CTEST_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file("configs/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    COPYONLY
)

install(EXPORT ctestTargets
    FILE
        "${PROJECT_NAME}Targets.cmake"
    DESTINATION
        ${package_location}
)
install(
    FILES
        "configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        ${package_location}
)
