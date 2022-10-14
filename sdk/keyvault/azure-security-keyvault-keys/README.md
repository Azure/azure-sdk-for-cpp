# Azure Key Vault key client library for C++

Azure Key Vault is a cloud service that provides secure storage of keys for encrypting your data.
Multiple keys, and multiple versions of the same key, can be kept in the Azure Key Vault.
Cryptographic keys in Azure Key Vault are represented as [JSON Web Key (JWK)](https://tools.ietf.org/html/rfc7517) objects.

Azure Key Vault Managed HSM is a fully-managed, highly-available, single-tenant, standards-compliant cloud service that enables
you to safeguard cryptographic keys for your cloud applications using FIPS 140-2 Level 3 validated HSMs.

The Azure Key Vault keys library client supports RSA keys and Elliptic Curve (EC) keys, each with corresponding support in hardware security modules (HSM). It offers operations to create, retrieve, update, delete, purge, backup, restore, and list the keys and its versions.

[Source code][key_client_src] | [VCPKG][key_client_vcpkg_package] | [API reference documentation][api_reference] | [Product documentation][keyvault_docs] | [Samples][key_client_samples]

## Getting started

### Include the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Security Key Vault Keys package via vcpkg:

```cmd
> vcpkg install azure-security-keyvault-keys-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-security-keyvault-keys-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-security-keyvault-keys)
```

### Prerequisites

- An [Azure subscription][azure_sub].
- An existing Azure Key Vault. If you need to create an Azure Key Vault, you can use the Azure Portal or [Azure CLI][azure_cli].

See the final two steps in the next section for details on creating the Key Vault with the Azure CLI.

### Authenticate the client

In order to interact with the Key Vault service, you'll need to create an instance of the [KeyClient][key_client_class] class. You need a **vault URL**, which you may see as "DNS Name" in the portal,
and **client secret credentials (client ID, client secret, tenant ID)** to instantiate a client object.

Client Secret Credential authentication is being used in this Getting Started section, but you can find more ways to authenticate with [Azure Identity][azure_identity]. To use the [DefaultAzureCredential][defaultazurecredential] provider shown below,
or other credential providers provided with the Azure SDK, you should install the Azure Identity package:

```PowerShell
# Windows
vcpkg.exe install azure-identity-cpp

# Linux
./vcpkg install azure-identity-cpp
```

#### Create/Get credentials

Use the [Azure CLI][azure_cli] snippet below to create/get client secret credentials.

- Create a service principal and configure its access to Azure resources:
  ```PowerShell
  az ad sp create-for-rbac -n <your-application-name> --skip-assignment
  ```
  Output:
  ```json
  {
    "appId": "generated-app-ID",
    "displayName": "dummy-app-name",
    "name": "http://dummy-app-name",
    "password": "random-password",
    "tenant": "tenant-ID"
  }
  ```
- Take note of the service principal objectId
  ```PowerShell
  az ad sp show --id <appId> --query objectId
  ```
  Output:
  ```
  "<your-service-principal-object-id>"
  ```
- Use the returned credentials above to set **AZURE_CLIENT_ID** (appId), **AZURE_CLIENT_SECRET** (password), and **AZURE_TENANT_ID** (tenant) environment variables. The following example shows a way to do this in Powershell:

  ```PowerShell
  $Env:AZURE_CLIENT_ID="generated-app-ID"
  $Env:AZURE_CLIENT_SECRET="random-password"
  $Env:AZURE_TENANT_ID="tenant-ID"
  ```

- Grant the above mentioned application authorization to perform key operations on the Azure Key Vault:

  ```PowerShell
  az keyvault set-policy --name <your-key-vault-name> --spn $Env:AZURE_CLIENT_ID --key-permissions backup delete get list create encrypt decrypt update
  ```

  > --key-permissions:
  > Accepted values: backup, create, decrypt, delete, encrypt, get, import, list, purge, recover, restore, sign, unwrapKey, update, verify, wrapKey

  If you have enabled role-based access control (RBAC) for Key Vault instead, you can find roles like "Key Vault Crypto Officer" in our [RBAC guide][rbac_guide].
  If you are managing your keys using Managed HSM, read about its [access control][access_control] that supports different built-in roles isolated from Azure Resource Manager (ARM).

- Use the above mentioned Azure Key Vault name to retrieve details of your Vault which also contains your Azure Key Vault URL:

  ```PowerShell
  az keyvault show --name <your-key-vault-name>
  ```

- Create the Azure Key Vault or Managed HSM and grant the above mentioned application authorization to perform administrative operations on the Managed HSM
  (replace `<your-resource-group-name>` and `<your-key-vault-name>` with your own unique names and `<your-service-principal-object-id>` with the value from above):

If you are creating a standard Key Vault resource, use the following CLI command:

```PowerShell
az keyvault create --resource-group <your-resource-group-name> --name <your-key-vault-name>
```

If you are creating a Managed HSM resource, use the following CLI command:

```PowerShell
    az keyvault create --hsm-name <your-key-vault-name> --resource-group <your-resource-group-name> --administrators <your-service-principal-object-id> --location <your-azure-location>
```

#### Activate your managed HSM

This section only applies if you are creating a Managed HSM. All data plane commands are disabled until the HSM is activated. You will not be able to create keys or assign roles.
Only the designated administrators that were assigned during the create command can activate the HSM. To activate the HSM you must download the security domain.

To activate your HSM you need:

- Minimum 3 RSA key-pairs (maximum 10)
- Specify minimum number of keys required to decrypt the security domain (quorum)

To activate the HSM you send at least 3 (maximum 10) RSA public keys to the HSM. The HSM encrypts the security domain with these keys and sends it back.
Once this security domain is successfully downloaded, your HSM is ready to use.
You also need to specify quorum, which is the minimum number of private keys required to decrypt the security domain.

The example below shows how to use openssl to generate 3 self signed certificate.

```PowerShell
openssl req -newkey rsa:2048 -nodes -keyout cert_0.key -x509 -days 365 -out cert_0.cer
openssl req -newkey rsa:2048 -nodes -keyout cert_1.key -x509 -days 365 -out cert_1.cer
openssl req -newkey rsa:2048 -nodes -keyout cert_2.key -x509 -days 365 -out cert_2.cer
```

Use the `az keyvault security-domain download` command to download the security domain and activate your managed HSM.
The example below uses 3 RSA key pairs (only public keys are needed for this command) and sets the quorum to 2.

```PowerShell
az keyvault security-domain download --hsm-name <your-key-vault-name> --sd-wrapping-keys ./certs/cert_0.cer ./certs/cert_1.cer ./certs/cert_2.cer --sd-quorum 2 --security-domain-file ContosoMHSM-SD.json
```

#### Create KeyClient

Once you've populated the **AZURE_CLIENT_ID**, **AZURE_CLIENT_SECRET** and **AZURE_TENANT_ID** environment variables and replaced **your-vault-url** with the above returned URI, you can create the [KeyClient][key_client_class]:

```cpp
// Create a new key client using the default credential from Azure Identity using environment variables previously set,
// including AZURE_CLIENT_ID, AZURE_CLIENT_SECRET, and AZURE_TENANT_ID.
auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();
KeyClient client("AZURE_KEYVAULT_URL", credential);

// Create a new key using the key client.
client.CreateKey("key-name", KeyVaultKeyType::Rsa);

// Retrieve a key using the key client.
key = client.GetKey("key-name");
```

#### Create CryptographyClient

Once you've created a `KeyVaultKey` in the Azure Key Vault, you can also create the [CryptographyClient][crypto_client_class]:

```cpp
// Create a new cryptography client using the default credential from Azure Identity using environment variables previously set,
// including AZURE_CLIENT_ID, AZURE_CLIENT_SECRET, and AZURE_TENANT_ID.
auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();
CryptographyClient cryptoClient(key.Id, credential);
```

## Key concepts

### KeyVaultKey

Azure Key Vault supports multiple key types and algorithms, and enables the use of hardware security modules (HSM) for high value keys.

### KeyClient

A `KeyClient` providing synchronous operations exists in the SDK. Once you've initialized a `KeyClient`, you can interact with the primary resource types in Azure Key Vault.

### CryptographyClient

A `CryptographyClient` providing synchronous operations exists in the SDK. Once you've initialized a `CryptographyClient`, you can use it to perform cryptographic operations with keys stored in Azure Key Vault.

### Thread safety

We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

<!-- CLIENT COMMON BAR -->

[Replaceable HTTP transport adapter](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#http-transport-adapter) |
[Long-running operations](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#long-running-operations) |

<!-- CLIENT COMMON BAR -->

## Examples

The Azure.Security.KeyVault.Keys package supports synchronous APIs.

The following section provides several code snippets using the `client` [created above](#create-keyclient), covering some of the most common Azure Key Vault key service related tasks:

### Examples

- [Create a key](#create-a-key)
- [Retrieve a key](#retrieve-a-key)
- [Update an existing key](#update-an-existing-key)
- [Delete a key](#delete-a-key)
- [Delete and purge a key](#delete-and-purge-a-key)
- [List Keys](#list-keys)
<!-- - [Encrypt and Decrypt](#encrypt-and-decrypt) -->

#### Create a key

Create a key to be stored in the Azure Key Vault. If a key with the same name already exists, then a new version of the key is created.

```cpp
// Create a key. Note that you can specify the type of key
// i.e. Elliptic curve, Hardware Elliptic Curve, RSA
auto key = client.CreateKey("key-name", KeyVaultKeyType::Rsa);

std::cout << key.Name();
std::cout << key.KeyType.ToString();

// Create a software RSA key
auto rsaCreateKey = CreateRsaKeyOptions("rsa-key-name", false);
KeyVaultKey rsaKey = client.CreateRsaKey(rsaCreateKey);

std::cout << rsaKey.Name();
std::cout << rsaKey.KeyType.ToString();

// Create a hardware Elliptic Curve key
// Because only premium Azure Key Vault supports HSM backed keys , please ensure your Azure Key Vault
// SKU is premium when you set "hardwareProtected" value to true
auto echsmkey = new CreateEcKeyOptions("ec-key-name", true);
KeyVaultKey ecKey = client.CreateEcKey(echsmkey);

std::cout << ecKey.Name();
std::cout << ecKey.KeyType.ToString();
```

#### Retrieve a key

`GetKey` retrieves a key previously stored in the Azure Key Vault.

```cpp
KeyVaultKey key = client.GetKey("key-name");

std::cout << key.Name();
std::cout << key.KeyType.ToString();
```

#### Update an existing key

`UpdateKeyProperties` updates a key previously stored in the Azure Key Vault.

```cpp
KeyVaultKey key = client.CreateKey("key-name", KeyVaultKeyType::Rsa);

// You can specify additional application-specific metadata in the form of tags.
key.Properties.Tags["foo"] = "updated tag";

KeyVaultKey updatedKey = client.UpdateKeyProperties(key.Properties);

std::cout << updatedKey.Name();
std::cout << updatedKey.Properties.Version;
std::cout << updatedKey.Properties.UpdatedOn->ToString();
```

#### Delete a key

`StartDeleteKey` starts a long-running operation to delete a key previously stored in the Azure Key Vault.
You can retrieve the key immediately without waiting for the operation to complete.
When [soft-delete](https://docs.microsoft.com/azure/key-vault/general/soft-delete-overview) is not enabled for the Azure Key Vault, this operation permanently deletes the key.

```cpp
DeleteKeyOperation operation = client.StartDeleteKey("key-name");

DeletedKey key = operation.Value();
std::cout << key.Name();
std::cout << key.DeletedOn->ToString();
```

#### Delete and purge a key

You will need to wait for the long-running operation to complete before trying to purge or recover the key.

```cpp
DeleteKeyOperation operation = client.StartDeleteKey("key-name");

// You only need to wait for completion if you want to purge or recover the key.
while (!operation.IsDone())
{
  std::this_thread::sleep_for(std::chrono::seconds(2));

  operation.Poll();
}

DeletedKey key = operation.Value();
client.PurgeDeletedKey(key.Name());
```

#### List Keys

This example lists all the keys in the specified Azure Key Vault.

```cpp
Pageable<KeyProperties> allKeys = client.GetPropertiesOfKeys();

for (auto keys = client.GetPropertiesOfKeys(); keys.HasPage(); keys.MoveToNextPage())
    {
      for (auto const& key : keys.Items)
      {
        std::cout << key.Name;
      }
    }
```

## Troubleshooting

### General

When you interact with the Azure Key Vault key client library using the C++ SDK, errors returned by the service correspond to the same HTTP status codes returned for [REST API][keyvault_rest] requests.

For example, if you try to retrieve a key that doesn't exist in your Azure Key Vault, a `404` error is returned, indicating "Not Found".

```cpp
try
{
  KeyVaultKey key = client.GetKey("some_key").Value;
}
catch (const Azure::Core::RequestFailedException& ex)
{
  std::cout << std::underlying_type<Azure::Core::Http::HttpStatusCode>::type(ex.StatusCode);
}
```

You will notice that additional information is logged, like the client request ID of the operation.

```
Message:
    Azure.RequestFailedException : Service request failed.
    Status: 404 (Not Found)
Content:
    {"error":{"code":"KeyNotFound","message":"Key not found: some_key"}}

Headers:
    Cache-Control: no-cache
    Pragma: no-cache
    Server: Microsoft-IIS/10.0
    x-ms-keyvault-region: westus
    x-ms-request-id: 625f870e-10ea-41e5-8380-282e5cf768f2
    x-ms-keyvault-service-version: 1.1.0.866
    x-ms-keyvault-network-info: addr=131.107.174.199;act_addr_fam=InterNetwork;
    X-AspNet-Version: 4.0.30319
    X-Powered-By: ASP.NET
    Strict-Transport-Security: max-age=31536000;includeSubDomains
    X-Content-Type-Options: nosniff
    Date: Tue, 18 Jun 2019 16:02:11 GMT
    Content-Length: 75
    Content-Type: application/json; charset=utf-8
    Expires: -1
```

## Next steps

Several Azure Key Vault keys client library samples are available to you in this GitHub repository. These samples provide example code for additional scenarios commonly encountered while working with Azure Key Vault:

- [sample1_hello_world.md][hello_world_sample] - for working with Azure Key Vault, including:

  - Create a key
  - Get an existing key
  - Update an existing key
  - Delete a key

- [sample2_backup_and_restore.md][backup_and_restore_sample] - Contains the code snippets working with Azure Key Vault keys, including:

  - Backup and recover a key

- [sample3_get_keys.md][get_keys_sample] - Example code for working with Azure Key Vault keys, including:

  - Create keys
  - List all keys in the Key Vault
  - Update keys in the Key Vault
  - List versions of a specified key
  - Delete keys from the Key Vault
  - List deleted keys in the Key Vault

- [sample4_encrypt_decrypt.md][encrypt_decrypt_sample] - Example code for performing cryptographic operations with Azure Key Vault keys, including:

  - Encrypt and Decrypt data with the CryptographyClient

- [sample5_sign_verify.md][sign_verify_sample] - Example code for working with Azure Key Vault keys, including:

  - Sign a precalculated digest and verify the signature with Sign and Verify
  - Sign raw data and verify the signature with SignData and VerifyData

- [sample6_wrap_unwrap.md][wrap_unwrap_sample] - Example code for working with Azure Key Vault keys, including:
  - Wrap and Unwrap a symmetric key 

- [sample7_key_rotation.md][key_rotation_sample] - Example code for working with Azure Key Vault keys, including:
  - Define Rotation policy
  - Apply rotation policy
  - Rotate key

### Additional Documentation

- For more extensive documentation on Azure Key Vault, see the [API reference documentation][keyvault_rest].
<!-- - For Secrets client library see [Secrets client library][secrets_client_library].
- For Certificates client library see [Certificates client library][certificates_client_library]. -->

## Contributing

See the [CONTRIBUTING.md][contributing] for details on building, testing, and contributing to these libraries.

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA)
declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, visit the [Contributor License Agreement](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment).
Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct][code_of_conduct].
For more information see the [Code of Conduct FAQ][coc_faq] or contact opencode@microsoft.com with any additional questions or comments.

<!-- LINKS -->

[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#download--install-the-sdk
[api_reference]: https://azure.github.io/azure-sdk-for-cpp/keyvault.html
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_identity]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity
[azure_sub]: https://azure.microsoft.com/free/
<!-- ----------------SAMPLES ---------------- -->
[hello_world_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample1_hello_world.md
[backup_and_restore_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample2_backup_and_restore.md
[code_of_conduct]: https://opensource.microsoft.com/codeofconduct/
[get_keys_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample3_get_keys.md
[encrypt_decrypt_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample4_encrypt_decrypt.md
[sign_verify_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample5_sign_verify.md
[wrap_unwrap_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample6_wrap_unwrap.md
[key_rotation_sample]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/samples/sample7_key_rotation.md

<!-- ----------------SAMPLES ---------------- -->


[key_client_class]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/inc/azure/keyvault/keys/key_client.hpp
[crypto_client_class]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/src/cryptography/cryptography_client.cpp
[key_client_vcpkg_package]: https://github.com/microsoft/vcpkg/tree/master/ports/azure-security-keyvault-keys-cpp
[key_client_samples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-keys/samples
[key_client_src]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-keys
[keyvault_docs]: https://docs.microsoft.com/azure/key-vault/
[keyvault_rest]: https://docs.microsoft.com/rest/api/keyvault/

<!-- [secrets_client_library]: https://github.com/Azure/azure-sdk-for-net/tree/main/sdk/keyvault/Azure.Security.KeyVault.Secrets -->

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity#credentials
[contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[access_control]: https://docs.microsoft.com/azure/key-vault/managed-hsm/access-control
[rbac_guide]: https://docs.microsoft.com/azure/key-vault/general/rbac-guide
