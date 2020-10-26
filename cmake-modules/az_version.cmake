# Sets ${AZ_LIBRARY_VERSION} in the parent context with the version value
# constructed from version.hpp. The version.hpp must follow the example in
# templates for version information to parse correctly.

function(get_az_version VERSION_HPP_FILE)
    if(NOT EXISTS ${VERSION_HPP_FILE})
        message(FATAL_ERROR "Missing Version file ${VERSION_HPP_FILE}")
    endif()

    file (STRINGS ${VERSION_HPP_FILE} VERSION_H_CONTENT NEWLINE_CONSUME)
    message(STATUS "Retrieving version from ${VERSION_HPP_FILE}")

    # Find relevant version parts
    string(
        REGEX
        MATCH
        "#define AZURE_[A-Z_]+_VERSION_MAJOR ([0-9]+)[ \t\r\n]+#define AZURE_[A-Z_]+_VERSION_MINOR ([0-9]+)[ \t\r\n]+#define AZURE_[A-Z_]+_VERSION_PATCH ([0-9]+)[ \t\r\n]+#define AZURE_[A-Z_]+_VERSION_PRERELEASE \"([a-zA-Z0-9.]*)\""
        VERSION_PARTS
        ${VERSION_H_CONTENT})

    #Ensure we matched as expected.
    # MAJOR.MINOR.PATCH are required.
    # PRERELEASE is optional.
    if(NOT CMAKE_MATCH_1 AND NOT CMAKE_MATCH_2 AND NOT CMAKE_MATCH_3)
        message(FATAL_ERROR "Unexpected version format in ${VERSION_HPP_FILE}")
    endif()

    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
    # If there is a prerelease version
    if(CMAKE_MATCH_4)
        set(VERSION_PRERELEASE ${CMAKE_MATCH_4})
        set(
            AZ_LIBRARY_VERSION
            "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_PRERELEASE}"
            PARENT_SCOPE)
    else()
        set(
            AZ_LIBRARY_VERSION
            "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
            PARENT_SCOPE)
    endif()

    message(STATUS "VERSION_MAJOR " ${VERSION_MAJOR})
    message(STATUS "VERSION_MINOR " ${VERSION_MINOR})
    message(STATUS "VERSION_PATCH " ${VERSION_PATCH})
    message(STATUS "VERSION_PRERELEASE " ${VERSION_PRERELEASE})
    message(STATUS "AZ_LIBRARY_VERSION " ${AZ_LIBRARY_VERSION})
endfunction()
