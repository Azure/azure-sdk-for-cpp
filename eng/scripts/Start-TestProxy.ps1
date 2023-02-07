# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

param(
[Parameter(Mandatory=$True)]
[string] $AssetsPath
)
# check is there is another test-proxy running
$running = Get-Process -Name test-proxy
echo $AssetsPath
if($running)
{
    echo "test-proxy running, no need for new instance"
    exit 0
}

# make sure errors collection is empty
$error.clear()

#check if we have a test-proxy available
$CurrentVersion = (Get-Command -Name "test-proxy" -ErrorAction SilentlyContinue).Version

if($error){
    echo "Will install testproxy"

    dotnet tool update azure.sdk.tools.testproxy --global --add-source https://pkgs.dev.azure.com/azure-sdk/public/_packaging/azure-sdk-for-net/nuget/v3/index.json --version "1.0.0-dev*"
    # clear the errors again
    $error.clear()

    #check again for test proxy presence
    $CurrentVersion = (Get-Command -Name "test-proxy" -ErrorAction SilentlyContinue).Version

    # if we have errors this means we had issues installing it , needs to be done by hand
    if($error){
        echo "Unable to install testproxy. Try installing manually."
        exit 1
    }
}

echo "Start test proxy with argument list -l $AssetsPath"
#starts it in a separate process that will outlive pwsh in order to serve requests.
Start-Process 'test-proxy' -ArgumentList "start -l $AssetsPath" -WorkingDirectory $AssetsPath
