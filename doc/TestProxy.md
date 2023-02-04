# Test Proxy

As the name implies is a proxy for http(s) requests performed by tests. While requests are proxied it allows for the recording and playback of the responses matching the requests. this way we can have a deterministic set og http request/responses allowing for predictable tests. 

## Recordings

The recordings are stored long term in a separate git repo indicated in the assets.json files peppered throughout the repo. 
E.G.:
```JSON
{
  "AssetsRepo": "Azure/azure-sdk-assets",
  "AssetsRepoPrefixPath": "cpp",
  "TagPrefix": "cpp/keyvault",
  "Tag": "cpp/keyvault_ea82152bd3"
}
```
The recordings are kept as releases for the repo with the tag indicated in the json file.

## Install test-proxy

```BASH
dotnet tool update azure.sdk.tools.testproxy --global --add-source https://pkgs.dev.azure.com/azure-sdk/public/_packaging/azure-sdk-for-net/nuget/v3/index.json --version "1.0.0-dev*"
```

Will install the latest version of the test-proxy using the dotnet tool.

## Starting test-proxy

When starting the test proxy you have several options :

* in the folder where assets.json exists run "test-proxy". in this case the proxy will read the local assets file and will download the indicated recording set when the first test is  executed. 
* by running test-proxy from any location and passing in the path to the assets.json file as a parameter to the executable. (test-proxy --help)
* as a docker image (https://github.com/Azure/azure-sdk-tools/tree/main/tools/test-proxy/docker)

## Creating a recording 
When AZURE_TEST_MODE=RECORD we will  first call the test-proxy /record/start endpoint with a json payload indicating we are starting a test run, when the test is done we call recording/stop with the recording id previously returned when starting. 

The proxy will create a file locally containing the data in the .assets folder in the root of the enlistment. 

## Playback 

When AZURE_TEST_MODE=PLAYBACK we invoke the playback/start endpoint on test-proxy which causes it to download the appropriate release indicated in assets.json if it is not there already, and replay the responses for the specific request based on the recorded data. When the test is done we call playback/stop

## LIve Testing

When AZURE_TEST_MODE=LIVE all requests are sent directly to their destination bypassing the proxy.

## Updating the remote

When all recordings are done, we can call "test-proxy push" which will cause the proxy to push the changes to the remote, create a new release with the appropriate tags and generate the assets.json file. 

## Advanced

The test-proxy will fetch the assets lazily, thus the first test execution will take a long time depending on external factors such as network, disk speed, cpu etc. While this is a one time event it can cause timeouts especially in environments that are time bound(e.g. piplines). 
To solve this issue we can run test-proxy -restore, which will force the download of the data. 