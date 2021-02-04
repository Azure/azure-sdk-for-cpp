# Azure SDK performance stress C++

Azure performance stress for C++ (`azure-performance-stress`) provides shared primitives, abstractions, and helpers for running performance tests for an Azure SDK clients for C++. It represent the C++ version of the [.NET original version](https://github.com/Azure/azure-sdk-for-net/tree/master/common/Perf).

## Getting started

See the [pre-requisites](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pre-requisites) for building before going to the next step.

### Build

Use the build option `BUILD_PERFORMANCE_TESTS` to build the test framework and all the performance tests. See the next example.

```bash
#
# Building the Performance framework and tests.
#
git clone https://github.com/Azure/azure-sdk-for-cpp.git
cd azure-sdk-for-cpp
mkdir build
cd build
cmake -DBUILD_TESTING=ON -DBUILD_PERFORMANCE_TESTS=ON ..
cmake --build .
```

### Run

Once building is completed, a performance test application will be created inside the the test folder from each service SDK. For instance, the Azure core performance test application would be inside: `build/sdk/core/azure-core/test/performance`. See next example for running the performance framework.

```bash
#
# Running the performance framework tests application
#
# From within the build folder (build)
./sdk/core/performance-stress/test/azure-performance-stress-test
```

>Note: When building the code with windows using Visual Studio, use the [cmake project view](https://docs.microsoft.com/cpp/build/cmake-projects-in-visual-studio?view=msvc-160) to run the performance tests. Find the tests directly in the tests folder from the cmake tree next to all other test cmake projects.

#### Options for running

After running the performance test application like it is mentioned above (without any command line arguments) the application will list the available test names to be run. The application will expect the test name to be executed as a mandatory argument. If the input test name is not found, the application will return an error and will terminate. The next pattern represents the right usage of the performance test application:
```bash
usage: azure-performance-stress-test testName [options]
```

>Note: You can use the option `-h` to print out the available options for a test name.

The next options can be used for any test:
| Option     | Activators | Description | Default | Example |
| ---------- | ---        | ---| ---| --- |
| Duration   | -d, --duration   | Duration of the test in seconds                  | 10    | -d 5
| Host       | --host           | Host to redirect HTTP requests                   | NA    | --host=https://something.com
| Insecure   | --insecure       | Allow untrusted SSL certs                        | false | --insecure=true
| Iterations | -i, --iterations | Number of iterations of main test loop           | 1     | -d 5
| Statistics | --statistics     | Print job statistics                             | false | --statistics=true
| Latency    | -l, --latency    | Track and print per-operation latency statistics | false | -l true
| No Clean   | --noclean        | Disables test clean up                           | false | --nocleanup=true
| Parallel   | -p, --parallel   | Number of operations to execute in parallel      | 1     | -p 5
| Port       | --port           | Port to redirect HTTP requests                   | NA    | --port=5000
| Rate       | -r, --rate       | Target throughput (ops/sec)                      | NA    | -r 3000
| Warm up    | -w, --warmup     | Duration of warmup in seconds                    | 5     | -w 0 (no warm up)

## Creating a performance test

Find below how to create a new CMake performance test project from scratch to an existing CMake project. Then how to add the performance tests to it.

### The CMake project

The main CMake project represents the definition of the main performance test application. It defines the name of the binary to be produced and the libraries to be linked to it. The recommended folder where this project should be is inside the `test` folder from a service package (i.e. sdk/core/azure-core/test/performance).

Follow the next template to create the main CMake project
```cmake
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(provide-a-project-name-here LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Name the binary to be created.
add_executable (
  azure-performance-library-name-test
    src/main.cpp
)

# Include the headers from the project.
target_include_directories(
  azure-performance-library-name-test
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-performance-stress` lib together with any other library which will be used for the tests. Below example is using azure-core only.
target_link_libraries(azure-performance-library-name-test PRIVATE azure-core azure-performance-stress)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-performance-library-name-test PROPERTIES FOLDER "Tests")

```

> Notes:
>- Create the `src` and `inc` folders.
>- Add a `main.cpp` file to the `src` folder.

### The `main.cpp`

The main source file defines the list of available tests and calls the performance framework `Run` function. Take a loop to the next example.

```cpp
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// The performance framework headers
#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/program.hpp>

// The test definition headers
#include "azure/performance-stress/test/extended_options.hpp"
#include "azure/performance-stress/test/no_op_test.hpp"

// The utilities
#include <functional>
#include <iostream>
#include <map>

int main(int argc, char** argv)
{

  /**  
   * The test list is a dictionary with the name of the test as a key and
   * an std::function to define how to instantiate a new Test for the PerformanceTest
   * interface.
   * 
   * In the next example, a map called `tests` is init with two test definitions.
   *
   **/
  std::map<
      std::string,
      std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
          Azure::PerformanceStress::TestOptions)>>
      tests{
          {"noOp", // Test Name
           [](Azure::PerformanceStress::TestOptions options) {
             // No Op test 
             return std::make_unique<Azure::PerformanceStress::Test::NoOp>(options);
           }},
          {"extendedOptions", // Test Name
            [](Azure::PerformanceStress::TestOptions options) {
             // Extended options test 
             return std::make_unique<Azure::PerformanceStress::Test::ExtendedOptionsTest>(options);
           }}};

  // Call the `Run` method with a context, the tests and the application arguments to launch the program.
  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}

