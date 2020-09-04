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
        set(DOXYGEN_INLINE_SOURCES YES)
        set(DOXYGEN_IGNORE_PREFIX az_ AZ_)
        set(DOXYGEN_HTML_HEADER ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/header.html)
        set(DOXYGEN_HTML_FOOTER ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/footer.html)
        set(DOXYGEN_HTML_STYLESHEET ${CMAKE_SOURCE_DIR}/eng/docs/api/assets/style.css)

        set(DOXYGEN_GENERATE_XML YES)
        set(DOXYGEN_GENERATE_LATEX NO)
        # Use MathJax instead of latex to render formulas
        set(DOXYGEN_USE_MATHJAX YES)

        doxygen_add_docs(${PROJECT_NAME}-docs
            ALL
            COMMENT "Generate documentation for ${TARGET_NAME}")
    endif()
endfunction()