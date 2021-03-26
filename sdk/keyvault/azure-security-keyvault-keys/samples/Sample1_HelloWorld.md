# Creating, getting, updating, and deleting keys

This sample demonstrates how to create, get, update, and delete a key in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-net/blob/master/sdk/keyvault/Azure.Security.KeyVault.Keys/README.md) for links and instructions.

## Creating a KeyClient

To create a new `KeyClient` to create, get, update, or delete keys, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Keys client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the tenant id, client id and client secret as environment variables.

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
std::string rsaKeyName("CloudRsaKey" + Azure::Core::Uuid::CreateUuid().ToString());
try
{
    auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
    rsaKey.KeySize = 2048;
    rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    keyClient.CreateRsaKey(rsaKey);
}
catch (Azure::Core::Credentials::AuthenticationException const& e)
{
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
}
catch (Azure::Security::KeyVault::Common::KeyVaultException const& e)
{
    std::cout << "KeyVault Client Exception happened:" << std::endl << e.Message << std::endl;
}
```

## Getting a key

Let's get the cloud RSA key from the Azure Key Vault.

```cpp Snippet:KeysSample1GetKey
try
{
    KeyVaultKey cloudRsaKey = keyClient.GetKey(rsaKeyName).ExtractValue();
    std::cout << "Key is returned with name " << cloudRsaKey.Name() << " and type "
            << KeyType::KeyTypeToString(cloudRsaKey.GetKeyType()) << std::endl;
}
catch (Azure::Core::Credentials::AuthenticationException const& e)
{
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
}
catch (Azure::Security::KeyVault::Common::KeyVaultException const& e)
{
    std::cout << "KeyVault Client Exception happened:" << std::endl << e.Message << std::endl;
}
```

## Updating key properties

After one year, the cloud RSA key is still required, so we need to update the expiry time of the key.
The update method can be used to update the expiry attribute of the key.

```C# Snippet:KeysSample1UpdateKeyProperties
cloudRsaKey.Properties.ExpiresOn.Value.AddYears(1);
KeyVaultKey updatedKey = client.UpdateKeyProperties(cloudRsaKey.Properties, cloudRsaKey.KeyOperations);
Debug.WriteLine($"Key's updated expiry time is {updatedKey.Properties.ExpiresOn}");
```

## Updating a key size

We need the cloud RSA key with bigger key size, so you want to update the key in Azure Key Vault to ensure it has the required size.
Calling `CreateRsaKey` on an existing key creates a new version of the key in the Azure Key Vault with the new specified size.

```C# Snippet:KeysSample1UpdateKey
var newRsaKey = new CreateRsaKeyOptions(rsaKeyName, hardwareProtected: false)
{
    KeySize = 4096,
    ExpiresOn = DateTimeOffset.Now.AddYears(1)
};

client.CreateRsaKey(newRsaKey);
```

## Deleting a key

The cloud RSA key is no longer needed, so we need to delete it from the Key Vault.

```C# Snippet:KeysSample1DeleteKey
DeleteKeyOperation operation = client.StartDeleteKey(rsaKeyName);
```

## Purging a deleted key

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the key before its `ScheduledPurgeDate`,
the deleted key needs to be purged. Before it can be purged, you need to wait until the key is fully deleted.

```C# Snippet:KeysSample1PurgeKey
// You only need to wait for completion if you want to purge or recover the key.
while (!operation.HasCompleted)
{
    Thread.Sleep(2000);

    operation.UpdateStatus();
}

client.PurgeDeletedKey(rsaKeyName);
```

## Purging a deleted key asynchronously

When writing asynchronous code, you can instead await `WaitForCompletionAsync` to wait indefinitely.
You can optionally pass in a `CancellationToken` to cancel waiting after a certain period or time or any other trigger you require.

```C# Snippet:KeysSample1PurgeKeyAsync
// You only need to wait for completion if you want to purge or recover the key.
await operation.WaitForCompletionAsync();

await client.PurgeDeletedKeyAsync(rsaKeyName);
```

## Source

- [Synchronous Sample1_HelloWorld.cs](https://github.com/Azure/azure-sdk-for-net/blob/master/sdk/keyvault/Azure.Security.KeyVault.Keys/tests/samples/Sample1_HelloWorld.cs)
- [Asynchronous Sample1_HelloWorldAsync.cs](https://github.com/Azure/azure-sdk-for-net/blob/master/sdk/keyvault/Azure.Security.KeyVault.Keys/tests/samples/Sample1_HelloWorldAsync.cs)

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-net/blob/master/sdk/identity/Azure.Identity/README.md
