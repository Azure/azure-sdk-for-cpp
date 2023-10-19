#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Silences a CMake warning, no apparent effect on the Azure IoT SDK
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build with -fPIC always
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

option(run_valgrind "set run_valgrind to ON if tests are to be run under valgrind/helgrind/drd. Default is OFF" OFF)
set(compileOption_C "" CACHE STRING "passes a string to the command line of the C compiler")
set(compileOption_CXX "" CACHE STRING "passes a string to the command line of the C++ compiler")

# These are the include folders. (assumes that this file is in a subdirectory of c-utility)
get_filename_component(SHARED_UTIL_FOLDER ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
set(SHARED_UTIL_FOLDER "${SHARED_UTIL_FOLDER}" CACHE INTERNAL "this is the sharedLib directory" FORCE)
set(SHARED_UTIL_INC_FOLDER ${SHARED_UTIL_FOLDER}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)
set(SHARED_UTIL_SRC_FOLDER ${SHARED_UTIL_FOLDER}/src CACHE INTERNAL "this is what needs to be included when doing include sources" FORCE)
set(SHARED_UTIL_ADAPTER_FOLDER "${SHARED_UTIL_FOLDER}/adapters" CACHE INTERNAL "this is where the adapters live" FORCE)
# PAL will eventually absorb the contents of SHARED_UTIL_ADAPTER_FOLDER
set(SHARED_UTIL_PAL_FOLDER "${SHARED_UTIL_FOLDER}/pal" CACHE INTERNAL "this is the PAL common sources directory" FORCE)
set(SHARED_UTIL_PAL_INC_FOLDER "${SHARED_UTIL_FOLDER}/pal/inc" CACHE INTERNAL "this is the PAL include directory" FORCE)


#making a global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
    #on Linux, enable valgrind
    #these commands (MEMORYCHECK...) need to apear BEFORE include(CTest) or they will not have any effect
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOSX TRUE)
endif()

include(CTest)

include_directories(${SHARED_UTIL_INC_FOLDER})

# OS-specific header files for ref counting
if(MSVC)
    include_directories(${SHARED_UTIL_PAL_FOLDER}/windows)
else()
    include_directories(${SHARED_UTIL_PAL_FOLDER}/linux)
endif()

# if the compiler check fails (such as for iOS) header search will also fail - this allows it to be suppressed
if(NOT suppress_header_searches)
    include(CheckIncludeFiles)
    CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
    CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)
else()
    message(STATUS "Bypassing header search")
    set(HAVE_STDINT_H TRUE)
    set(HAVE_STDBOOL_H TRUE)
endif()

if ((NOT HAVE_STDINT_H) OR (NOT HAVE_STDBOOL_H))
    include_directories(${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/windowsce)
endif()

# System-specific compiler flags
if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
elseif(UNIX) #LINUX OR APPLE
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    if(NOT (IN_OPENWRT OR APPLE))
        set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
    endif()
endif()

enable_testing()

include(CheckSymbolExists)
function(detect_architecture symbol arch)
    if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if (ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()
if (MSVC)
    detect_architecture("_M_AMD64" x86_64)
    detect_architecture("_M_IX86" x86)
    detect_architecture("_M_ARM" ARM)
else()
    detect_architecture("__x86_64__" x86_64)
    detect_architecture("__i386__" x86)
    detect_architecture("__arm__" ARM)
endif()
if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
    set(ARCHITECTURE "GENERIC")
endif()
message(STATUS "target architecture: ${ARCHITECTURE}")

# if any compiler has a command line switch called "OFF" then it will need special care
if (NOT "${compileOption_C}" STREQUAL "")
    set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

if (NOT "${compileOption_CXX}" STREQUAL "")
    set(CMAKE_CXX_FLAGS "${compileOption_CXX} ${CMAKE_CXX_FLAGS}")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" CXX_FLAG_CXX11)

macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
      if (CXX_FLAG_CXX11)
        set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
      else()
        set (CMAKE_CXX_FLAGS "--std=c++0x ${CMAKE_CXX_FLAGS}")
      endif()
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)

macro(compileAsC11)
  if (CXX_FLAG_CXX11)
    if (CMAKE_VERSION VERSION_LESS "3.1")
      if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=c11 ${CMAKE_C_FLAGS}")
        set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
      endif()
    else()
      set (CMAKE_C_STANDARD 11)
      set (CMAKE_CXX_STANDARD 11)
    endif()
  else()
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "--std=c++0x ${CMAKE_CXX_FLAGS}")
    else()
      set (CMAKE_C_STANDARD 11)
      set (CMAKE_CXX_STANDARD 11)
    endif()
  endif()
