macro(GetFolderList project)
    message ("project found ${project}")
    message ("FLAG VALUE : ${FETCH_SOURCE_DEPS}")
    if(${project} STREQUAL CERTIFICATES)
        DownloadDepVersion(sdk/core azure-core 1.4.0)
    elseif(${project} STREQUAL IDENTITY)
        DownloadDepVersion(sdk/core azure-core 1.2.0)
    elseif(${project} STREQUAL SECRETS)
        DownloadDepVersion(sdk/core azure-core 1.4.0)
    elseif(${project} STREQUAL KEYS)
        DownloadDepVersion(sdk/core azure-core 1.4.0)
    elseif(${project} STREQUAL STORAGE_COMMON)
        DownloadDepVersion(sdk/core azure-core 1.3.1)
    elseif(${project} STREQUAL STORAGE_BLOBS)
        DownloadDepVersion(sdk/core azure-core 1.3.1)
        DownloadDepVersion(sdk/storage/azure-storage-common azure-storage-common 12.2.2)
     elseif(${project} STREQUAL STORAGE_DATALAKE)
        DownloadDepVersion(sdk/core azure-core 1.3.1)
        DownloadDepVersion(sdk/storage/azure-storage-common azure-storage-common 12.2.2)
        DownloadDepVersion(sdk/storage/azure-storage-blobs azure-storage-blobs 12.3.0)
     elseif(${project} STREQUAL STORAGE_SHARES)
        DownloadDepVersion(sdk/core azure-core 1.3.1)
        DownloadDepVersion(sdk/storage/azure-storage-common azure-storage-common 12.2.2)
    elseif(${project} STREQUAL STORAGE_QUEUES)
        DownloadDepVersion(sdk/core azure-core 1.3.1)
        DownloadDepVersion(sdk/storage/azure-storage-common azure-storage-common 12.2.2)
    endif()
    list(REMOVE_DUPLICATES BUILD_FOLDERS)
endmacro()

macro(SetGlobalOptions)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
    option(BUILD_TRANSPORT_CURL "Build an HTTP transport implementation with CURL" OFF)
    option(BUILD_TRANSPORT_WINHTTP "Build an HTTP transport implementation with WIN HTTP" OFF)
    option(BUILD_TRANSPORT_CUSTOM "Implementation for AzureSdkGetCustomHttpTransport function must be linked to the final application" OFF)
    option(BUILD_TESTING "Build test cases" OFF)
    option(BUILD_RTTI "Build libraries with run-time type information." ON)
    option(BUILD_CODE_COVERAGE "Build gcov targets for HTML and XML reports. Requires debug build and BUILD_TESTING" OFF)
    option(BUILD_DOCUMENTATION "Create HTML based API documentation (requires Doxygen)" OFF)
    option(BUILD_SAMPLES "Build sample application for Azure Storage clients" OFF)
    option(RUN_LONG_UNIT_TESTS "Tests that takes more than 5 minutes to complete. No effect if BUILD_TESTING is OFF" OFF)
    option(BUILD_PERFORMANCE_TESTS "Build the performance test library" OFF)
    option(MSVC_USE_STATIC_CRT "(MSVC only) Set to ON to link SDK with static CRT (/MT or /MTd switch)." OFF)
    option(FETCH_SOURCE_DEPS "fetch source dependencies for a package, not for global use, instead use when building specific component" OFF)
endmacro()

macro(SetCompileOptions project)
    message ("setting up compile options for ${project}")
    # Compile Options
    SetGlobalOptions()
endmacro()

macro(DownloadDepVersion DEP_FOLDER DEP_NAME DEP_VERSION)

    file(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/build/${DEP_FOLDER})
    set(DOWNLOAD_FOLDER ${CMAKE_SOURCE_DIR}/build/downloads)
    set(DOWNLOAD_FILE ${DEP_NAME}_${DEP_VERSION}.zip)
    set(DEP_PREFIX azure-sdk-for-cpp)

    if(FETCH_SOURCE_DEPS STREQUAL "LATEST")
        message("Downloading latest version of ${DEP_NAME}")
        #get the latest version from main
        file(DOWNLOAD http://github.com/Azure/azure-sdk-for-cpp/archive/main.zip ${DOWNLOAD_FOLDER}/${DOWNLOAD_FILE})
    else()
        message("Downloading version ${DEP_VERSION} of ${DEP_NAME}")
        # get the zip
        file(DOWNLOAD https://github.com/Azure/azure-sdk-for-cpp/archive/refs/tags/${DOWNLOAD_FILE} ${DOWNLOAD_FOLDER}/${DOWNLOAD_FILE})
    endif()

    #extract the zip
    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_FOLDER}/${DOWNLOAD_FILE} DESTINATION ${DOWNLOAD_FOLDER}/${DEP_NAME})
    #make target folder
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${DEP_FOLDER})
    
    # need a nicer way to copy/move folder 
    # i need to archive the folder then extract at new location
    if(FETCH_SOURCE_DEPS STREQUAL "LATEST")
        execute_process(COMMAND tar -cf  ${DOWNLOAD_FOLDER}/${DEP_NAME}.tar -C ${DOWNLOAD_FOLDER}/${DEP_NAME}/azure-sdk-for-cpp-main/${DEP_FOLDER} .)
    else()
        execute_process(COMMAND tar -cf  ${DOWNLOAD_FOLDER}/${DEP_NAME}.tar -C ${DOWNLOAD_FOLDER}/${DEP_NAME}/${DEP_PREFIX}-${DEP_NAME}_${DEP_VERSION}/${DEP_FOLDER} .)
    endif()
    
    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_FOLDER}/${DEP_NAME}.tar DESTINATION ${CMAKE_SOURCE_DIR}/build/${DEP_FOLDER})
    #cleanup
    file(REMOVE_RECURSE ${DOWNLOAD_FOLDER})
    #add dependency folder to build list
    list(APPEND BUILD_FOLDERS build/${DEP_FOLDER})

endmacro()
