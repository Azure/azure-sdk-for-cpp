// client.tsp
import "./main.tsp";
import "@azure-tools/typespec-client-generator-core";

using Azure.ClientGenerator.Core;
using Azure.Core.Traits;
using TypeSpec.Rest;
using TypeSpec.Http;

using KeyVault;
@@clientName(KeyVault, "SecretClient"); // This is the name of the client
@@clientName(KeyVault.SecretProperties,"SecretKeyProperties");
@@clientName(KeyVault.SecretAttributes, "SecretProperties");
@@clientName(KeyVault.getSecrets, "GetPropertiesOfSecrets");
@@clientName(KeyVault.getSecretVersions, "GetPropertiesOfSecretsVersions");
@@clientName(KeyVault.SecretBundle, "KeyVaultSecret");
@@clientName(KeyVault.SecretBundle.attributes, "Properties");
@@clientName(KeyVault.DeletedSecretBundle,"DeletedSecret");
@@clientName(KeyVault.updateSecret, "UpdateSecretProperties");
@@clientName(KeyVault.restoreSecret, "RestoreSecretBackup");
@@clientName(KeyVault.deleteSecret, "StartDeleteSecret");// <-- this should be an LRO
@@clientName(KeyVault.DeletedSecretBundle,"DeletedSecret"); 
@@clientName(KeyVault.recoverDeletedSecret, "StartRecoverDeletedSecret");// <-- this should be an LRO

