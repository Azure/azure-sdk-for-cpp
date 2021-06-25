# Creating, getting, updating, and deleting keys

This sample demonstrates how to create, get, update, and delete a key in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/README.md) for links and instructions.

## Creating a KeyClient

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

## Creating a key

Let's create an RSA key valid for 1 year.
If the key already exists in the Azure Key Vault, then a new version of the key is created.

```cpp Snippet:KeysSample1CreateKey
auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
rsaKey.KeySize = 2048;
rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

keyClient.CreateRsaKey(rsaKey);
```

## Getting a key

Let's get the cloud RSA key from the Azure Key Vault.

```cpp Snippet:KeysSample1GetKey
KeyVaultKey cloudRsaKey = keyClient.GetKey(rsaKeyName).ExtractValue();
std::cout << "Key is returned with name " << cloudRsaKey.Name() << " and type "
            << KeyType::KeyTypeToString(cloudRsaKey.GetKeyType()) << std::endl;

```

## Updating key properties

After one year, the cloud RSA key is still required, so we need to update the expiry time of the key.
The update method can be used to update the expiry attribute of the key.

```cpp Snippet:KeysSample1UpdateKeyProperties
cloudRsaKey.Properties.ExpiresOn
    = cloudRsaKey.Properties.ExpiresOn.GetValue() + std::chrono::hours(24 * 365);
KeyVaultKey updatedKey = keyClient.UpdateKeyProperties(cloudRsaKey.Properties).ExtractValue();
std::cout << "Key's updated expiry time is " << updatedKey.Properties.ExpiresOn->ToString()
            << std::endl;
```

## Updating a key size

We need the cloud RSA key with bigger key size, so you want to update the key in Azure Key Vault to ensure it has the required size.
Calling `CreateRsaKey` on an existing key creates a new version of the key in the Azure Key Vault with the new specified size.

```cpp Snippet:KeysSample1UpdateKey
CreateRsaKeyOptions newRsaKey(rsaKeyName);
newRsaKey.KeySize = 4096;
newRsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

keyClient.CreateRsaKey(newRsaKey);
```

## Deleting a key

The cloud RSA key is no longer needed, so we need to delete it from the Key Vault.

```cpp Snippet:KeysSample1DeleteKey
DeleteKeyOperation operation = keyClient.StartDeleteKey(rsaKeyName);
```

## Purging a deleted key

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the key before its `ScheduledPurgeDate`,
the deleted key needs to be purged. Before it can be purged, you need to wait until the key is fully deleted.

```cpp Snippet:KeysSample1PurgeKey
// You only need to wait for completion if you want to purge or recover the key.
operation.PollUntilDone(std::chrono::milliseconds(2000));

keyClient.PurgeDeletedKey(rsaKeyName);
```

## Source

- [sample1_hello_world.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/test/samples/sample1-hello-world/sample1_hello_world.cpp)

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md