```

The `Run` method from the performance framework will parse the command line arguments to find out the test name to be run. Then it will try to get that test name from the `tests` map. If the test is found, the framework will get any extra options defined in the test and parse it from the command line arguments. Then it uses the `std::function` to create a as many instances of the test as the `parallel` option value.

In the above code example, the two tests added to the `map` are defined in the project headers. Each test is defined in its own header. See below example for how to define a test.

### Create a performance test

The next code example illustrates how to define a very simple empty test.

```cpp
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define an empty test.
 *
 */

#pragma once

// The performance framework headers
#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>

// Use a namespace according to the service package
namespace Azure { namespace PerformanceStress { namespace Test {

  // Define a derived class from a PerformanceTest
  class NoOp : public Azure::PerformanceStress::PerformanceTest {
  public:

    // Define a constructor to take TestOptions and use the options for the
    // PerformanceTest constructor.
    NoOp(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    // Override the `Run` method with the test definition 
    void Run(Azure::Core::Context const& ctx) override {
      (void)ctx;
    }
  };

}}} // namespace Azure::PerformanceStress::Test

```

### Create a performance test with options

A test can define its own options as an addition to the base options from the performance framework. See the next example to learn how to do it.

```cpp
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

// The performance framework headers
#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>
#include <azure/performance-stress/test_options.hpp>

// Utilities
#include <vector>

namespace Azure { namespace PerformanceStress { namespace Test {

  // The test class definition and constructor looks just the same as any
  // other test (with or without extended options).
  class ExtendedOptionsTest : public Azure::PerformanceStress::PerformanceTest {
  public:
    
    ExtendedOptionsTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    // Override the `GetTestOptions` function to define the unique test options.
    // Do not duplicate activators or test names or the application will fail during parsing.
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      // Each test option is define by 4 properties:
      // - Name (string)
      // - Activators (array of strings which must starts with `-` or `--`)
      // - Description (string that will be use for the help message on console)
      // - Expected Arguments (int to define how many arguments are expected after the activator)
      return {{"extraOption", {"-e"}, "Example for extended option for test.", 1}};
    }

    // Use the `m_options.GetOptionsOrDefault` function inside the test definition to get
    // the test options.
    void Run(Azure::Core::Context const& ctx) override
    {
      // Get a defined test option.
      auto myTestOption = m_options.GetOptionOrDefault("extraOption", 0);
      (void)ctx;
      (void)myTestOption;
    }
  };

}}} // namespace Azure::PerformanceStress::Test

```


## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors  
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/core/performance-stress/LICENSE) license.

<!-- LINKS -->
[azure_sdk_for_c_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md
[azure_sdk_for_c_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#developer-guide
[azure_sdk_for_c_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pull-requests
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash
