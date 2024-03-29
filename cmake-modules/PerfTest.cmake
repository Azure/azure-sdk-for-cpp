macro(SetPerfDeps PACKAGE VAR_RESULT)
    string(TOUPPER ${PACKAGE} SUFFIX)
    string(CONCAT VAR_TRIGGER "VCPKG-" ${SUFFIX})
    message(STATUS "trigger name ${VAR_TRIGGER}")
    if(DEFINED ENV{${VAR_TRIGGER}})
        find_package(${PACKAGE} $ENV{${VAR_TRIGGER}} EXACT)
        add_compile_definitions(${VAR_RESULT}="$ENV{${VAR_TRIGGER}}")
    else()
        add_compile_definitions(${VAR_RESULT}="source")
    endif()
endmacro()
