# Azure SDK for C++

[![Build Status](https://dev.azure.com/azure-sdk/public/_apis/build/status/cpp/cpp%20-%20client%20-%20ci?branchName=main)](https://dev.azure.com/azure-sdk/public/_build/latest?definitionId=1611&branchName=main)

This repository is for active development of the Azure SDK for C++. For consumers of the SDK we recommend visiting our [developer docs](https://azure.github.io/azure-sdk-for-cpp).

## Getting started

For the best development experience, we recommend developers use [CMake projects in Visual Studio](https://docs.microsoft.com/cpp/build/cmake-projects-in-visual-studio?view=vs-2019) to view and build the source code together with its dependencies. You can also use any other text editor of your choice, such as [VS Code](https://code.visualstudio.com/), along with the command line for building your application with the SDK.

You can find additional information for specific libraries by navigating to the appropriate folder in the `/sdk` directory. See the **README.md** file located in the library's project folder, for example, the [Azure Storage client library](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage#azure-storage-client-library-for-c).

For API reference docs, tutorials, samples, quick starts, and other documentation, go to [Azure SDK for C++ Developer Docs](https://azure.github.io/azure-sdk-for-cpp).

### Download & Install the SDK

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. You will need to install [Git](https://git-scm.com/downloads) before getting started.

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

To install the libraries for your project, run the following, optionally specifying the triplet. For example, the following will install packages for the `x64-windows` triplet. On Windows, not specifying a triplet will default to `x86-windows`:

```cmd
> .\vcpkg\vcpkg install azure-storage-blobs-cpp:x64-windows
```

See the [list of packages](https://github.com/Azure/azure-sdk-for-cpp#vcpkg) available via vcpkg below. All Azure C++ SDK package names start with `azure-`. You can also search for the libraries you need with the `search` command. For example:

```cmd
> .\vcpkg\vcpkg search azure-
```

Once the library is installed, follow the instructions from the console output to include the library in your `CMake` application. For example, to include `azure-storage-blobs-cpp`, add the following to your `CMakeLists.txt` file:

```CMake
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-storage-blobs)
```

> NOTE: All the Azure client libraries take a dependency on `azure-core-cpp` which provides functionality commonly needed by all Azure clients. When you install any client library via vcpkg, it will bring in all the necessary dependencies as well. You don't need to install those individually to get started.

You can reference this [vcpkg Quick Start](https://github.com/microsoft/vcpkg#quick-start-windows) for more details.

#### Getting Beta Releases in Vcpkg

Official vcpkg registry may have beta versions of Azure SDK client libraries, up until a given library gets released as stable. After that, we don't publish post-first-stable beta releases of that library in the official registry.

If you are interested in both stable releases and post-first-stable beta releases, see [Azure SDK Beta Vcpkg Registry](https://github.com/Azure/azure-sdk-vcpkg-betas/).

### Building your Application

In order to use the SDK installed via vcpkg with CMake, you can use the toolchain file from vcpkg:

```cmd
> cmake -B [build directory] -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg repo]/scripts/buildsystems/vcpkg.cmake
> cmake --build [build directory]
```

#### Using the SDK within your Application

The **entry point** for most scenarios when using the SDK will be a top-level client type corresponding to the Azure service. For example, sending requests to blob storage can be done via the `Azure::Storage::Blobs::BlobClient` API. All APIs on the client type send HTTP requests to the cloud service and return back an HTTP `Response<T>`.

Azure C++ SDK headers needed are located within the `<azure>` folder, with sub-folders corresponding to each service. Similarly, all types and APIs can be found within the `Azure::` namespace. For example, to use functionality from `Azure::Core`, include the following header at the beginning of your application `#include <azure/core.hpp>`.

Here's an example application to help you get started:

```C++
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

#### Key Core concepts

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

```C++
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

You can intermittently poll whether the operation has finished by using the `Poll()` method inside a loop on the returned `Operation<T>` and track progress of the operation using `Value()`, while the operation is not done (using `IsDone()`). Your per-polling custom logic can go in that loop, such as logging progress.Alternatively, if you just want to wait until the operation completes, you can use `PollUntilDone()`.

```C++
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

* `azure-core-cpp`
* `azure-identity-cpp`
* `azure-security-attestation-cpp`
* `azure-security-keyvault-certificates-cpp`
* `azure-security-keyvault-keys-cpp`
* `azure-security-keyvault-secrets-cpp`
* `azure-storage-blobs-cpp`
* `azure-storage-files-datalake-cpp`
* `azure-storage-files-shares-cpp`
* `azure-storage-queues-cpp`

> NOTE: In case of getting linker errors when consuming the SDK on Windows, make sure that [vcpkg triplet](https://vcpkg.readthedocs.io/en/latest/users/triplets/) being consumed matches the [CRT link flags](https://docs.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160) being set for your app or library build. See also `MSVC_USE_STATIC_CRT` build flag.

## Need help

- For reference documentation visit the [Azure SDK for C++ documentation](https://azure.github.io/azure-sdk-for-cpp).
- For tutorials, samples, quick starts and other documentation, visit [Azure for C++ Developers](https://docs.microsoft.com/azure/).
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
