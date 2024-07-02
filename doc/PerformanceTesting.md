# Performance Testing

## Writing a test 

Adding tests , at the moment relies in making modifications in a couple of repositories. 

## Azure-SDK-For-Cpp repo

E.G. https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-blobs 

### Location

Performance tests are located under **"test/perf"** folder under the service folder(e.g. alongside ut folder).

The perf test folder should be added in the parent CMakeLists.txt guarded by the **BUILD_PERFORMANCE_TESTS** flag.

E.G.

```markdown
    if(BUILD_PERFORMANCE_TESTS)
        add_subdirectory(test/perf)
    endif()
```

### Structure

As any other CPP project you will need a **CMakeLists.txt** file , along side **src** and **inc** folders

#### Contents of the **inc** directory

Under inc folder create a sub-folder structure following your namespace hierarchy ending with another test folder since these are in the ...Test namespace.

E.G. test/perf/inc/azure/storage/blobs/test

The tests are defined in .hpp files under this folder. 

##### Test definition

In a .hpp file we define a class that will be contain the tests methods. The class is defined in the ::Test namespace for your service. 

The class will inherit from the **PerfTest** base  class and will override several methods as follows:
- Constructor(Azure::Perf::TestOptions options) : PerfTest(options)
  Options field are passed from the perf test framework and contain the various options defined for running the test.
- void Run(Azure::Core::Context const&) override {...}.
  Runs the actual test code. It is strongly recommended that the test code does as little extra work here as possible, it should consist solely of the actual test invocation. The test code should remove all assert, conditional statements ("if"/"else") or any other unnecessary work as any extra work will skew the results.
- std::vector<Azure::Perf::TestOption> GetTestOptions() override 
  Defines the various parameters for the test run that can be passed to the test from the performance framework. The perf framework uses these params to run various combinations(e.g. blob size)
- static Azure::Perf::TestMetadata GetTestMetadata()
  Returns TestMetadate object used to identify the test. 

#### Contents of the **src** directory

Contains one cpp file that contains the main method defintion 

```cpp
    int main(int argc, char** argv)
    {
    std::cout << "SERVICE VERSION " << VCPKG_SERVICE_VERSION << std::endl;
    // Create the test list
    std::vector<Azure::Perf::TestMetadata> tests{
        Service::Namespace::Test::TestName::GetTestMetadata(),
    };
    
    Azure::Perf::Program::Run(Azure::Core::Context{}, tests, argc, argv);

    return 0;
    }
```

#### CMakeLists.txt

Beyond the regular cmake definition in your cmake file make sure to add 

```makefile
include(AzureVcpkg)
az_vcpkg_integrate()
...
include(PerfTest)
SETPERFDEPS(azure-storage-blobs-cpp VCPKG_SERVICE_VERSION)
```

The crucial part here is the SETPERFDEPS cmake macro.
The perf framework will set an environment variable based on the service name with the value representing a version, thus allowing to run the tests against diffrent VCPKG published versions. If the env is not defined then the test will build against the current source code of the service. 
There can be multiple set perf for each dependency of the service ( e.g. identity, storage). 

## Pipeline definition

### Perf.yml
The file should be named `perf.yml`, and should be located in the service directory that is to be tested, for consistency and to keep service related resources in the service folder. 

```yml
parameters:
- name: PackageVersions
  displayName: PackageVersions (regex of package versions to run)
  type: string
  default: '12|source'
- name: Tests
  displayName: Tests (regex of tests to run)
  type: string
  default: '^(download|upload|list-blobs)$'
- name: Arguments
  displayName: Arguments (regex of arguments to run)
  type: string
  default: '(10240)|(10485760)|(1073741824)|(5 )|(500 )|(50000 )' 
- name: Iterations
  displayName: Iterations (times to run each test)
  type: number
  default: '5'
- name: AdditionalArguments
  displayName: AdditionalArguments (passed to PerfAutomation)
  type: string
  default: ' '

extends:
  template: /eng/pipelines/templates/jobs/perf.yml
  parameters:
    ServiceDirectory: service folder
    Services: "^service name$"
    PackageVersions: ${{ parameters.PackageVersions }}
    Tests: ${{ parameters.Tests }}
    Arguments: ${{ parameters.Arguments }}
    Iterations: ${{ parameters.Iterations }}
    AdditionalArguments: ${{ parameters.AdditionalArguments }}
    InstallLanguageSteps: 
      - pwsh: |
          Write-Host "##vso[task.setvariable variable=VCPKG_BINARY_SOURCES_SECRET;issecret=true;]clear;x-azblob,https://cppvcpkgcache.blob.core.windows.net/public-vcpkg-container,,read"
        displayName: Set Vcpkg Variables

    EnvVars: 
      # This is set in the InstallLanguageSteps 
      VCPKG_BINARY_SOURCES_SECRET: $(VCPKG_BINARY_SOURCES_SECRET)
```

The fields of interest here are the parameters:
- Tests which define which test s run in this pipeline ( which tests are available)
- Arguments which allow to filter which subset of parameters are acceptable for the tests from the plurality in the framework
- ServiceDirectory which represents the root folder of the service
- Services which represents the services which these tests target. 
  
