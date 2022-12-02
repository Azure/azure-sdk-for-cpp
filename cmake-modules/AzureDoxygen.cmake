# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Creates custom targets for documentation generation if
# BUILD_DOCUMENTATION == YES
# Usage: generate_documentation(azure-core 1.0.0-preview.1)
# Requires: Doxygen
# Target name in the form of ${PROJECT_NAME}-docs (e.g. azure-core-docs)

function(generate_documentation PROJECT_NAME PROJECT_VERSION)
    if(BUILD_DOCUMENTATION)
        find_package(Doxygen REQUIRED doxygen)

        set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
        set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})

        set(DOXYGEN_OUTPUT_DIRECTORY docs)
        set(DOXYGEN_LAYOUT_FILE ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/DoxygenLayout.xml)
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ./README.md)
        # Setting the INLINE_SOURCES tag to YES will include the body of functions,
        # classes and enums directly into the documentation.
        set(DOXYGEN_INLINE_SOURCES NO)
        # Skip generating docs for json, test, samples, and private files.
        set(DOXYGEN_EXCLUDE_PATTERNS
            json.hpp
            test
            samples
            *private*)
        # Skip documenting internal and private symbols (all from ::_detail/_::internal namespaces)
        set(DOXYGEN_EXCLUDE_SYMBOLS _detail _internal)
        set(DOXYGEN_IGNORE_PREFIX az_ AZ_)
        set(DOXYGEN_HTML_HEADER ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/header.html)
        set(DOXYGEN_HTML_FOOTER ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/footer.html)
        set(DOXYGEN_HTML_STYLESHEET ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/style.css)

        set(DOXYGEN_GENERATE_XML YES)
        set(DOXYGEN_GENERATE_LATEX NO)
        # Use MathJax instead of latex to render formulas
        set(DOXYGEN_USE_MATHJAX YES)

        set(DOXYGEN_REPEAT_BRIEF NO)

        doxygen_add_docs(${PROJECT_NAME}-docs
            ALL
            COMMENT "Generate documentation for ${PROJECT_NAME}")
    endif()
endfunction()