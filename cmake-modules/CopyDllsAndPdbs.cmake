# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Helper script to copy all DLL and PDB files from SOURCE_DIR to DEST_DIR
# This script is called via cmake -P from copy_shared_lib_binaries function

if(NOT DEFINED SOURCE_DIR OR NOT DEFINED DEST_DIR)
  message(FATAL_ERROR "SOURCE_DIR and DEST_DIR must be defined")
endif()

# Find all DLL files
file(GLOB DLL_FILES "${SOURCE_DIR}/*.dll")
foreach(dll_file ${DLL_FILES})
  get_filename_component(dll_name ${dll_file} NAME)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dll_file} ${DEST_DIR}/${dll_name}
  )
endforeach()

# Find all PDB files
file(GLOB PDB_FILES "${SOURCE_DIR}/*.pdb")
foreach(pdb_file ${PDB_FILES})
  get_filename_component(pdb_name ${pdb_file} NAME)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${pdb_file} ${DEST_DIR}/${pdb_name}
  )
endforeach()
