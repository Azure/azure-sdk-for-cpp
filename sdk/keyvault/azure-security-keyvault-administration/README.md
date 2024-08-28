# Azure Security KeyVault Administration client library for C++

Azure Key Vault Managed HSM is a fully-managed, highly-available, single-tenant, standards-compliant cloud service that enables you to safeguard
cryptographic keys for your cloud applications using FIPS 140-2 Level 3 validated HSMs.

Azure Security Keyvault Administration Package client library for C++ (`azure-security-keyvault-administration`) matches necessary patterns that the development team has established to create a unified SDK written in the C++ programming language. These libraries follow the Azure SDK Design Guidelines for C++.

The library allows client libraries to expose common functionality in a consistent fashion.  Once you learn how to use these APIs in one client library, you will know how to use them in other client libraries.

[Source code][administration_client_src] | [Package (vcpkg)](https://vcpkg.io/en/package/azure-security-keyvault-administration-cpp) | [API reference documentation][api_reference] | [Product documentation][keyvault_docs] | [Samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-administration/samples)

## Getting started

### Prerequisites
- [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview) for package acquisition and dependency management
- [CMake](https://cmake.org/download/) for project build
- An [Azure subscription][azure_sub].
- An existing Azure Key Vault. If you need to create an Azure Key Vault, you can use the [Azure CLI][azure_cli].
- Authorization to an existing Azure Key Vault using either [RBAC][rbac_guide] (recommended) or [access control][access_policy].

To create a Managed HSM resource, run the following CLI command:

```PowerShell
az keyvault create --hsm-name <your-key-vault-name> --resource-group <your-resource-group-name> --administrators <your-user-object-id> --location <your-azure-location>
```

To get `<your-user-object-id>` you can run the following CLI command:

```PowerShell
az ad user show --id <your-user-principal> --query id
```

#### Activate your managed HSM

All data plane commands are disabled until the HSM is activated. You will not be able to create keys or assign roles.
Only the designated administrators that were assigned during the create command can activate the HSM. To activate the HSM you must download the security domain.

To activate your HSM you need:

* A minimum of 3 RSA key-pairs (maximum 10)
* Specify the minimum number of keys required to decrypt the security domain (quorum)

To activate the HSM you send at least 3 (maximum 10) RSA public keys to the HSM. The HSM encrypts the security domain with these keys and sends it back.
Once this security domain is successfully downloaded, your HSM is ready to use.
You also need to specify quorum, which is the minimum number of private keys required to decrypt the security domain.

The example below shows how to use openssl to generate 3 self-signed certificates.

```PowerShell
openssl req -newkey rsa:2048 -nodes -keyout cert_0.key -x509 -days 365 -out cert_0.cer
openssl req -newkey rsa:2048 -nodes -keyout cert_1.key -x509 -days 365 -out cert_1.cer
openssl req -newkey rsa:2048 -nodes -keyout cert_2.key -x509 -days 365 -out cert_2.cer
```

Use the `az keyvault security-domain download` command to download the security domain and activate your managed HSM.
The example below uses 3 RSA key pairs (only public keys are needed for this command) and sets the quorum to 2.

```PowerShell
az keyvault security-domain download --hsm-name <your-managed-hsm-name> --sd-wrapping-keys ./certs/cert_0.cer ./certs/cert_1.cer ./certs/cert_2.cer --sd-quorum 2 --security-domain-file ContosoMHSM-SD.json
```

#### Controlling access to your managed HSM

The designated administrators assigned during creation are automatically added to the "Managed HSM Administrators" [built-in role][built_in_roles],
who are able to download a security domain and [manage roles for data plane access][access_control], among other limited permissions.

To perform other actions on keys, you need to assign principals to other roles such as "Managed HSM Crypto User", which can perform non-destructive key operations:

```PowerShell
az keyvault role assignment create --hsm-name <your-managed-hsm-name> --role "Managed HSM Crypto User" --scope / --assignee-object-id <principal-or-user-object-ID> --assignee-principal-type <principal-type>
```

Please read [best practices][best_practices] for properly securing your managed HSM.

### Install the package

he easiest way to acquire the C++ SDK is leveraging the vcpkg package manager and CMake. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install]. We'll use vcpkg in manifest mode. To start a vcpkg project in manifest mode use the following command at the root of your project: 

```batch
vcpkg new --application
```

To install the Azure \<Service-Name> package via vcpkg:
To add the Azure \<Service-Name> package to your vcpkg enter the following command (We'll also add the Azure Identity library for authentication):

```batch
vcpkg add port azure-security-keyvault-administration-cpp azure-identity-cpp
```

Then, add the following in your CMake file:

```CMake
find_package(azure-security-keyvault-administration-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-security-keyvault-administration Azure::azure-identity)
```

Remember to set `CMAKE_TOOLCHAIN_FILE` to the path to `vcpkg.cmake` either by adding the following to your `CMakeLists.txt` file before your project statement:

```CMake
set(CMAKE_TOOLCHAIN_FILE "vcpkg-root/scripts/buildsystems/vcpkg.cmake")
```

Or by specifying it in your CMake commands with the `-DCMAKE_TOOLCHAIN_FILE` argument.

There is more than one way to acquire and install this library. Check out [our samples on different ways to set up your Azure C++ project][project_set_up_examples].

## Key Concepts
### Thread safety
We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

<!-- CLIENT COMMON BAR -->
[Replaceable HTTP transport adapter](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#http-transport-adapter) |
[Long-running operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#long-running-operations) |
<!-- CLIENT COMMON BAR -->

## Examples 

For detailed samples please review the code provided. 
### SettingsClient 
We'll be using the `DefaultAzureCredential` to authenticate which will pick up the credentials we used when logging in with the Azure CLI earlier. `DefaultAzureCredential` can pick up on a number of Credential types from your environment and is ideal when getting started and developing. Check out our section on [DefaultAzureCredentials](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity#defaultazurecredential) to learn more.

#### GetSettings 

To get all the available settings present on the Keyvault instance we will first create a client : 
```cpp
  auto credential
      = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  SettingsClient settingsClient(std::getenv("AZURE_KEYVAULT_HSM_URL"), credential);
```
Please note that we are using the HSM URL, not the keyvault URL. 

To get the settings we will call the GetSettings API 

```cpp
  // Get all settings
  SettingsListResult settingsList = settingsClient.GetSettings().Value;
```

#### GetSetting

To get a specific setting we will call the GetSetting API bassing the setting name as a string parameter. 

```cpp
  Setting setting = settingsClient.GetSetting(settingsList.Value[0].Name).Value;
```

#### UpdateSetting

To update the value of any of the the available settings, we will call the UpdateSettings API as follows:
```cpp
 UpdateSettingOptions options;
 options.Value = <setting value>;

Setting updatedSetting
   = settingsClient.UpdateSetting(settingsList.Value[0].Name, options).Value;
```

### BackupClient

To create a new `BackupClient` to perform these operations, you need the endpoint to an Azure Key Vault HSM and credentials.

Key Vault BackupClient client for C++ currently supports any `TokenCredential` for authenticating.

```cpp
  auto credential
      = std::make_shared<Azure::Identity::DefaultAzureCredential>();
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp
  // create client
  BackupClient client(std::getenv("AZURE_KEYVAULT_HSM_URL"), credential);
```
#### Create the SasTokenParameter

Since these operations require a blob storage for the backup/restore operations, a SAS token is required for the connection between the services(Key Vault and Storage).  

In this sample we rely on a couple of extra environment variables. 

```cpp
 SasTokenParameter sasTokenParameter;
 // the backup/restore needs a SAS token to access the storage account
 sasTokenParameter.Token
     = Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_TOKEN");
 // the backup/restore needs a url to a blob storage resource
 Azure::Core::Url blobUrl
    = Azure::Core::Url(Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_URL"));
```

#### The Backup operation 

Since this is a long running operation the service provides endpoints to determine the status while the opperation is running. 

##### Starting the backup operation

```cpp
// Create a full backup using a user-provided SAS token to an Azure blob storage container.
auto backupResponse = client.FullBackup(blobUrl, sasTokenParameter).Value;

std::cout << "Backup Job Id: " << backupResponse.Value().JobId << std::endl
            << "Backup Status: " << backupResponse.Value().Status << std::endl;
```

##### Backup operation waiting 

In order to wait for the operation to complete we will call the polling method.

```cpp
// Wait for the operation to complete.
auto backupStatus = backupResponse.PollUntilDone(10s);

std::cout << "Backup Job Id: " << backupStatus.Value.JobId << std::endl
            << "Backup Status: " << backupStatus.Value.Status << std::endl;
```

#### The FullRestore operation

Similar to the backup operation after we initialize the operation we can check the status. 

#### Starting the restore operation 

the restore operation requires a folder where a backup was previously performed along side the SAS token parameter. 
```cpp
// Restore the full backup using a user-provided SAS token to an Azure blob storage container.
std::cout << "Folder to restore: " << folderToRestore << std::endl;
auto restoreResponse = client.FullRestore(blobUrl, folderToRestore, sasTokenParameter).Value;
std::cout << "Restore Job Id: " << restoreResponse.Value().JobId << std::endl
            << "Restore Status: " << restoreResponse.Value().Status << std::endl;
```

##### FullRestore operation waiting

```cpp
// Wait for the operation to complete.
auto restoreStatus = restoreResponse.PollUntilDone(10s);
std::cout << "Restore Job Id: " << restoreStatus.Value.JobId << std::endl
            << "Restore Status: " << restoreStatus.Value.Status << std::endl;
```

#### The SelectiveRestore operation

Similar to the backup operation after we initialize the operation we can check the status. 

##### Starting the restore operation 

The selective restore operation requires a folder where a backup was previously performed along side the SAS token parameter. 

```cpp
// Restore the full backup using a user-provided SAS token to an Azure blob storage container.
std::string folderToRestore = ...;
std::cout << "Folder to restore: " << restoreBlobDetails.FolderToRestore << std::endl;
auto selectiveRestore = client.SelectiveKeyRestore("keyName", blobUrl, folderToRestore, sasTokenParameter);
std::cout << "Restore Job Id: " << restoreResponse.Value.JobId << std::endl
          << "Restore Status: " << restoreResponse.Value.Status << std::endl;
```

##### Selective restore operation completion

```cpp
// Wait for the operation to complete.
auto selectiveStatus = selectiveRestore.PollUntilDone(10s);
std::cout << "Selective Restore Job Id: " << selectiveStatus.Value.JobId << std::endl
            << "Selective Restore Status: " << selectiveStatus.Value.Status << std::endl;
```


## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit the [Contributor License Agreement](https://cla.microsoft.com).

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

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_sub]: https://azure.microsoft.com/free/
[api_reference]: https://azure.github.io/azure-sdk-for-cpp/keyvault.html
[administration_client_src]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-administration
[keyvault_docs]: https://docs.microsoft.com/azure/key-vault/
[access_control]: https://learn.microsoft.com/azure/key-vault/managed-hsm/access-control
[access_policy]: https://learn.microsoft.com/azure/key-vault/general/assign-access-policy
[rbac_guide]: https://learn.microsoft.com/azure/key-vault/general/rbac-guide
[best_practices]: https://learn.microsoft.com/azure/key-vault/managed-hsm/best-practices
[built_in_roles]: https://learn.microsoft.com/azure/key-vault/managed-hsm/built-in-roles
