macro(CopyTestProxyScripts)
    #file(COPY ${CMAKE_SOURCE_DIR}/sdk/core/azure-core-test/src/private/testproxy.ps1
    #    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    #file(COPY ${CMAKE_SOURCE_DIR}/sdk/core/azure-core-test/src/private/stopProxy.ps1
    #    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    write_file(${CMAKE_CURRENT_BINARY_DIR}/stopProxy.ps1 "Stop-Process -Name \"test-proxy\"" )
    write_file(${CMAKE_CURRENT_BINARY_DIR}/testProxy.ps1 "\
# check is there is another test-proxy running\n \
$running = Get-Process -Name test-proxy\n \
\n \
if($running)\n \
{\n \
    echo \"test-proxy running, no need for new instance\"\n \
    exit 0\n \
}\n \
\n \
# make sure errors collection is empty\n \
$error.clear()\n \
\n \
#check if we have a test-proxy available\n \
$CurrentVersion = (Get-Command -Name \"test-proxy\" -ErrorAction SilentlyContinue).Version\n \
\n \
if($error){\n \
    echo \"Will install testproxy\"\n \
\n \
    dotnet tool update azure.sdk.tools.testproxy --global --add-source https://pkgs.dev.azure.com/azure-sdk/public/_packaging/azure-sdk-for-net/nuget/v3/index.json --version \"1.0.0-dev*\"\n \
    # clear the errors again\n \
    $error.clear()\n \
\n \
    #check again for test proxy presence\n \
    $CurrentVersion = (Get-Command -Name \"test-proxy\" -ErrorAction SilentlyContinue).Version\n \
\n \
    # if we have errors this means we had issues installing it , needs to be done by hand\n \
    if($error){\n \
        echo \"unable to install testproxy. try manual install\"\n \
        exit 1\n \
    }\n \
}\n \
\n \
echo \"start test proxy\"\n \
#starts it in a separate process that will outlive pwsh in order to serve requests.\n \
Start-Process 'test-proxy'\n \
")
endmacro()