# Azure SDK for C++

[![Build Status](https://dev.azure.com/azure-sdk/public/_apis/build/status/cpp/cpp%20-%20client%20-%20ci?branchName=master)](https://dev.azure.com/azure-sdk/public/_build/latest?definitionId=1611&branchName=master)

This repository is for active development of the Azure SDK for C++. For consumers of the SDK we recommend visiting our [public developer docs](https://azure.github.io/azure-sdk-for-cpp).

## Getting started

For the best development experience, we recommend that developers use the [CMake projects in Visual Studio](https://docs.microsoft.com/cpp/build/cmake-projects-in-visual-studio?view=vs-2019) to view and build the source code together with its dependencies. You can also use any other text editor of your choice, such as [VS Code](https://code.visualstudio.com/), along with the command line for building your application with the SDK.

You can find additional information for specific libraries by navigating to the appropriate folder in the `/sdk` directory. See the **README.md** file located in the library's project folder. For example, [here is the doc for the storage client library](https://github.com/Azure/azure-sdk-for-cpp/tree/master/sdk/storage#azure-storage-client-library-for-c).

For API reference docs, tutorials, samples, quick starts, and other documentation, go to [Azure for C++ Developer Docs](https://azure.github.io/azure-sdk-for-cpp).

### Download & Install the SDK

The easiest way to get the C++ SDK is to leverage [vcpkg](https://github.com/microsoft/vcpkg). You'll need to have [Git](https://git-scm.com/downloads) installed before getting started.

First clone and bootstrap vcpkg itself. You can install it anywhere on your machine, but **make note** of the directory where you clone the vcpkg repo.

```cmd
> git clone https://github.com/microsoft/vcpkg
```

On Windows:

```cmd
> .\vcpkg\bootstrap-vcpkg.bat
```

On Linux:

```sh
> ./vcpkg/bootstrap-vcpkg.sh
```

To install the libraries for your project, run the following, optionally specifying the triplet:

```cmd
> .\vcpkg\vcpkg install [packages to install]
```

For example, this will install the `x64-windows` triplet. On Windows, not specifying a triplet will default to x86-windows:

```cmd
> .\vcpkg\vcpkg install azure-storage-blobs-cpp:x64-windows
```

See the [list of packages](https://github.com/Azure/azure-sdk-for-cpp#vcpkg) available via vcpkg below. All Azure C++ SDK package names start with `azure-`.

You can also search for the libraries you need with the `search` subcommand:

```cmd
> .\vcpkg\vcpkg search [search term]
```

For example:

```cmd
> .\vcpkg\vcpkg search azure-
```

Once the library is installed, follow the instructions from the console output to include the library in your `CMake` application. For example, to include `azure-storage-blobs-cpp`, add the following to your `CMakeLists.txt` file:

```CMake
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
target_link_libraries(myProject PRIVATE Azure::azure-storage-blobs)
```

> NOTE: All the Azure client libraries take a dependency on `azure-core-cpp` which provides functionality commonly needed by all Azure clients. When you install any client library via vcpkg, it will bring in all the necessary dependencies as well. You don't need to install those individually to get started.

You can reference this [vcpkg Quick Start](https://github.com/microsoft/vcpkg#quick-start-windows) for more details.

### Building your Application

In order to use the SDK installed via vcpkg with CMake, you can use the toolchain file from vcpkg:

```cmd
> cmake -B [build directory] -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg repo]/scripts/buildsystems/vcpkg.cmake
> cmake --build [build directory]
```

The **entry point** for most scenarios when using the SDK will be a top-level client type corresponding to the Azure service you want to talk to. For example, sending requests to blob storage can be done via the `Azure::Storage::Blobs::BlobClient` API.

All the Azure C++ SDK headers needed to be included are located within the `<azure>` folder, with sub-folders corresponding to each service. Similarly, all types and APIs can be found within the `Azure::` namespace. For example, to use functionality form `Azure::Core`, include the following header at the beginning of your application `#include <azure/core.hpp>`.

Here's an example application to help you get started:

```C++
#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>

using namespace Azure::Storage::Blobs;

int main()
{
	std::string containerName = "testcontainer";
	std::string blobName = "sample-blob";

	try
	{
		auto client = BlobClient::CreateFromConnectionString(
            GetConnectionString(), containerName, blobName);
	}
	catch (Azure::Core::RequestFailedException& e)
	{
		puts(e.what());
		return 1;
	}
	return 0;
}
```

#### Visual Studio - CMakeSettings.json

When building your application via Visual Studio, you can create and update a `CMakeSettings.json` file and include the following properties to let Visual Studio know where the packages are installed and which triplet needs to be used:

```json
{
  "configurations": [
    {
        "cmakeToolchain": "<path to vcpkg repo>/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "variables": [
        {
          "name": "VCPKG_TARGET_TRIPLET",
          "value": "x64-windows",
          "type": "STRING"
        }
      ]
    }
  ]
}
```

### Azure Requirements

To call Azure services, you must first have an Azure subscription. Sign up for a [free trial](https://azure.microsoft.com/pricing/free-trial/) or use your [MSDN subscriber benefits](https://azure.microsoft.com/pricing/member-offers/msdn-benefits-details/).

## Packages available

Each service might have a number of libraries available. These libraries follow the [Azure SDK Design Guidelines for C++](https://azure.github.io/azure-sdk/cpp_introduction.html) and share a number of core features such as HTTP retries, logging, transport protocols, authentication protocols, etc., so that once you learn how to use these features in one client library, you will know how to use them in other client libraries. You can learn about these shared features at [Azure::Core](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/core/azure-core/README.md).

The client libraries can be identified by the naming used for their folder, package, and namespace. Each will start with `azure`, followed by the service category, and then the name of the service. For example `azure-storage-blobs`.

For a complete list of available packages, please see the [latest available packages](https://azure.github.io/azure-sdk/releases/latest/#c) page.

> NOTE: If you need to ensure your code is ready for production we strongly recommend using one of the stable, non-beta libraries.

### Vcpkg

The following SDK library releases are available on [Vcpkg](https://github.com/microsoft/vcpkg):

* `azure-core-cpp`
* `azure-identity-cpp`
* `azure-storage-blobs-cpp`
* `azure-storage-common-cpp`
* `azure-storage-files-datalake-cpp`
* `azure-storage-files-shares-cpp`

> NOTE: In case of getting linker errors when consuming the SDK on Windows, make sure that [Vcpkg triplet](https://vcpkg.readthedocs.io/en/latest/users/triplets/) being consumed matches the [CRT link flags](https://docs.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160) being set for your app or library build. See also `MSVC_USE_STATIC_CRT` build flag.

## Need help

- For reference documentation visit the [Azure SDK for C++ documentation](https://azure.github.io/azure-sdk-for-cpp).
- For tutorials, samples, quick starts and other documentation, visit [Azure for C++ Developers](https://docs.microsoft.com/azure/).
- File an issue via [Github Issues](https://github.com/Azure/azure-sdk-for-cpp/issues/new/choose).

## Navigating the repository

### Master branch

The master branch has the most recent code with new features and bug fixes. It does **not** represent latest released **beta** or **GA** SDK.

### Release branches (Release tagging)

For each package we release there will be a unique git tag created that contains the name and the version of the package to mark the commit of the code that produced the package. This tag will be used for servicing via hotfix branches as well as debugging the code for a particular beta or stable release version.
Format of the release tags are `<package-name>_<package-version>`. For more information please see [our branching strategy](https://github.com/Azure/azure-sdk/blob/master/docs/policies/repobranching.md#release-tagging).

## Contributing

For details on contributing to this repository, see the [contributing guide](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md).

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, view [Microsoft's CLA](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors

Many people all over the world have helped make this project better.  You'll want to check out:

- [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
- [How to build and test your change](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#developer-guide)
- [How you can make a change happen!](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pull-requests)
- Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/master/LICENSE.txt) license.

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2FREADME.png)
