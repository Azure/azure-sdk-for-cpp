# Creating, updating rotation policy, and rotating keys

This sample demonstrates how to create a key, update the rotation policy of the key, rotate the key in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/README.md) for links and instructions.

## Creating a KeyClient

To create a new `KeyClient` to create, get, update, or delete keys, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Keys client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and client secret as environment variables.

```cpp Snippet:KeysSample7CreateCredential
auto tenantId = std::getenv("AZURE_TENANT_ID");
auto clientId = std::getenv("AZURE_CLIENT_ID");
auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:KeysSample7KeyClient
KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a key

Let's create an EC key.
If the key already exists in the Azure Key Vault, then a new version of the key is created.

```cpp Snippet:KeysSample7CreateKey
auto keyName = "RotateKey-" + Azure::Core::Uuid::CreateUuid().ToString();
auto createKeyResponse = keyClient.CreateEcKey(CreateEcKeyOptions(keyName));

std::cout << "Created key " << createKeyResponse.Value.Name() << "with id "
          << createKeyResponse.Value.Id() << " and version "
          << createKeyResponse.Value.Properties.Version << std::endl;
```

## Create the key rotation policy 

Next we will define the key rotation policy as needed.

```cpp Snippet:KeysSample7DefinePolicy
KeyRotationPolicy policy;

LifetimeActionsType lifetimeAction1;
lifetimeAction1.Trigger.TimeBeforeExpiry = "P18M";
lifetimeAction1.Action = LifetimeActionType::Notify;
policy.LifetimeActions.emplace_back(lifetimeAction1);

LifetimeActionsType lifetimeAction2;
lifetimeAction2.Action = LifetimeActionType::Rotate;
lifetimeAction2.Trigger.TimeBeforeExpiry = "P30D";
policy.LifetimeActions.emplace_back(lifetimeAction2);

policy.Attributes.ExpiryTime = "P48M";
```

## Updating key properties

Now we will update the key with the new rotation policy.

```cpp Snippet:KeysSample7UpdateKeyRotation
auto putPolicy = keyClient.UpdateKeyRotationPolicy(keyName, policy).Value;

std::cout << "Updated rotation policy " << putPolicy.Id << " for key "
          << createKeyResponse.Value.Name() << std::endl;
```

## Calling Rotate and checking the result

Next we will rotate the key and check the result of the api call.
```cpp Snippet:KeysSample7RotateKey
auto originalKey = keyClient.GetKey(keyName);
auto rotatedKey = keyClient.RotateKey(keyName);

std::cout << "Rotated key " << originalKey.Value.Name() << std::endl
          << "Original version " << originalKey.Value.Properties.Version << std::endl
          << "New Version " << rotatedKey.Value.Properties.Version << std::endl;
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

- [sample7_key_rotation.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/test/samples/sample7-key-rotation/sample7_key_rotation.cpp)

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md