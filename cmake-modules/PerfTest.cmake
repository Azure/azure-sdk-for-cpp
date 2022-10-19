macro(SetPerfDeps PACKAGE VAR_RESULT)
    string(TOUPPER ${PACKAGE} SUFFIX)
    string(CONCAT VAR_TRIGGER "VCPKG-" ${SUFFIX})
    message(STATUS "trigger name ${VAR_TRIGGER}")
    if(DEFINED ENV{${VAR_TRIGGER}})
        find_package(${PACKAGE} $ENV{${VAR_TRIGGER}} EXACT)
        add_compile_definitions(${VAR_RESULT}="$ENV{${VAR_TRIGGER}}")
        find_path(WASTORAGE_INCLUDE_DIR /azure/storage/blobs.h)
        include_directories(${WASTORAGE_INCLUDE_DIR})
        # Include the headers from the project.
        target_include_directories(
          azure-storage-blobs-perf
            PUBLIC
            ${WASTORAGE_INCLUDE_DIR}>
)
    else()
        add_compile_definitions(${VAR_RESULT}="source")
        target_include_directories(
          azure-storage-blobs-perf
          PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        )
    endif()
endmacro()