endmacro(compileAsC11)

function(compileTargetAsC99 theTarget)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set_target_properties(${theTarget} PROPERTIES COMPILE_FLAGS "--std=c99")
    endif()
  else()
    set_target_properties(${theTarget} PROPERTIES C_STANDARD 99)
    set_target_properties(${theTarget} PROPERTIES CXX_STANDARD 11)
  endif()
endfunction()

function(compileTargetAsC11 theTarget)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      if (CXX_FLAG_CXX11)
        set_target_properties(${theTarget} PROPERTIES COMPILE_FLAGS "--std=c11 -D_POSIX_C_SOURCE=200112L")
      else()
        set_target_properties(${theTarget} PROPERTIES COMPILE_FLAGS "--std=c99 -D_POSIX_C_SOURCE=200112L")
      endif()
    endif()
  else()
    set_target_properties(${theTarget} PROPERTIES C_STANDARD 11)
    set_target_properties(${theTarget} PROPERTIES CXX_STANDARD 11)
  endif()
endfunction()

macro(generate_cpp_wrapper setVar whatIsBuilding)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${whatIsBuilding}.cxx "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${whatIsBuilding}.c\"")
  set(${setVar} ${CMAKE_CURRENT_BINARY_DIR}/${whatIsBuilding}.cxx)
endmacro(generate_cpp_wrapper)

macro(generate_cppunittest_wrapper whatIsBuilding)
    if (${use_cppunittest} AND WIN32)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${whatIsBuilding}.cxx "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${whatIsBuilding}.c\"")
      set(${whatIsBuilding}_test_files ${CMAKE_CURRENT_BINARY_DIR}/${whatIsBuilding}.cxx)
      #CPP compiler on windows likes to complain about unused local function removed (C4505)
      #C compiler doesn't like to complain about the same thing
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4505")
    else()
      set(${whatIsBuilding}_test_files ${whatIsBuilding}.c)
    endif()
endmacro(generate_cppunittest_wrapper)

IF((WIN32) AND (NOT(MINGW)))
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Make warning as error
    add_definitions(/WX)
ELSE()
    # Make warning as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
ENDIF()


function(add_files_to_install filesToBeInstalled)
    set(INSTALL_H_FILES ${INSTALL_H_FILES} ${filesToBeInstalled} CACHE INTERNAL "Files that will be installed on the system")
endfunction()

# XCode and stricter warning levels such as -Wall and -Wextra warn about unused
# variables and unused static functions, both of which are produced by serializer
function(usePermissiveRulesForSamplesAndTests)
    if (NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable  -Wno-unused-function -Wno-missing-braces -Wno-strict-aliasing")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable  -Wno-unused-function -Wno-missing-braces -Wno-strict-aliasing")
        if(NOT APPLE AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable -Wno-clobbered")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable -Wno-clobbered")
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
    endif()
endfunction()

# For targets which set warning switches as project properties (e.g. XCode)
function(setTargetBuildProperties stbp_target)
    if(XCODE)
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_COMMA "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INFINITE_RECURSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_NON_LITERAL_NULL_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_LITERAL_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_ROOT_CLASS "YES_ERROR")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_RANGE_LOOP_ANALYSIS "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_STRICT_PROTOTYPES "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_MOVE "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_UNREACHABLE_CODE "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN__DUPLICATE_METHOD_MATCH "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNDECLARED_SELECTOR "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS "YES_AGGRESSIVE")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE "YES")
    endif()
endfunction()

