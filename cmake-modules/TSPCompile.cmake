# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

find_package(Git)

macro(DownloadTSPFiles TSP_REPO TSP_SHA TSP_REPO_PATH TSP_DESTINATION)
    message ("Downloading TSP files using the following params  TSP_REPO=${TSP_REPO} TSP_SHA=${TSP_SHA} TSP_REPO_PATH=${TSP_REPO_PATH}  TSP_DESTINATION=${TSP_DESTINATION}")

    if(Git_FOUND)
        message("Git found: ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Git not found")
    endif()
    
    set(DOWNLOAD_TSP_FOLDER ${CMAKE_SOURCE_DIR}/build/${TSP_DESTINATION})
    # if we have the git folder, we don't need to download it again
    # this also saves times on incremental builds
    if(NOT EXISTS ${DOWNLOAD_TSP_FOLDER}/.git)
    message("First time setting up the ${TSP_DESTINATION} repo.")
        #make folder
        make_directory(${DOWNLOAD_TSP_FOLDER})
    
        #init git in folder
        execute_process(COMMAND ${GIT_EXECUTABLE} init
            WORKING_DIRECTORY ${DOWNLOAD_TSP_FOLDER})
        #add remote
        execute_process(COMMAND ${GIT_EXECUTABLE} remote add origin ${TSP_REPO}
             WORKING_DIRECTORY ${DOWNLOAD_TSP_FOLDER})
        #set sparse-checkout
        execute_process(COMMAND ${GIT_EXECUTABLE} sparse-checkout init --cone
             WORKING_DIRECTORY ${DOWNLOAD_TSP_FOLDER})
        #set sparse-checkout folder
        execute_process(COMMAND ${GIT_EXECUTABLE} sparse-checkout set ${TSP_REPO_PATH}
             WORKING_DIRECTORY ${DOWNLOAD_TSP_FOLDER})
    else()
        message("Repo detected at ${TSP_DESTINATION}. Jumping aheat to checkout.") 
    endif()
    #fetch
    execute_process(COMMAND ${GIT_EXECUTABLE} fetch
         WORKING_DIRECTORY ${DOWNLOAD_TSP_FOLDER})
    #switch branch
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${TSP_SHA}
          WORKING_DIRECTORY ${DOWNLOAD_TSP_FOLDER})
 
    if(NOT ${STATUS_CODE} EQUAL 0)
        message(FATAL_ERROR "TSP download failed.")
    endif()
endmacro()

macro (DownloadCodeGenerator CODEGEN_REPO CODEGEN_SHA CODEGEN_DESTINATION)
    message("Downloading CODEGEN files using the following params  CODEGEN_REPO=${CODEGEN_REPO} CODEGEN_SHA=${CODEGEN_SHA} CODEGEN_DESTINATION=${CODEGEN_DESTINATION}")

    if(Git_FOUND)
        message("Git found: ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Git not found")
    endif()
    
    set(DOWNLOAD_CODEGEN_FOLDER ${CMAKE_SOURCE_DIR}/build/${CODEGEN_DESTINATION})

    # if we have the git folder, we don't need to download it again
    # this also saves times on incremental builds
    if(NOT EXISTS ${DOWNLOAD_CODEGEN_FOLDER}/.git)
        message("First time setting up the ${CODEGEN_DESTINATION} repo.")
        #make folder
        make_directory(${DOWNLOAD_CODEGEN_FOLDER})
        #init git in folder
        execute_process(COMMAND ${GIT_EXECUTABLE} clone ${CODEGEN_REPO} ${DOWNLOAD_CODEGEN_FOLDER})
    else()
        message("Repo detected at  ${TSP_DESTINATION}. Jumping aheat to checkout.") 
    endif()

    #checkout SHA
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${CODEGEN_SHA}
          WORKING_DIRECTORY ${DOWNLOAD_CODEGEN_FOLDER})

    if(NOT ${STATUS_CODE} EQUAL 0)
        message(FATAL_ERROR "CODEGEN download failed.")
    endif()
endmacro()

macro(GenerateCode TSP_DESTINATION TSP_REPO_PATH CODEGEN_DESTINATION)
    message("Generating code using the following params  TSP_DESTINATION=${TSP_DESTINATION} TSP_REPO_PATH=${TSP_REPO_PATH} CODEGEN_DESTINATION=${CODEGEN_DESTINATION}")

    set(DOWNLOAD_CODEGEN_FOLDER ${CMAKE_SOURCE_DIR}/build/${CODEGEN_DESTINATION})
    set(DOWNLOAD_TSP_FOLDER ${CMAKE_SOURCE_DIR}/build/${TSP_DESTINATION})

endmacro()
