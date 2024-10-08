# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

find_package(Git)

macro(DownloadTSPFiles TSP_REPO TSP_BRANCH TSP_REPO_PATH TSP_DESTINATION)
    message ("Downloading TSP files using the following params  TSP_REPO=${TSP_REPO} TSP_BRANCH=${TSP_BRANCH} TSP_REPO_PATH=${TSP_REPO_PATH}  TSP_DESTINATION=${TSP_DESTINATION}")

    if(Git_FOUND)
        message("Git found: ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Git not found")
    endif()
    
    set(DOWNLOAD_FOLDER ${CMAKE_SOURCE_DIR}/build/${TSP_DESTINATION})
    #cleanup folder
    file(REMOVE_RECURSE ${DOWNLOAD_FOLDER})
    make_directory(${DOWNLOAD_FOLDER})

    #init git in folder
    execute_process(COMMAND ${GIT_EXECUTABLE} init
        WORKING_DIRECTORY ${DOWNLOAD_FOLDER})
    #add remote
    execute_process(COMMAND ${GIT_EXECUTABLE} remote add origin ${TSP_REPO}
         WORKING_DIRECTORY ${DOWNLOAD_FOLDER})
    #set sparse-checkout
    execute_process(COMMAND ${GIT_EXECUTABLE} sparse-checkout init --cone
         WORKING_DIRECTORY ${DOWNLOAD_FOLDER})
    #set sparse-checkout folder
    execute_process(COMMAND ${GIT_EXECUTABLE} sparse-checkout set ${TSP_REPO_PATH}
         WORKING_DIRECTORY ${DOWNLOAD_FOLDER})
    #fetch
    execute_process(COMMAND ${GIT_EXECUTABLE} fetch
         WORKING_DIRECTORY ${DOWNLOAD_FOLDER})
    #switch branch
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${TSP_BRANCH}
          WORKING_DIRECTORY ${DOWNLOAD_FOLDER})
 
    if (NOT ${STATUS_CODE} EQUAL 0)
        message(FATAL_ERROR "TSP download failed (Link: ${TSP_FULL_PATH}).")
    endif()
endmacro()
