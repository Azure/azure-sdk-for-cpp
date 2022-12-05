# Backup and Restore secrets

This sample demonstrates how to backup and restore in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault.

## Creating a SecretClient

To create a new `SecretClient` to create, get, update, or delete secrets, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Secrets client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:SecretSample2CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SecretSample2SecretClient
SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Secret

Call SetSecret to create a secret with the name and secret value.

```cpp Snippet:SecretSample2SetSecret
std::string secretName("MySampleSecret");
std::string secretValue("my secret value");

secretClient.SetSecret(secretName, secretValue);
```

## Getting a Secret

Call GetSecret to retrieve a secret from Key Vault.

```cpp Snippet:SecretSample2GetSecret
// get secret
Secret secret = secretClient.GetSecret(secretName).Value;
std::cout << "Secret is returned with name " << secret.Name << " and value " << secret.Value
          << std::endl;
```

## Creating a Backup for the secret properties

Call BackupSecret to retrieve the secret backup.  BackupSecret will will return a vector of bytes representing the backed up content.


```cpp Snippet:SecretSample2BackupSecret
std::cout << "\t-Backup secret" << std::endl;
std::vector<uint8_t> backupSecret(secretClient.BackupSecret(secret.Name).Value.Secret);
backUpSize = backupSecret.size();
```

## Deleting the secret in order to later restore it

Call StartDeleteSecret to delete a secret. This is a long running operation.

```cpp Snippet:SecretSample2DeleteSecret
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);
```

## Purging a deleted secret

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the secret before its `ScheduledPurgeDate`, the secret needs to be purged.

```cpp Snippet:SecretSample2PurgeSecret
// You only need to wait for completion if you want to purge or recover the secret.
operation.PollUntilDone(2s);

// purge the deleted secret
secretClient.PurgeDeletedSecret(secret.Name);
```

## Restoring a secret

Call RestoreSecretBackup  to restore a secret from a backup  obtained at the previous(backup) step.

```cpp Snippet:SecretSample2RestoreSecret
std::cout << "\t-Restore Secret" << std::endl;
auto restoredSecret = secretClient.RestoreSecretBackup(backedUpSecret).Value;
```

## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-secrets/test/samples/sample2-backup-restore)