# Cmake variables:
# UNIX   : is TRUE on all UNIX-like OS's, including Apple OS X and CygWin

# WIN32  : is TRUE on Windows. Prior to 2.8.4 this included CygWin
# MINGW  : is TRUE when using the MinGW compiler in Windows
# MSYS   : is TRUE when using the MSYS developer environment in Windows
# CYGWIN : is TRUE on Windows when using the CygWin version of cmake

# APPLE  : is TRUE on Apple systems. Note this does not imply the
#          system is Mac OS X, only that APPLE is #defined in C/C++
#          header files.

if (WIN32 OR MINGW OR MSYS OR CYGWIN)
  add_compile_definitions(WINDOWS)
endif (UNIX)
if (UNIX)
  add_compile_definitions(POSIX)
endif (UNIX)
