# Azure SDK for C++ Contributing Guide

Thank you for your interest in contributing to Azure SDK for C++.

- For reporting bugs, requesting features, or asking for support, please file an issue in the [issues](https://github.com/Azure/azure-sdk-for-cpp/issues) section of the project.

- If you would like to become an active contributor to this project please follow the instructions provided in [Microsoft Azure Projects Contribution Guidelines](https://azure.github.io/azure-sdk/policies_opensource.html).

- To make code changes, or contribute something new, please follow the [GitHub Forks / Pull requests model](https://docs.github.com/articles/fork-a-repo/): Fork the repo, make the change and propose it back by submitting a pull request.

## Pull Requests

- **DO** follow the API design and implementation [C++ Guidelines](https://azure.github.io/azure-sdk/cpp_introduction.html).
  - When submitting large changes or features, **DO** have an issue or spec doc that describes the design, usage, and motivating scenario.
- **DO** submit all code changes via pull requests (PRs) rather than through a direct commit. PRs will be reviewed and potentially merged by the repo maintainers after a peer review that includes at least one maintainer.
- **DO** review your own PR to make sure there aren't any unintended changes or commits before submitting it.
- **DO NOT** submit "work in progress" PRs. A PR should only be submitted when it is considered ready for review and subsequent merging by the contributor.
  - If the change is work-in-progress or an experiment, **DO** start it off as a temporary draft PR.
- **DO** give PRs short-but-descriptive names (e.g. "Improve code coverage for Azure.Core by 10%", not "Fix #1234") and add a description which explains why the change is being made.
- **DO** refer to any relevant issues, and include [keywords](https://docs.github.com/articles/closing-issues-via-commit-messages/) that automatically close issues when the PR is merged.
- **DO** tag any users that should know about and/or review the change.
- **DO** ensure each commit successfully builds. The entire PR must pass all tests in the Continuous Integration (CI) system before it'll be merged.
- **DO** address PR feedback in an additional commit(s) rather than amending the existing commits, and only rebase/squash them when necessary. This makes it easier for reviewers to track changes.
- **DO** assume that ["Squash and Merge"](https://github.com/blog/2141-squash-your-commits) will be used to merge your commit unless you request otherwise in the PR.
- **DO NOT** mix independent, unrelated changes in one PR. Separate real product/test code changes from larger code formatting/dead code removal changes. Separate unrelated fixes into separate PRs, especially if they are in different modules or files that otherwise wouldn't be changed.
- **DO** comment your code focusing on "why", where necessary. Otherwise, aim to keep it self-documenting with appropriate names and style.
- **DO** add [doxygen style API comments](https://azure.github.io/azure-sdk/cpp_introduction.html#documentation-style) when adding new APIs or modifying header files.
- **DO** make sure there are no typos or spelling errors, especially in user-facing documentation.
- **DO** verify if your changes have impact elsewhere. For instance, do you need to update other docs or exiting markdown files that might be impacted?
- **DO** add relevant unit tests to ensure CI will catch future regressions.

## Merging Pull Requests (for project contributors with write access)

- **DO** use ["Squash and Merge"](https://github.com/blog/2141-squash-your-commits) by default for individual contributions unless requested by the PR author.
  Do so, even if the PR contains only one commit. It creates a simpler history than "Create a Merge Commit".
  Reasons that PR authors may request "Merge and Commit" may include (but are not limited to):

  - The change is easier to understand as a series of focused commits. Each commit in the series must be buildable so as not to break `git bisect`.
  - Contributor is using an e-mail address other than the primary GitHub address and wants that preserved in the history. Contributor must be willing to squash
    the commits manually before acceptance.

# Developer Guide

## Codespaces

Codespaces is new technology that allows you to use a container as your development environment. This repo provides a Codespaces container which is supported by both GitHub Codespaces and VS Code Codespaces.

### GitHub Codespaces

1. From the Azure SDK GitHub repo, click on the "Code -> Open with Codespaces" button.
1. Open a Terminal. The development environment will be ready for you. Continue to [Building the project](#building-the-project).

### VS Code Codespaces

1. Install the [VS Code Remote Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack)
1. When you open the Azure SDK for C++ repo in VS Code, it will prompt you to open the project in the Dev Container. If it does not prompt you, then hit CTRL+P, and select "Remote-Containers: Open Folder in Container..."
1. Open a Terminal. The development environment will be ready for you. Continue to [Building the project](#building-the-project).

## Full Local Setup

### Pre-requisites

#### CMake

CMake version 3.13 or higher is required to build these libraries. Download and install CMake from the project's
[website](https://cmake.org/download/).

### Dotnet

Required to get and execute test proxy (see [/doc/TestProxy.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/TestProxy.md)). Download the dotnet CLI from the project's [website](https://dotnet.microsoft.com/download).

### Third Party Dependencies

Azure SDK uses Vcpkg manifest mode to declare the [list of required 3rd party dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/main/vcpkg.json) for building the SDK service libraries. It will also get and set up Vcpkg automatically. **You can move on to [Building the project](#building-the-project)** and skip the next part if you are not interested in learning about alternatives for setting up dependencies.

#### Customize the Vcpkg dependency integration

If the CMake option _-DCMAKE_TOOLCHAIN_FILE=..._ is not defined to generate the project, the Azure SDK project will automatically get Vcpkg and link it to get its dependencies. You can use the next environment variables to change this behavior:

<center>

<table>
<tr>
<td>Environment Variable</td>
<td>Description</td>
</tr>
<tr>
<td>AZURE_SDK_DISABLE_AUTO_VCPKG</td>
<td>When defined, Vcpkg won't be automatically cloned and linked. Use this setting, for example, if your dependencies are installed on the system and you don't need to get them.</td>
</tr>
<tr>
<td>AZURE_SDK_VCPKG_COMMIT</td>
<td>This variable can set the git commit id to be used when automatically cloning Vcpkg.</td>
</tr>
<tr>
<td>VCPKG_ROOT</td>
<td>Use this variable to set an existing Vcpkg folder from your system to be linked for building. Use this, for example, when working with Vcpkg classic mode, to switch between different Vcpkg folders.</td>
</tr>
</table>

</center>


## Building the project

Generate the CMake files and build as you would with any standard CMake project. From the
repo root, run:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

#### Static Analysis

When the project is built using MSVC on Windows, the compiler can run [static analysis](https://docs.microsoft.com/cpp/code-quality/walkthrough-analyzing-c-cpp-code-for-defects) on the code. The CMake project can add the required compiler flags to perform this check. To enable this feature, set an environment variable `AZURE_ENABLE_STATIC_ANALYSIS`.

Keep in mind that enabling static analysis will significantly impact build time. It is recommended to run it right before submitting the PR, but not in your inner developer loop.

The static code analysis is `ON` for the CI pipelines. You can turn this feature `ON` locally to debug errors reported during CI or for the last time you build and test before creating a new PR.

#### CMake build options

The following CMake options are available for adding/removing project features.

**Note:** These CMake options are meant for contributors to the repo, and not end users.

<table>
<tr>
<td>Option</td>
<td>Description</td>
<td>Default Value</td>
</tr>
<tr>
<td>BUILD_TESTING</td>
<td>Generates Unit Test for compilation. CMake will automatically download and build g-test.<br>After Compiling, use `ctest` to run Unit Test. <br> <strong>Note:</strong> The BUILD_TESTING option is provided only for contributors to this repo to perform unit testing.  The flag should NEVER be enabled in production code and the use of it in such situations is NOT supported. </td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_CODE_COVERAGE</td>
<td>Build HTML and XML targets for each package which can be call to produce XML or HTML reports. The generated CMake targets are named `package-name_cov_xml` and `package-name_cov_html` (for example, for Azure Core, it would be `azure-core-cov_xml`).<br> <br>Ths option requires compiling on `debug` mode, building tests (BUILD_TESTING) and a GNU compiler like gcc. </td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_SAMPLES</td>
<td>Build Azure SDK for C++ sample applications. </td>
<td>OFF</td>
</tr>
<tr>
<td>RUN_LONG_UNIT_TESTS</td>
<td>Enables the special unit tests which takes more than 3 minutes to run. These tests are for some specific features like the connection pool for libcurl transport adapter.</td>
<td>OFF</td>
</tr>
<tr>
<td>WARNINGS_AS_ERRORS</td>
<td>Warnings will make compiling fail</td>
<td>ON</td>
</tr>
<tr>
<td>BUILD_TRANSPORT_CURL</td>
<td>Build the libcurl HTTP transport adapter. When building on POSIX systems, if no other transport adapter is built, this option will be automatically turned ON</td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_TRANSPORT_WINHTTP</td>
<td>Build the WinHTTP transport adapter. When building on Windows systems, if no other transport adapter is built, this option will be automatically turned ON. This option is not supported on non-Windows OSes.</td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_DOCUMENTATION</td>
<td>Build Doxygen documentation</td>
<td>OFF</td>
</tr>
<tr>
<td>MSVC_USE_STATIC_CRT</td>
<td>On MSVC, link SDK with static CRT (use `/MT` or `/MTd` switch)</td>
<td>OFF</td>
</tr>
</table>

#### Testing the project

##### Test Mode

Before running unit tests, you have to decide what is the test mode you want to use. To set the test mode,
use the environment variable `AZURE_TEST_MODE`. See the supported values next:

- LIVE

When setting `AZURE_TEST_MODE=LIVE`, test cases will try to connect to `AZURE` cloud services using the [test environment configuration](#test-environment-configuration). Make sure to set up the environment variables required to run test cases.

This is the default test mode if `AZURE_TEST_MODE` is not even set.

- PLAYBACK

When setting `AZURE_TEST_MODE=PLAYBACK`, test cases will consume pre-recorded data instead of sending requests to an AZURE cloud service. The [test environment configuration](#test-environment-configuration) is still required, but the configuration values won't be relevant. Use this test mode to run tests cases without a network connection.

- RECORD

When setting `AZURE_TEST_MODE=RECORD`, test cases will run the as when running `LIVE`. All the AZURE service network responses are recorded in a json file within the /recordings folder from the /test/ut directory. Use this test mode to generate pre-recorded data to be used on `PLAYBACK` mode.

##### Test Environment Configuration

Some environment variables are expected to be defined for some test binaries. For example, for `azure-storage-blobs-test`, there should be a `STANDARD_STORAGE_CONNECTION_STRING` variable to let tests know how to connect to the Azure Storage account.

Even for running on `PLAYBACK` mode, the env configuration is mandatory. This is because a test case does not know about the test modes. A test case will always look for the environment configuration to connect/authenticate to Azure.

Take a look to [this file](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/ci.yml#L52) which defines the required configuration for each SDK package. Those settings are used to run all unit test on `PLAYBACK` mode on CI, you can use the same settings from that file to run on `PLAYBACK` locally.

##### Test-Proxy 

Recording and playing back tests depends on an external tool called "test-proxy" (see doc/TestProxy.md)
You can start this tool manually or automatically as part of you development flow. 
For automatic start you will need to set the environment variable "AZURE_TEST_USE_TEST_PROXY" to value "ON".

##### Running tests

If you want to run tests, generate build files using below command and then build.

```sh
cmake -DBUILD_TESTING=ON ..
cmake --build .
```

Tests are executed via the `ctest` command included with CMake. After setting a [test mode](#test-mode), from the build directory, run:

```sh
# use -V for verbose
ctest -V
# Use -N to list test that are part of test set
ctest -N
# Use -R to use a regular exp for what to run
ctest -R Http # runs only HTTP tests
ctest -R storage # runs all the azure storage unit tests
```

#### Generating Code Coverage reports

`gcov` and `gcovr` must be installed on your system.
Also, make sure to generate the project with Debug mode. Then, option `-DBUILD_TESTING` must be `ON` and to use a GNU compiler (like gcc).

```sh
# install gcov and gcovr if missing
sudo apt-get install gcov gcovr # example for Linux

cmake -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DBUILD_CODE_COVERAGE=ON ..

# After this, generate reports by calling a package target
make package-name_cov_xml # for example `azure-core_cov_xml`
make package-name_cov_html # for example `azure-core_cov_html`
```

Running the above commands will create the test executable and run it. While it runs, gcov will capture coverage and produce coverage data. And when test finished, gcovr is used to parse the coverage data to produce and XML or HTML. The output files will be inside the package build directory. For example, for Azure core, it will be `../build/path/sdk/core/azure-core/`.

If the coverage data has been previously generated (for example, if you manually run the unit tests), you can define `CODE_COVERAGE_COLLECT_ONLY` environment variable (set it to any value) and then the report will be generated without running the tests again. This is how the coverage reports are generated on CI, where the tests runs prior to code coverage step.

### Visual Studio 2019

You can also build the project by simply opening the repo directory in Visual Studio. Visual Studio will detect the `CMake` file and will configure itself to generate, build and run tests.

## Samples

### Third-party dependencies

Third party libraries should only be included in samples when necessary to demonstrate usage of an Azure SDK package; they should not be suggested or endorsed as alternatives to the Azure SDK.

When code samples take dependencies, readers should be able to use the material without significant license burden or research on terms. This goal requires restricting dependencies to certain types of open source or commercial licenses.

Samples may take the following categories of dependencies:

- **Open-source** : Open source offerings that use an [Open Source Initiative (OSI) approved license](https://opensource.org/licenses). Any component whose license isn't OSI-approved is considered a commercial offering. Prefer OSS projects that are members of any of the [OSS foundations that Microsoft is part of](https://opensource.microsoft.com/ecosystem/). Prefer permissive licenses for libraries, like [MIT](https://opensource.org/licenses/MIT) and [Apache 2](https://opensource.org/licenses/Apache-2.0). Copy-left licenses like [GPLv3](https://opensource.org/license/gpl-3-0) are acceptable for tools, and OSs. [Kubernetes](https://github.com/kubernetes/kubernetes), [Linux](https://github.com/torvalds/linux), and [Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json) are examples of this license type. Links to open source components should be to where the source is hosted, including any applicable license, such as a GitHub repository (or similar).

- **Commercial**: Commercial offerings that enable readers to learn from our content without unnecessary extra costs. Typically, the offering has some form of a community edition, or a free trial sufficient for its use in content. A commercial license may be a form of dual-license, or tiered license. Links to commercial components should be to the commercial site for the software, even if the source software is hosted publicly on GitHub (or similar).

- **Dual licensed**: Commercial offerings that enable readers to choose either license based on their needs. For example, if the offering has an OSS and commercial license, readers can  choose between them. [MySql](https://github.com/mysql/mysql-server) is an example of this license type.

- **Tiered licensed**: Offerings that enable readers to use the license tier that corresponds to their characteristics. For example, tiers may be available for students, hobbyists, or companies with defined revenue  thresholds. For offerings with tiered licenses, strive to limit our use in tutorials to the features available in the lowest tier. This policy enables the widest audience for the article. [Docker](https://www.docker.com/), [IdentityServer](https://duendesoftware.com/products/identityserver), [ImageSharp](https://sixlabors.com/products/imagesharp/), and [Visual Studio](https://visualstudio.com) are examples of this license type.

In general, we prefer taking dependencies on licensed components in the order of the listed categories. In cases where the category may not be well known, we'll document the category so that readers understand the choice that they're making by using that dependency.
