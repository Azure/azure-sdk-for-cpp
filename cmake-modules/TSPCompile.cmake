# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

find_package(Git)

macro(GenerateCodeFromTypeSpec TSP_SHA TSP_REPO_PATH TSP_DESTINATION TSP_SERVICE_PATH CODEGEN_SHA CODEGEN_DESTINATION)

    message("\
    GenerateCodeFromTypeSpec using the following params \n\
    TSP_SHA=${TSP_SHA} \n\
    TSP_REPO_PATH=${TSP_REPO_PATH} \n\
    TSP_DESTINATION=${TSP_DESTINATION} \n\
    TSP_SERVICE_PATH=${TSP_SERVICE_PATH} \n\
    CODEGEN_SHA=${CODEGEN_SHA} \n\
    CODEGEN_DESTINATION=${CODEGEN_DESTINATION}")
    set(CODEGEN_PATH packages/typespec-cpp)
    DownloadTSPFiles(${TSP_SHA} ${TSP_REPO_PATH} ${TSP_DESTINATION})
    DownloadCodeGenerator(${CODEGEN_SHA} ${CODEGEN_DESTINATION})
    GenerateCodeFromTSP(${TSP_DESTINATION} ${TSP_REPO_PATH} ${TSP_SERVICE_PATH} ${CODEGEN_DESTINATION} ${CODEGEN_PATH})
endmacro()

macro(DownloadTSPFiles TSP_SHA TSP_REPO_PATH TSP_DESTINATION)
    set(TSP_REPO "https://github.com/Azure/azure-rest-api-specs.git")
    message ("\
    DownloadTSPFiles using the following params \n\
    TSP_REPO = ${TSP_REPO} \n\
    TSP_SHA=${TSP_SHA} \n\
    TSP_REPO_PATH=${TSP_REPO_PATH} \n\
    TSP_DESTINATION=${TSP_DESTINATION}")

    if(Git_FOUND)
        message("Git found: ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Git not found")
    endif()
    
    set(DOWNLOAD_TSP_FOLDER ${CMAKE_SOURCE_DIR}/build/${TSP_DESTINATION})
    # if we have the git folder, we don't need to download it again
    # this also saves times on incremental builds
    if(NOT EXISTS ${DOWNLOAD_TSP_FOLDER}/.git)
    message("\
    First time setting up the repo in \n\
    ${TSP_DESTINATION}")
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
        message("Repo detected at ${TSP_DESTINATION}. Jumping ahead to checkout.") 
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

macro (DownloadCodeGenerator CODEGEN_SHA CODEGEN_DESTINATION)
    message("\
    DownloadCodeGenerator using the following params \n\
    CODEGEN_REPO=${CODEGEN_REPO} \n\
    CODEGEN_SHA=${CODEGEN_SHA} \n\
    CODEGEN_DESTINATION=${CODEGEN_DESTINATION}")

    if(Git_FOUND)
        message("Git found: ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Git not found")
    endif()

    set(CODEGEN_REPO "https://github.com/Azure/autorest.cpp.git")
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
        message("Repo detected at  ${TSP_DESTINATION}. Jumping ahead to checkout.") 
    endif()

    #checkout SHA
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${CODEGEN_SHA}
          WORKING_DIRECTORY ${DOWNLOAD_CODEGEN_FOLDER})
 
    if(NOT ${STATUS_CODE} EQUAL 0)
        message(FATAL_ERROR "CODEGEN download failed.")
    endif()
endmacro()

macro(GenerateCodeFromTSP TSP_DESTINATION TSP_REPO_PATH TSP_SERVICE_PATH CODEGEN_DESTINATION CODEGEN_PATH)
    message("\
    GenerateCodeFromTSP using the following params \n\
    TSP_DESTINATION=${TSP_DESTINATION}\n\
    TSP_REPO_PATH=${TSP_REPO_PATH}\n\
    TSP_SERVICE_PATH=${TSP_SERVICE_PATH}\n\
    CODEGEN_DESTINATION=${CODEGEN_DESTINATION}\n\
    CODEGEN_PATH=${CODEGEN_PATH}")
    message("Remember to Download the typspec-cpp emitter from npmjs.org")
    #TODO : https://github.com/Azure/azure-sdk-for-cpp/issues/6071
    set(DOWNLOAD_CODEGEN_FOLDER ${CMAKE_SOURCE_DIR}/build/${CODEGEN_DESTINATION}/${CODEGEN_PATH}/specs/${TSP_SERVICE_PATH})
    set(DOWNLOAD_TSP_FOLDER ${CMAKE_SOURCE_DIR}/build/${TSP_DESTINATION}/${TSP_REPO_PATH})
    set(TSP_FINAL_LOCATION ${DOWNLOAD_CODEGEN_FOLDER}/${TSP_SERVICE_PATH})
    set(SCRIPTS_FOLDER ${CMAKE_SOURCE_DIR}/eng/scripts/typespec/)
    message("\
    Will copy tsp files from \n\
    ${DOWNLOAD_TSP_FOLDER} to \n\
    ${DOWNLOAD_CODEGEN_FOLDER}")
    #copy tsp files to the codegen folder
    file(COPY ${DOWNLOAD_TSP_FOLDER} 
    DESTINATION ${DOWNLOAD_CODEGEN_FOLDER})
    message("\
    Will copy tsp generation scripts from \n\
    ${SCRIPTS_FOLDER} to \n\
    ${TSP_FINAL_LOCATION}")
    file(COPY ${SCRIPTS_FOLDER}
    DESTINATION ${TSP_FINAL_LOCATION})
    message("\
    Will copy \n\
    ${CMAKE_CURRENT_SOURCE_DIR}/client.tsp to \n\
    ${TSP_FINAL_LOCATION}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/client.tsp
    DESTINATION ${TSP_FINAL_LOCATION})
    message("\
    Will copy \n\
    ${CMAKE_CURRENT_SOURCE_DIR}/tspconfig.yaml to \n\
    ${TSP_FINAL_LOCATION}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tspconfig.yaml
    DESTINATION ${TSP_FINAL_LOCATION})
    #build codegen
    message("Building codegen in folder \n\
    ${TSP_FINAL_LOCATION}")
    execute_process(COMMAND pwsh Build-Codegen.ps1
    WORKING_DIRECTORY ${TSP_FINAL_LOCATION})
    
    #generate code
    message("\
    Use codegen in folder \n\
    ${TSP_FINAL_LOCATION}")
    execute_process(COMMAND pwsh Generate-Code.ps1
    WORKING_DIRECTORY ${TSP_FINAL_LOCATION})
endmacro()

macro(UpdateCodeFilesFromGenerated CODEGEN_DESTINATION TSP_SERVICE_PATH INCLUDE_DESTINATION SOURCE_DESTINATION)
    message("\
    Updating code files using the following params \n\
    CODEGEN_DESTINATION=${CODEGEN_DESTINATION} \n\
    TSP_SERVICE_PATH=${TSP_SERVICE_PATH} \n\
    INCLUDE_DESTINATION=${INCLUDE_DESTINATION} \n\
    SOURCE_DESTINATION=${SOURCE_DESTINATION}")
    set(CODEGEN_PATH packages/typespec-cpp/specs/${TSP_SERVICE_PATH}/${TSP_SERVICE_PATH})
    set(INCLUDE_SRC ${CMAKE_SOURCE_DIR}/build/${CODEGEN_DESTINATION}/${CODEGEN_PATH}/generated/inc/)
    set(SOURCE_SRC ${CMAKE_SOURCE_DIR}/build/${CODEGEN_DESTINATION}/${CODEGEN_PATH}/generated/src/)

    message("\
    Copying files from \n\
    ${INCLUDE_SRC} to \n\
    ${INCLUDE_DESTINATION}")
    file(COPY ${INCLUDE_SRC} DESTINATION ${INCLUDE_DESTINATION})

    message("\
    Copying files from \n\
    ${SOURCE_SRC} to \n\
    ${SOURCE_DESTINATION}")
    file(COPY ${SOURCE_SRC} DESTINATION ${SOURCE_DESTINATION})
    message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()
