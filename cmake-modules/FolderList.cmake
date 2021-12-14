macro(GetFolderList project)
    message ("project found ${project}")
  
    if(${project} STREQUAL CERTIFICATES)
        DownloadDepVersion(sdk/core azure-core 1.2.1)
        DownloadDepVersion(sdk/identity azure-identity 1.1.0)
        #list(APPEND BUILD_FOLDERS "sdk/keyvault/azure-security-keyvault-certificates")
    endif()

    list(REMOVE_DUPLICATES BUILD_FOLDERS)
endmacro()

macro(SetCompileOptions project)
    message ("setting up compile options for ${project}")
    
    if(${project} STREQUAL CERTIFICATES)
    # Compile Options
        option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
        option(BUILD_TRANSPORT_CURL "Build an HTTP transport implementation with CURL" OFF)
        option(BUILD_TRANSPORT_WINHTTP "Build an HTTP transport implementation with WIN HTTP" OFF)
        option(BUILD_TRANSPORT_CUSTOM "Implementation for AzureSdkGetCustomHttpTransport function must be linked to the final application" OFF)
        option(BUILD_TESTING "Build test cases" ON)
        option(BUILD_RTTI "Build libraries with run-time type information." ON)
        option(BUILD_CODE_COVERAGE "Build gcov targets for HTML and XML reports. Requires debug build and BUILD_TESTING" OFF)
        option(BUILD_DOCUMENTATION "Create HTML based API documentation (requires Doxygen)" OFF)
        option(RUN_LONG_UNIT_TESTS "Tests that takes more than 5 minutes to complete. No effect if BUILD_TESTING is OFF" OFF)
        option(BUILD_PERFORMANCE_TESTS "Build the performance test library" OFF)
        option(MSVC_USE_STATIC_CRT "(MSVC only) Set to ON to link SDK with static CRT (/MT or /MTd switch)." OFF)
        option(BUILD_FROM_SOURCE_EXPORT "if export is built from source" ON)
        option(BUILD_CERTIFICATES "only build certificates and dependencies" ON)
        option(TESTING_BUILD "BUILD test cases" ON)
    endif()
endmacro()

macro(DownloadDepVersion DEP_FOLDER DEP_NAME DEP_VERSION)

    file(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/${DEP_FOLDER})
    set(DOWNLOAD_FOLDER ${CMAKE_SOURCE_DIR}/downloads)
    set(DOWNLOAD_FILE ${DEP_NAME}_${DEP_VERSION}.zip)
    set(DEP_PREFIX azure-sdk-for-cpp)
    # get the zip
    file(DOWNLOAD https://github.com/Azure/azure-sdk-for-cpp/archive/refs/tags/${DOWNLOAD_FILE} ${DOWNLOAD_FOLDER}/${DOWNLOAD_FILE})

    #extract the zip
    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_FOLDER}/${DOWNLOAD_FILE} DESTINATION ${DOWNLOAD_FOLDER}/${DEP_NAME})
    #make target folder
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/${DEP_FOLDER})
    # need a nicer way to copy/move folder 
    # this is stupid, i need to archive the folder then extract at new location
    execute_process(COMMAND tar -cf  ${DOWNLOAD_FOLDER}/${DEP_NAME}.tar -C ${DOWNLOAD_FOLDER}/${DEP_NAME}/${DEP_PREFIX}-${DEP_NAME}_${DEP_VERSION}/${DEP_FOLDER} .)
    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_FOLDER}/${DEP_NAME}.tar DESTINATION ${CMAKE_SOURCE_DIR}/${DEP_FOLDER})
    #cleanup
    file(REMOVE_RECURSE ${DOWNLOAD_FOLDER})
    #add dependency folder to build list
    list(APPEND BUILD_FOLDERS ${DEP_FOLDER})

endmacro()