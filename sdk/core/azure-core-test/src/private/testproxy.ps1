$error.clear()

$running = Get-Process -Name test-proxy

if($running)
{
    echo "test-proxy running, no need for new instance"
    exit 0
}

$error.Clear()
$CurrentVersion = (Get-Command -Name "test-proxy" -ErrorAction SilentlyContinue).Version

if($error){
    echo "Will install testproxy"

    dotnet tool update azure.sdk.tools.testproxy --global --add-source https://pkgs.dev.azure.com/azure-sdk/public/_packaging/azure-sdk-for-net/nuget/v3/index.json --version "1.0.0-dev*"
    $error.clear()
    $CurrentVersion = (Get-Command -Name "test-proxy" -ErrorAction SilentlyContinue).Version

    if($error){
        echo "unable to install testproxy. try manual install"
        exit 1
    }
}

echo "start test proxy"

Start-Process 'test-proxy' 