### Perf-Tests.yml
Next to the perf.yaml  file you should have a perf-tests.yml file which contains the list of tests that are available for the service. 

```yml
Service: core

Project: azure-core-perf

PackageVersions:
- azure-core-cpp: 1.11.0
- azure-core-cpp: source

Tests:
  - Test: delay
    Class: delay
    Arguments:
    - --m 100 --n 1000 --t 500
```

The fields of interest here are the parameters:
- Service which represents the service name
- Project which represents the name of the project containing the tests 
- PackageVersions which represents the versions of the packages that are available for the tests
- Tests which represents the list of tests that are available for the service:
    - Test which represents the name of the test
    - Class which represents the name of the class that contains the test
    - Arguments which represents the arguments that are passed to the test n.b. the tests are run for each argument entry in the list. Thus make sure to specify all parameters that are required for the test to run in one entry.
 
### Test-proxy

The perf test pipeline installs and starts a test proxy server that is used to capture the network traffic of the tests. The test proxy server is specified with the **test-proxies** argument that is passed to the tests. This can be used to pass multiple proxies if so wanted and the tests will load balance against proxies in the list.

If a test proxy is specified the tests will first run with the test proxy in record mode and then in playback mode. The record mode will capture the network traffic in memory. The playback mode will replay the network traffic from the file. At the end of the test the test proxy will flush the recording from memory. 

## Resources

If the tests require certain resources to exist beforehand the pipeline can deploy them based on the presence of the **perf-resources.bicep** or **perf-resources.json** file which will be used to deploy the required resources defined within. 
As with any other test resource file the results are exported as environment variables that can be used in the tests.


## Azure-SDK-Tools repo

## Location 

The performance automation is located under azure-sdk-tools\tools\perf-automation\Azure.Sdk.Tools.PerfAutomation folder. 
(https://github.com/Azure/azure-sdk-tools/tree/main/tools/perf-automation/Azure.Sdk.Tools.PerfAutomation) 

## PerfAutomation tool 

The perf automation tool is used to run tests and report results. 
It is written in .net and it is invoked by the perf pipeline to execute the tests. 
The file relevant for us is the Cpp.cs file which defines how to build, run, and report the test results. 

## Running local 
### Option 1
Tests can be ran locally using the perf test executable that is built from the CMake file. passing along the required parameters such as test name and test related arguments. 

E.G. Adding to launch.vs.json the following configuration 
```bash
{
  "type": "default",
  "project": "CMakeLists.txt",
  "projectTarget": "azure-core-perf.exe (sdk\\core\\azure-core\\test\\perf\\azure-core-perf.exe)",
  "name": "azure-core-perf.exe (sdk\\core\\azure-core\\test\\perf\\azure-core-perf.exe)",
  "args": [
    "HTTPTransport",
    "--test-proxies",
    "http://127.0.0.1:5000",
    "--transport",
    "winhttp",
    "--method",
    "POST"
  ]
}
```
will allow you to run/debug the HttpTransport test using POST method, winhttp transport and a test proxy server running on localhost:5000.
### Option 2
the second options to run the tests locally you can use the perf automation tool and pass along the parameters required to run the required tests. 
E.G. In order to run the tests from azure-core-perf project you can pass the following debug command line arguments to the perf automation tool. 

```bash
--language Cpp --language-version N/A --repo-root E:\a --tests-file E:\a\sdk\core\perf-tests.yml --package-versions "1|source" --tests "^(delay|exception|extendedOptions|json|noop|nullable|uuid)$" --arguments ".*" --iterations 1
```

The parameters of interest here are :
 - --language Cpp : The language of the tests
 - --language-version N/A : The version of the language
 - --repo-root E:\a : The root of the repository
 - --tests-file E:\a\sdk\core\perf-tests.yml : The location of the perf-tests.yml file
 - --package-versions "1|source" : The versions of the packages that are available for the tests
 - --tests "^(delay|exception|extendedOptions|json|noop|nullable|uuid)$" : The tests that are run in this pipeline
 - --arguments ".*" : The arguments that are passed to the tests n.b. the tests are run for each argument entry in the list. Thus make sure to specify all parameters that are required for the test to run in one entry.
 - --iterations 1 : The number of iterations that the tests are run for.
 
 Tests and arguments are regexes that are used to filter the tests and arguments that are run. To run all tests and arguments you can use ".*" as the value.

## Pipeline

Once you have everything in place create a pipeline using the definition in your in the cpp repo by going to https://dev.azure.com/azure-sdk/internal/_build?definitionScope=%5Cperf and create a new one under the cpp node. 

To test intermediate definitions of your pipeline you can run the https://dev.azure.com/azure-sdk/internal/_build?definitionId=5121 pipeline and set the proper values for the cpp node( make sure to deselect all other languages except cpp unless you want to run them).

To validate a set of changes using an existing pipeline, navigate to an existing perf pipeline and press `Run`.  Enter the following information in the presented dialog: 
    - Branch/Tag: refs/pull/<PR_NUMBER>/merge
    - Commit: <COMMIT_HASH> 
    - The remaining arguments can be left as default. 

Press `Run` and wait for the pipeline to finish.
  
