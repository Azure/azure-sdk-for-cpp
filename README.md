# Azure SDK for C++

[![Build Status](https://dev.azure.com/azure-sdk/public/_apis/build/status/cpp/cpp%20-%20client%20-%20ci?branchName=main)](https://dev.azure.com/azure-sdk/public/_build/latest?definitionId=1611&branchName=main)
<a href="https://www.surveymonkey.com/r/gh-badge"><img src="https://img.shields.io/badge/Take%20Our%20Design%20Discussion%20Survey-008000"/></a>

This repository is for active development of the Azure SDK for C++. For consumers of the SDK we recommend visiting our [developer docs](https://azure.github.io/azure-sdk-for-cpp).

## Getting started

The Azure SDK for C++ is compatible with a number of different development environments and tools. The following instructions will utilize [Visual Studio](https://visualstudio.microsoft.com/) or [VSCode](https://code.visualstudio.com/) as the IDE, [CMake](https://cmake.org/) for build automation, and [vcpkg](https://vcpkg.io/) as our package manager.

### Prerequisites

- An Azure subscription. Sign up for a [free trial](https://azure.microsoft.com/pricing/free-trial/) or use your [MSDN subscriber benefits](https://azure.microsoft.com/pricing/member-offers/msdn-benefits-details/).
- [A C++ compiler](https://code.visualstudio.com/docs/languages/cpp#_install-a-compiler).

### Development environment and tools set up

- [Visual Studio, CMake and vcpkg](https://learn.microsoft.com/vcpkg/get_started/get-started-vs/)
- [VS Code, CMake and vcpkg](https://learn.microsoft.com/vcpkg/get_started/get-started-vscode/)

### Install libraries

- Open a terminal
  - **Visual Studio:** Open the Developer Command Prompt: **Tools > Commandline > Developer Command Prompt**
  - **VSCode:** Open a new Terminal in VSCode: **Terminal > New Terminal**
- Add the `azure-identity-cpp` and `azure-storage-blobs-cpp` libraries with the following command:

```console
vcpkg add port azure-identity-cpp azure-storage-blobs-cpp
```

- Your `vcpkg.json` should now contain:

```json
{
    "dependencies": [
        "azure-identity-cpp",
        "azure-storage-blobs-cpp"
    ]
}
```

### Configure CMake project files

- Replace the contents of `CMakeLists.txt` with the following:

```cmake
cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)

add_executable(HelloWorld helloworld.cpp)

target_link_libraries(HelloWorld PRIVATE Azure::azure-identity Azure::azure-storage-blobs)
```

### Installing Azure SDK packages in a Visual Studio MSBuild (.vcxproj) project

Here's a walkthrough video on how to install the Azure SDK packages, using vcpkg, into an MSBuild project in VS: https://aka.ms/azsdk/cpp/gettingstarted-vcpkg-msbuild-video

See the [vcpkg documentation](https://learn.microsoft.com/vcpkg/get_started/get-started-msbuild?pivots=shell-cmd) for more details.

### Additional methods for installing and configuring

<!-- Commenting out for now until we know if something similar already exists or if we have to create it. -->
<!-- - [Get started by cloning a sample project]() -->
- [CMake project and fetch content](https://github.com/Azure/azure-sdk-for-cpp/tree/main/samples/integration/cmake-fetch-content/)
- [How to use beta packages](https://github.com/Azure/azure-sdk-for-cpp/tree/main/samples/integration/beta-packages-vcpkg/)

### Using the SDK within your application

The **entry point** for most scenarios when using the SDK will be a top-level client type corresponding to the Azure service. For example, sending requests to blob storage can be done via the `Azure::Storage::Blobs::BlobClient` API. All APIs on the client type send HTTP requests to the cloud service and return back an HTTP `Response<T>`.

Azure C++ SDK headers needed are located within the `<azure>` folder, with sub-folders corresponding to each service. Similarly, all types and APIs can be found within the `Azure::` namespace. For example, to use functionality from `Azure::Core`, include the following header at the beginning of your application `#include <azure/core.hpp>`.

Here's an example application to help you get started:

```cpp
#include <iostream>

// Include the necessary SDK headers
#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>

// Add appropriate using namespace directives
using namespace Azure::Storage;
using namespace Azure::Storage::Blobs;

// Secrets should be stored & retrieved from secure locations such as Azure::KeyVault. For
// convenience and brevity of samples, the secrets are retrieved from environment variables.
std::string GetEndpointUrl() { return std::getenv("AZURE_STORAGE_ACCOUNT_URL"); }
std::string GetAccountName() { return std::getenv("AZURE_STORAGE_ACCOUNT_NAME"); }
std::string GetAccountKey() { return std::getenv("AZURE_STORAGE_ACCOUNT_KEY"); }

int main()
{
  std::string endpointUrl = GetEndpointUrl();
  std::string accountName = GetAccountName();
  std::string accountKey = GetAccountKey();

  try
  {
    auto sharedKeyCredential = std::make_shared<StorageSharedKeyCredential>(accountName, accountKey);

    auto blockBlobClient = BlockBlobClient(endpointUrl, sharedKeyCredential);

    // Create some data to upload into the blob.
    std::vector<uint8_t> data = {1, 2, 3, 4};
    Azure::Core::IO::MemoryBodyStream stream(data);

    Azure::Response<Models::UploadBlockBlobResult> response = blockBlobClient.Upload(stream);

    Models::UploadBlockBlobResult model = response.Value;
    std::cout << "Last modified date of uploaded blob: " << model.LastModified.ToString()
              << std::endl;
  }
  catch (const Azure::Core::RequestFailedException& e)
  {
    std::cout << "Status Code: " << static_cast<int>(e.StatusCode)
              << ", Reason Phrase: " << e.ReasonPhrase << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}
```

### Build and run the project

- **Visual Studio:** Press `Ctrl+Shift+B` to build the project in Visual Studio. Then click the run play button.
- **VSCode:** Open the Command Palette with `Ctrl+Shift+P` and run the `CMake: Build` command. Select the `default` CMake preset. Then launch the project.

## Key Core concepts

Understanding the key concepts from the `Azure Core` library, which is leveraged by all client libraries is helpful in getting started, regardless of which Azure service you want to use.

The main shared concepts of `Azure Core` include:

- Accessing HTTP response details for the returned model of any SDK client operation, via `Response<T>`.
- Exceptions for reporting errors from service requests in a consistent fashion via the base exception type `RequestFailedException`.
- Abstractions for Azure SDK credentials (`TokenCredential`).
- Handling streaming data and input/output (I/O) via `BodyStream` along with its derived types.
- Polling long-running operations (LROs), via `Operation<T>`.
- Collections are returned via `PagedResponse<T>`.
- HTTP pipeline and HTTP policies such as retry and logging, which are configurable via service client specific options.
- Replaceable HTTP transport layer to send requests and receive responses over the network.

#### `Response <T>` Model Types

Many client library operations **return** the templated `Azure::Core::Response<T>` type from the API calls. This type let's you get the raw HTTP response from the service request call the Azure service APIs make, along with the result of the operation to get more API specific details. This is the templated `T` operation result which can be extracted from the response, using the `Value` field.

```cpp
  // Azure service operations return a Response<T> templated type.
  Azure::Response<Models::BlobProperties> propertiesResponse = blockBlobClient.GetProperties();

  // You can get the T, from the returned Response<T>,
  // which is typically named with a Result suffix in the type name.
  Models::BlobProperties propertiesModel = propertiesResponse.Value;

  // Now you can look at API specific members on the result object that is returned.
  std::cout << "The size of the blob is: " << propertiesModel.BlobSize << std::endl;
```

#### Long Running Operations

Some operations take a long time to complete and require polling for their status. Methods starting long-running operations return `Operation<T>` types.

You can intermittently poll whether the operation has finished by using the `Poll()` method inside a loop on the returned `Operation<T>` and track progress of the operation using `Value()`, while the operation is not done (using `IsDone()`). Your per-polling custom logic can go in that loop, such as logging progress. Alternatively, if you just want to wait until the operation completes, you can use `PollUntilDone()`.

```cpp
  std::string sourceUri = "<a uri to the source blob to copy>";

  // Typically, long running operation APIs have names that begin with Start.
  StartBlobCopyOperation operation = blockBlobClient.StartCopyFromUri(sourceUri);

  // Waits for the operation to finish, checking for status every 1 second.
  auto copyResponse = operation.PollUntilDone(std::chrono::milliseconds(1000));
  auto propertiesModel = copyResponse.Value;

  // Now you can look at API specific members on the result object that is returned.
  if (propertiesModel.CopySource.HasValue())
  {
    std::cout << "The source of the copied blob is: " << propertiesModel.CopySource.Value()
              << std::endl;
  }
```

#### `Azure::Core::Context`

Most Azure SDK Service Client methods accept an optional `Azure::Core::Context` parameter, which is used to enable cancellation of the operation or to
establish an absolute deadline for the operation.

This is useful when you want to assign a time limit on an operation to ensure that it completes in a "reasonable" timeframe. For instance, the 
snippet below will cancel a blob client upload after 5 seconds.

<!-- @insert_snippet: CreateBlobContext -->
```cpp
      Azure::Core::Context cancelledIn5s{
          std::chrono::system_clock::now() + std::chrono::seconds(5)};

      auto containerClient = BlobContainerClient::CreateFromConnectionString(
          GetConnectionString(), containerName + std::to_string(i));
      containerClient.CreateIfNotExists({}, cancelledIn5s);
      for (int j = 0; j < 3; ++j)
      {
        BlockBlobClient blobClient
            = containerClient.GetBlockBlobClient(blobName + std::to_string(j));
        blobClient.UploadFrom(
            reinterpret_cast<const uint8_t*>(blobContent.data()),
            blobContent.size(),
            {},
            cancelledIn5s);
      }
```

`Context` objects can also be directly cancelled using the `Cancel()` method.

`Context` objects form a directed tree, where a child context can be created from a parent context. 
The context tree is unidirectional and acyclic.

These are the basic operations that can be performed on a `Context` object:

* Create a child context from a parent context with a Key/Value pair. This is useful for associating metadata with a context.
* Create a child context from a parent context with a deadline. This is useful for setting a timeout.
* Cancel a context. This will cancel the context and all its children. Note that there is no way of un-cancelling a context.
* Check if a context is cancelled.

When a context is copied from another context, the copied context will share state with the original context. 
This means that if the original context is cancelled, the copied context will also be cancelled.

Cancellation of a `Context` is a permanent operation. Once a context is cancelled, it cannot be un-cancelled.

When a client operation fails with an `Azure::Core::OperationCancelledException`, it is typically due to a `Context` getting cancelled. This exception can be caught and handled by the application.

#### Public, Private, and Internal Types

For the most part, the APIs defined in the Azure SDK for C++ fall into three categories:

- **Public**: These are the types that are intended to be used by the consumers of the SDK.
They are part of the public API and are stable. Breaking changes to these types will be avoided as much
as possible. All public types and functions are located are in the `Azure` root namespace.
- **Internal**: These are the types that are used internally by the packages within the SDK.
They are intended for use by the packages which make up the Azure SDK. They are NOT intended to be used by the consumers of the SDK.
Breaking changes to these types are allowed, within certain constraints. These types are located in an `Azure` namespace within the `_internal` terminal namespace (for instance, `Azure::Core::Http::Policies::_internal::RequestActivityPolicy`).
- **Private**: These are the types that are used internally to individual Azure SDK Packages.
They are not intended to be used by the consumers of the SDK, nor by other SDK packages. Breaking changes
to these types are allowed. These types are located in an `Azure` namespace within the `_detail` terminal namespace.

Within the source tree, Internal types are typically declared in directories named "internal", and Private types are typically declared in directories named "private".

#### Interacting with Azure SDK for C++

Static SDK members should not be accessed and SDK functions should not be called before the static initialization phase is finished.

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

Each service might have a number of libraries available. These libraries follow the [Azure SDK Design Guidelines for C++](https://azure.github.io/azure-sdk/cpp_introduction.html) and share a number of core features such as HTTP retries, logging, transport protocols, authentication protocols, etc., so that once you learn how to use these features in one client library, you will know how to use them in other client libraries. You can learn about these shared features at [Azure::Core](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core/README.md).

The client libraries can be identified by the naming used for their folder, package, and namespace. Each will start with `azure`, followed by the service category, and then the name of the service. For example `azure-storage-blobs`.

For a complete list of available packages, please see the [latest available packages](https://azure.github.io/azure-sdk/releases/latest/#c) page.

> NOTE: If you need to ensure your code is ready for production we strongly recommend using one of the stable, non-beta libraries.

### Vcpkg

The following SDK library releases are available on [vcpkg](https://github.com/microsoft/vcpkg):

- `azure-core-cpp`
- `azure-identity-cpp`
- `azure-security-attestation-cpp`
- `azure-security-keyvault-certificates-cpp`
- `azure-security-keyvault-keys-cpp`
- `azure-security-keyvault-secrets-cpp`
- `azure-storage-blobs-cpp`
- `azure-storage-files-datalake-cpp`
- `azure-storage-files-shares-cpp`
- `azure-storage-queues-cpp`

> NOTE: In case of getting linker errors when consuming the SDK on Windows, make sure that [vcpkg triplet](https://github.com/microsoft/vcpkg-docs/blob/main/vcpkg/users/triplets.md) being consumed matches the [CRT link flags](https://learn.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160) being set for your app or library build. See also `MSVC_USE_STATIC_CRT` build flag.

## OpenSSL Version

Several packages within the Azure SDK for C++ use the OpenSSL library. By default, the Azure SDK will use whatever the most recent version of OpenSSL is within the VCPKG repository.

### Using a specific version of OpenSSL

If you need to use a specific version of OpenSSL, you can use the vcpkg custom ports feature to specify the version of OpenSSL to use.
For example, if you want to use OpenSSL 1.1.1, you should create a folder named `vcpkg-custom-ports` next to to your vcpkg.json file.

Navigate to your clone of the vcpkg vcpkg repo and execute "git checkout 3b3bd424827a1f7f4813216f6b32b6c61e386b2e" - this will reset your repo to the last version of OpenSSL 1.1.1
in vcpkg. Then, copy the contents of the `ports/openssl` folder from the vcpkg repo to the `vcpkg-custom-ports` folder you created earlier:

```sh
cd <your vcpkg repo>
git checkout 3b3bd424827a1f7f4813216f6b32b6c61e386b2e
cd ports
cp -r openssl <the location of the vcpkg-custom-ports directory listed above>
```

This will copy the port information for OpenSSL 1.1.1n to your vcpkg-custom-ports directory.

Once that is done, you can install the custom port of OpenSSL 1.1.1n using the vcpkg tool:

```sh
vcpkg install --overlay-ports=<path to the vcpkg-custom-ports above>
```

If you are building using CMAKE, you can instruct CMAKE to apply the overlay ports using the following command line switches:

```sh
cmake -DVCPKG_MANIFEST_MODE=ON -DVCPKG_OVERLAY_PORTS=<path to the vcpkg-custom-ports above> -DVCPKG_MANIFEST_DIR=<path to the directory containing the vcpkg.json file>
```

In addition, if you need to consume OpenSSL from a dynamic linked library/shared object, you can set the VCPKG triplet to reflect that you want to build the library with dynamic entries. Set the VCPKG_you can set the environment variable to `x64-windows-static` or `x64-windows-dynamic` depending on whether you want to use the static or dynamic version of OpenSSL.
Similarly you can use the x64-linux-dynamic and x64-linux-static triplet to specify consumption of libraries as a shared object or dynamic.

### Using the system package manager to install OpenSSL

If you are using a Linux distribution that uses the system package manager to install libraries, you can use the system package
manager to install OpenSSL.

The vcpkg team has a [feature](https://devblogs.microsoft.com/cppblog/using-system-package-manager-dependencies-with-vcpkg/)
which allows you to use the system package manager to install dependencies.

## Need help

- For reference documentation visit the [Azure SDK for C++ documentation](https://azure.github.io/azure-sdk-for-cpp).
- For tutorials, samples, quick starts and other documentation, visit [Azure for C++ Developers](https://learn.microsoft.com/azure/).
- File an issue via [GitHub Issues](https://github.com/Azure/azure-sdk-for-cpp/issues/new/choose).

## Navigating the repository

### Main branch

The main branch has the most recent code with new features and bug fixes. It does **not** represent latest released **beta** or **GA** SDK.

### Release branches (Release tagging)

For each package we release there will be a unique Git tag created that contains the name and the version of the package to mark the commit of the code that produced the package. This tag will be used for servicing via hotfix branches as well as debugging the code for a particular beta or stable release version.
Format of the release tags are `<package-name>_<package-version>`. For more information please see [our branching strategy](https://github.com/Azure/azure-sdk/blob/main/docs/policies/repobranching.md#release-tagging).

## Contributing

For details on contributing to this repository, see the [contributing guide](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md).

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, view [Microsoft's CLA](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors

Many people all over the world have helped make this project better.  You'll want to check out:

- [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
- [How to build and test your change](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide)
- [How you can make a change happen!](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests)
- Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

### Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow [Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/legal/intellectualproperty/trademarks/usage/general). Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those third-party's policies.

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2FREADME.png)

