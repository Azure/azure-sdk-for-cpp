# Performance Testing

## Writing a test 

Adding tests , at the moment relies in making modifications in a couple of repositories. 

## Azure-SDK-For-Cpp repo

E.G. https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-blobs 

### Location

Performance tests are located under **"test/perf"** folder under the service folder(e.g. alongside ut folder).

The perf test folder should be added in the parent CMakeLists.txt guarded by the **BUILD_PERF_TEST** flag.

E.G.

```markdown

    if(BUILD_PERFORMANCE_TESTS)
        add_subdirectory(test/perf)
    endif()

```

### Structure

As any other CPP project you will need a **CMakeLists.txt** file , along side **src** and **inc** folders

#### Contents of the **inc** directory

Under inc folder create a subfolder structure following your namespace hierarchy ending with another test folder since these are in the ...Test namespace.

E.G. test/perf/inc/azure/storage/blobs/test

The tests are defined in .hpp files under this folder. 

##### Test definition

In a .hpp file we define a class that will be contain the tests methods. The class is defined in the ::Test namespace for your service. 

The class will inherit from the **PerfTest** base  class and will override several methods as follows:
- Constructor(Azure::Perf::TestOptions options) : PerfTest(options)
  Options field are passed from the perf test framework and constain the various options defined for running the test.
- void Run(Azure::Core::Context const&) override {...}.
  Runs the actual test code. It is strongly recommended that the test code does as little extra work here as possible, it should consist solely of the actual test invocation. The test code should all assert, conditional statements ("if"/"else") or unnecessary method calls as any extra work will skew the data.
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
    
    Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

    return 0;
    }
```

#### CMakeLists.txt

Beyond the regular cmake defintion in your cmake file make sure to add 

```makefile
include(AzureVcpkg)
az_vcpkg_integrate()
...
include(PerfTest)
SETPERFDEPS(azure-storage-blobs-cpp VCPKG_SERVICE_VERSION)
```

The crucial part here is the SetPerfDeps cmake macro. \
The perf framework will set an environment variable based on the service name with the value representing a version, thus allowing to run the tests against diffrent VCPKG published versions. If the env is not defined then the test will build against the current source code of the service. 
There can be multiple set perf for each dependency of the service ( e.g. identity, storage). There is a bug in the main perf framework that causes some issues with multiple packages with different major versions, but it will get fixed.

## Pipeline definition
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
  
## Resources

If the tests require certain resources to exist beforehand the pipeline can deploy them based on the presence of the **perf-resources.bicep** file which will be used to deploy the required resources defined within. 

## Azure-SDK-Tools repo

## Location 

The performance automation is located under azure-sdk-tools\tools\perf-automation\Azure.Sdk.Tools.PerfAutomation folder. 
(https://github.com/Azure/azure-sdk-tools/tree/main/tools/perf-automation/Azure.Sdk.Tools.PerfAutomation) 

## Test definition

in the above mentioned folder resided the test defintion file "tests.yml". 

### The tests exists in other languages

If the test exists in other languages then making the CPP version visible to the framework requires adding under **Service**/**Languages** an entry with the name **CPP** followed by the pakages and versions available for the testing(this ties into the CMakeLists SetPerfDeps macro).

Next under the **Tests**/**Test** node with the desired name add the CPP test name. In this section mind the aArguments list , this ies with the regex in the cpp sdk pipeline.yml definition. 

### The test does not exist in other laguages

In this case you are the first to add the required nodes. The defintion is fairly simple and straightforward. 

## Pipeline

Once you have everything in place create a pipeline using the definition in your in the cpp repo by going to https://dev.azure.com/azure-sdk/internal/_build?definitionScope=%5Cperf and create a new one under the cpp node. 

To test intermediate definitions of your pipeline you can run the https://dev.azure.com/azure-sdk/internal/_build?definitionId=5121 pipline and set the proper values for the cpp node( make sure to deselect all other languages except cpp unless you want to run them).

