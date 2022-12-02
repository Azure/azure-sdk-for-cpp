# download test data
include(DownloadJsonTestData)

# test fixture to download test data
add_test(NAME "download_test_data" COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target download_test_data)
set_tests_properties(download_test_data PROPERTIES FIXTURES_SETUP TEST_DATA)

#############################################################################
# doctest library with the main function to speed up build
#############################################################################

add_library(doctest_main OBJECT src/unit.cpp)
# Remove global compile options from this target.
set_target_properties(doctest_main PROPERTIES COMPILE_OPTIONS "")
set_target_properties(doctest_main PROPERTIES
    COMPILE_DEFINITIONS "$<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>"
    COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>"
)
if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
    target_compile_features(doctest_main PUBLIC cxx_range_for)
else()
    target_compile_features(doctest_main PUBLIC cxx_std_11)
endif()
target_include_directories(doctest_main PRIVATE "thirdparty/doctest")

# https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

	# Disable warning C4566: character represented by universal-character-name '\uFF01' cannot be represented in the current code page (1252)
	# Disable warning C4996: 'nlohmann::basic_json<std::map,std::vector,std::string,bool,int64_t,uint64_t,double,std::allocator,nlohmann::adl_serializer>::operator <<': was declared deprecated
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4566 /wd4996")

	# https://github.com/nlohmann/json/issues/1114
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

#############################################################################
# one executable for each unit test file
#############################################################################

set(files
    src/unit-algorithms.cpp
    src/unit-allocator.cpp
    src/unit-alt-string.cpp
    #src/unit-bson.cpp                      - Not required.
    src/unit-capacity.cpp
    #src/unit-cbor.cpp                      - Not required.
    src/unit-class_const_iterator.cpp
    src/unit-class_iterator.cpp
    src/unit-class_lexer.cpp
    #src/unit-class_parser.cpp              - No `_json` support.
    src/unit-comparison.cpp
    src/unit-concepts.cpp
    src/unit-constructor1.cpp
    src/unit-constructor2.cpp
    src/unit-convenience.cpp
    src/unit-conversions.cpp
    src/unit-deserialization.cpp
    src/unit-element_access1.cpp
    src/unit-element_access2.cpp
    src/unit-inspection.cpp
    src/unit-items.cpp
    src/unit-iterators1.cpp
    src/unit-iterators2.cpp
    #src/unit-json_patch.cpp                - No `_json` support.
    #src/unit-json_pointer.cpp              - No `_json_pointer. 
    src/unit-large_json.cpp
    #src/unit-merge_patch.cpp               - No `_json` and `R` literal support.
    #src/unit-meta.cpp                      - No meta() required/supported.
    src/unit-modifiers.cpp
    #src/unit-msgpack.cpp                   - Not required.
    src/unit-noexcept.cpp
    src/unit-pointer_access.cpp
    src/unit-readme.cpp
    src/unit-reference_access.cpp
    #src/unit-regression.cpp                - No need to check for regressions as this version is unique.
    src/unit-serialization.cpp
    src/unit-testsuites.cpp
    src/unit-to_chars.cpp
    #src/unit-ubjson.cpp                    - Not Required
    src/unit-udt.cpp
    #src/unit-unicode.cpp                   - Takes long time to complete.
    src/unit-user_defined_input.cpp
    src/unit-wstring.cpp
    )

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "json-test-\\1" testcase ${file_basename})

    add_executable(${testcase} $<TARGET_OBJECTS:doctest_main> ${file})
    # Remove global compile options from this target.
    set_target_properties(${testcase} PROPERTIES COMPILE_OPTIONS "")
    target_compile_definitions(${testcase} PRIVATE DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
    target_compile_options(${testcase} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-deprecated;-Wno-float-equal>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
    )
    target_include_directories(${testcase} PRIVATE ${CMAKE_BINARY_DIR}/include thirdparty/doctest thirdparty/fifo_map)
    target_link_libraries(${testcase} PRIVATE ${NLOHMANN_JSON_TARGET_NAME} azure-core)

    if (JSON_Coverage)
        target_compile_options(${testcase} PRIVATE --coverage)
        target_link_libraries(${testcase} PRIVATE --coverage)
    endif()

    add_test(NAME "${testcase}"
        COMMAND ${testcase} ${DOCTEST_TEST_FILTER} --no-skip
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties("${testcase}" PROPERTIES LABELS "all" FIXTURES_REQUIRED TEST_DATA)

    if(JSON_Valgrind)
        add_test(NAME "${testcase}_valgrind"
            COMMAND ${memcheck_command} ${CMAKE_CURRENT_BINARY_DIR}/${testcase} ${DOCTEST_TEST_FILTER}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        set_tests_properties("${testcase}_valgrind" PROPERTIES LABELS "valgrind")
    endif()
endforeach()

add_executable(json_unit EXCLUDE_FROM_ALL $<TARGET_OBJECTS:doctest_main> ${files})
target_compile_definitions(json_unit PRIVATE DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
target_compile_options(json_unit PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-deprecated;-Wno-float-equal>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
)
target_include_directories(json_unit PRIVATE ${CMAKE_BINARY_DIR}/include thirdparty/doctest thirdparty/fifo_map)
target_link_libraries(json_unit ${NLOHMANN_JSON_TARGET_NAME})
add_dependencies(json_unit download_test_data)