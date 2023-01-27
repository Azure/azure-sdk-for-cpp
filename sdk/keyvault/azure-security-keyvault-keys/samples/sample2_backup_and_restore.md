# _Back up and restore a key

This sample demonstrates how to back up and restore a Key from Azure Key Vault.
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
auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
rsaKey.KeySize = 2048;
rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

keyClient.CreateRsaKey(rsaKey);
```

## _Backing up a key

You might make backups in case keys get accidentally deleted.
For long term storage, it is ideal to write the backup to a file, disk, database, etc.
For the purposes of this sample, we are storing the back up in a temporary memory area.

```cpp Snippet:KeysSample2BackupKey
std::vector<uint8_t> backupKey(keyClient.BackupKey(rsaKeyName).ExtractValue());
```

## _Restoring a key

If the key is deleted for any reason, we can use the backup value to restore it in the Azure Key Vault.

```cpp Snippet:KeysSample2RestoreKey
auto restoredKey = keyClient.RestoreKeyBackup(inMemoryBackup).ExtractValue();
```

## _Source

To see the full example source, see:

- [sample2_backup_and_restore.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/test/samples/sample2-backup-and-restore/sample2_backup_and_restore.cpp)

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md
