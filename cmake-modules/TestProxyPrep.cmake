macro(CopyTestProxyScripts)
    file(COPY ${CMAKE_SOURCE_DIR}/sdk/core/azure-core-test/src/private/testproxy.ps1
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_SOURCE_DIR}/sdk/core/azure-core-test/src/private/stopProxy.ps1
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

endmacro()
