macro(CopyTestProxyScripts)
    file(COPY ${CMAKE_SOURCE_DIR}/eng/Scripts/Start-TestProxy.ps1
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_SOURCE_DIR}/eng/Scripts/Stop-TestProxy.ps1
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endmacro()
