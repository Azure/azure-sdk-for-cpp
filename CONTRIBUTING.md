Azure SDK for C++ Contributing Guide
-------------------------------------

Thank you for your interest in contributing to Azure SDK for C++.

- For reporting bugs, requesting features, or asking for support, please file an issue in the [issues](https://github.com/Azure/azure-sdk-for-cpp/issues) section of the project.

- If you would like to become an active contributor to this project please follow the instructions provided in [Microsoft Azure Projects Contribution Guidelines](https://azure.github.com/guidelines.html).

- To make code changes, or contribute something new, please follow the [GitHub Forks / Pull requests model](https://help.github.com/articles/fork-a-repo/): Fork the repo, make the change and propose it back by submitting a pull request.

- Refer to the [wiki](https://github.com/Azure/azure-sdk-for-c/wiki) to learn about how Azure SDK for C generates lint checker, doxygen, and code coverage reports.

Pull Requests
-------------

- **DO** follow the API design and implementation [C++ Guidelines](https://azure.github.io/azure-sdk/cpp_introduction.html).
  - When submitting large changes or features, **DO** have an issue or spec doc that describes the design, usage, and motivating scenario.
- **DO** submit all code changes via pull requests (PRs) rather than through a direct commit. PRs will be reviewed and potentially merged by the repo maintainers after a peer review that includes at least one maintainer.
- **DO** review your own PR to make sure there aren't any unintended changes or commits before submitting it.
- **DO NOT** submit "work in progress" PRs. A PR should only be submitted when it is considered ready for review and subsequent merging by the contributor.
  - If the change is work-in-progress or an experiment, **DO** start if off as a temporary draft PR.
- **DO** give PRs short-but-descriptive names (e.g. "Improve code coverage for Azure.Core by 10%", not "Fix #1234") and add a description which explains why the change is being made.
- **DO** refer to any relevant issues, and include [keywords](https://help.github.com/articles/closing-issues-via-commit-messages/) that automatically close issues when the PR is merged.
- **DO** tag any users that should know about and/or review the change.
- **DO** ensure each commit successfully builds.  The entire PR must pass all tests in the Continuous Integration (CI) system before it'll be merged.
- **DO** address PR feedback in an additional commit(s) rather than amending the existing commits, and only rebase/squash them when necessary.  This makes it easier for reviewers to track changes.
- **DO** assume that ["Squash and Merge"](https://github.com/blog/2141-squash-your-commits) will be used to merge your commit unless you request otherwise in the PR.
- **DO NOT** mix independent, unrelated changes in one PR. Separate real product/test code changes from larger code formatting/dead code removal changes. Separate unrelated fixes into separate PRs, especially if they are in different modules or files that otherwise wouldn't be changed.
- **DO** comment your code focusing on "why", where necessary. Otherwise, aim to keep it self-documenting with appropriate names and style.
- **DO** add [doxygen style API comments](https://azure.github.io/azure-sdk/cpp_documentation.html#docstrings) when adding new APIs or modifying header files.
- **DO** make sure there are no typos or spelling errors, especially in user-facing documentation.
- **DO** verify if your changes have impact elsewhere. For instance, do you need to update other docs or exiting markdown files that might be impacted?
- **DO** add relevant unit tests to ensure CI will catch future regressions.

Merging Pull Requests (for project contributors with write access)
----------------------------------------------------------

* **DO** use ["Squash and Merge"](https://github.com/blog/2141-squash-your-commits) by default for individual contributions unless requested by the PR author.
  Do so, even if the PR contains only one commit. It creates a simpler history than "Create a Merge Commit".
  Reasons that PR authors may request "Merge and Commit" may include (but are not limited to):

  - The change is easier to understand as a series of focused commits. Each commit in the series must be buildable so as not to break `git bisect`.
  - Contributor is using an e-mail address other than the primary GitHub address and wants that preserved in the history. Contributor must be willing to squash
    the commits manually before acceptance.



## Developer Guide

### Pre-requisites

#### CMake
CMake version 3.13 or higher is required to build these libraries. Download and install CMake from the project's
[website](https://cmake.org/download/).

#### Third Party Dependencies
- curl
- libxml2

Vcpkg can be used to install the Azure SDK for CPP dependencies into a specific folder on the system instead of globally installing them.
Follow [vcpkg install guide](https://github.com/microsoft/vcpkg#getting-started) to get vcpkg and install the following dependencies:

```sh
./vcpkg install curl libxml2
```

When using vcpkg, make sure to set the `VCPKG_ROOT` environment variable to the vcpkg Git repository folder before using `CMake`.

The Azure SDK for C++ uses [this vcpkg release version](https://github.com/Azure/azure-sdk-for-cpp/blob/master/eng/vcpkg-commit.txt) for continuos integration (CI) building and testing. Make sure to checkout this version when following the next steps for building and running the Azure SDK for C++. Using a newer vcpkg version might still work, however, if it is tested.

```sh
# Checking out vcpkg release version before installing dependencies

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
# Checkout the vcpkg commit from the vcpkg-commit.txt file (link above)
git checkout <vcpkg commit>

# build vcpkg (showing linux command, see vcpkg getting started for windows)
./bootstrap-vcpkg.sh
./vcpkg install curl libxml2
```
 
### Building and Testing

#### Building the project
First, ensure that the `VCPKG_ROOT` environment variable is set, as described [above](#vcpkg). This needs to be defined
any time you want to build using vcpkg. Then generate the build files and build as you would with any standard CMake project. From the
repo root, run:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

#### CMake build options
The following CMake options are available for adding/removing project features.

<table>
<tr>
<td>Option</td>
<td>Description</td>
<td>Default Value</td>
</tr>
<tr>
<td>BUILD_TESTING</td>
<td>Generates Unit Test for compilation. CMake will automatically download and build g-test.<br>After Compiling, use `ctest` to run Unit Test.</td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_CODE_COVERAGE</td>
<td>Build HTML and XML targets for each package which can be call to produce XML or HTML reports. The generated CMake targets are named `package-name_cov_xml` and `package-name_cov_html` (for example, for Azure Core, it would be `azure-core-cov_xml`).<br> <br>Ths option requires compiling on `debug` mode, building tests (BUILD_TESTING) and a GNU compiler like gcc. </td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_STORAGE_SAMPLES</td>
<td>Build Azure Storage clients sample application.</td>
<td>OFF</td>
</tr>
<tr>
<td>RUN_LONG_UNIT_TESTS</td>
<td>Enables the special unit tests which takes more than 3 minutes to run. THis tests are for some specific features like the connection pool for curl transport adapter.</td>
<td>OFF</td>
</tr>
<tr>
<td>WARNINGS_AS_ERRORS</td>
<td>Warnings will make compiling fail</td>
<td>ON</td>
</tr>
<tr>
<td>BUILD_TRANSPORT_CURL</td>
<td>Build the curl http transport adapter. When building on Posix systems, if no other transport adapter is built, this option will be automatically turned ON</td>
<td>OFF</td>
</tr>
<tr>
<td>BUILD_DOCUMENTATION</td>
<td>Build Doxygen documentation</td>
<td>OFF</td>
</tr>
</table>

#### Testing the project
If you want to run tests also, generate build files using below command and then build.
```sh
cmake -DBUILD_TESTING=ON ..
cmake --build .
```
Tests are executed via the `ctest` command included with CMake. From the build directory, run:

```sh
# use -V for verbose
ctest -V
# Use -N to list test that are part of test set
ctest -N
# Use -R to use a regular exp for what to run
ctest -R Http # runs only Http tests
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

If the coverage data has been previously generated (for example, if you manually run the unit tests), you can define `AZURE_CI_TEST` environment variable (set it to any value) and then the report will be generated without running the tests again. This is how the coverage reports are generated on CI, where the tests runs prior to code coverage step. 

### Visual Studio 2019
You can also build the project by simply opening the repo directory in Visual Studio. Visual Studio will detect the `CMake` file and will configure itself to generate, build and run tests.
