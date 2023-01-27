# _Listing keys, key versions, and deleted keys

This sample demonstrates how to list keys and versions of a given key, and list deleted keys in a soft delete-enabled Key Vault.
To get started, you'll need a URI to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/README.md) for links and instructions.

## _Creating a KeyClient

To create a new `KeyClient` to create, get, update, or delete keys, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Keys client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and client secret as environment variables.

```cpp Snippet:KeysSample1CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:KeysSample1KeyClient
KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## _Creating a key

Let's create an RSA key valid for 1 year.
If the key already exists in the Azure Key Vault, then a new version of the key is created.

```cpp Snippet:KeysSample1CreateKey
std::string rsaKeyName("CloudRsaKey-" + Azure::Core::Uuid::CreateUuid().ToString());
auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
rsaKey.KeySize = 2048;
rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

std::string ecKeyName("CloudEcKey-" + Azure::Core::Uuid::CreateUuid().ToString());
auto ecKey = CreateEcKeyOptions(ecKeyName);
ecKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

std::cout << "\t-Create Keys" << std::endl;
keyClient.CreateRsaKey(rsaKey);
keyClient.CreateEcKey(ecKey);
```

## _Listing keys

You need to check the type of keys that already exist in your Azure Key Vault.
Let's list the keys and print their types. List operations don't return the actual key, but only properties of the key.
So, for each returned key we call GetKey to get the actual key.

```cpp Snippet:KeysSample3ListKeys
for (auto keys = keyClient.GetPropertiesOfKeys().ExtractValue();;)
{
    for (auto const& key : keys.Items)
    {
    if (key.Managed)
    {
        continue;
    }
    auto keyWithType = keyClient.GetKey(key.Name).ExtractValue();
    std::cout << "Key is returned with name: " << keyWithType.Name()
                << " and type: " << KeyType::KeyTypeToString(keyWithType.GetKeyType())
                << std::endl;
    }

    if (!keys.ContinuationToken.HasValue())
    {
    // No more pages for the response, break the loop
    break;
    }

    // Get the next page
    GetPropertiesOfKeysOptions options;
    options.ContinuationToken = keys.ContinuationToken.GetValue();
    keys = keyClient.GetPropertiesOfKeys(options).ExtractValue();
}
```

## _Updating RSA key size

We need the cloud RSA key with bigger key size, so you want to update the key in Azure Key Vault to ensure it has the required size.
Calling `CreateRsaKey` on an existing key creates a new version of the key in the Azure Key Vault with the new specified size.

```cpp Snippet:KeysSample3UpdateKey
CreateRsaKeyOptions newRsaKey(rsaKeyName);
newRsaKey.KeySize = 4096;
newRsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

keyClient.CreateRsaKey(newRsaKey);
```

## _Listing key versions

You need to check all the different versions cloud RSA key had previously.
Lets print all the versions of this key.

```cpp Snippet:KeysSample3ListKeyVersions
for (auto keyVersions
        = keyClient.GetPropertiesOfKeyVersions(rsaKeyName).ExtractValue();
        ;)
{
    for (auto const& key : keyVersions.Items)
    {
    std::cout << "Key's version: " << key.Version << " with name: " << key.Name << std::endl;
    }

    if (!keyVersions.ContinuationToken.HasValue())
    {
    // No more pages for the response, break the loop
    break;
    }

    // Get the next page
    GetPropertiesOfKeyVersionsOptions options;
    options.ContinuationToken = keyVersions.ContinuationToken.GetValue();
    keyVersions
        = keyClient.GetPropertiesOfKeyVersions(rsaKeyName, options).ExtractValue();
}
```

## _Deleting keys

The cloud RSA Key and the cloud EC keys are no longer needed.
You need to delete them from the Azure Key Vault.

```cpp Snippet:KeysSample3DeletedKeys
DeleteKeyOperation rsaOperation = keyClient.StartDeleteKey(rsaKeyName);
DeleteKeyOperation ecOperation = keyClient.StartDeleteKey(ecKeyName);

// You only need to wait for completion if you want to purge or recover the key.
rsaOperation.PollUntilDone(std::chrono::milliseconds(2000));
ecOperation.PollUntilDone(std::chrono::milliseconds(2000));
```

## _Listing deleted keys

You can list all the deleted and non-purged keys, assuming Azure Key Vault is soft delete-enabled.

```cpp Snippet:KeysSample3ListDeletedKeys
nextPage = true;
for (auto keysDeletedPage = keyClient.GetDeletedKeys().ExtractValue();;)
{
    for (auto const& key : keysDeletedPage.Items)
    {
    std::cout << "Deleted key's name: " << key.Name()
                << ", recovery level: " << key.Properties.RecoveryLevel
                << " and recovery Id: " << key.RecoveryId << std::endl;
    }

    if (!keysDeletedPage.ContinuationToken.HasValue())
    {
    // No more pages for the response, break the loop
    break;
    }

    // Get the next page
    GetDeletedKeysOptions options;
    options.ContinuationToken = keysDeletedPage.ContinuationToken.GetValue();
    keysDeletedPage = keyClient.GetDeletedKeys(options).ExtractValue();
}
```

## _Source

To see the full example source, see:

- [sample3_get_keys.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/test/samples/sample3-get-keys/sample3_get_keys.cpp)

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md
