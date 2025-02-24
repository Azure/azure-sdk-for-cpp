// client.tsp
import "./main.tsp";
import "@azure-tools/typespec-client-generator-core";

using Azure.ClientGenerator.Core;
using Azure.Core.Traits;
using TypeSpec.Rest;
using TypeSpec.Http;

using KeyVault;
@@clientName(KeyVault, "SecretClient","cpp"); // This is the name of the client
@@clientName(KeyVault.SecretProperties,"SecretKeyProperties","cpp");
@@clientName(KeyVault.SecretAttributes, "SecretProperties","cpp");
@@clientName(KeyVault.getSecrets, "GetPropertiesOfSecrets","cpp");
@@clientName(KeyVault.getSecretVersions, "GetPropertiesOfSecretsVersions","cpp");
@@clientName(KeyVault.SecretBundle, "KeyVaultSecret","cpp");
@@clientName(KeyVault.SecretBundle.attributes, "Properties","cpp");
@@clientName(KeyVault.DeletedSecretBundle,"DeletedSecret","cpp");
@@clientName(KeyVault.updateSecret, "UpdateSecretProperties","cpp");
@@clientName(KeyVault.restoreSecret, "RestoreSecretBackup","cpp");
@@clientName(KeyVault.deleteSecret, "StartDeleteSecret","cpp");// <-- this should be an LRO
@@clientName(KeyVault.DeletedSecretBundle,"DeletedSecret","cpp"); 
@@clientName(KeyVault.recoverDeletedSecret, "StartRecoverDeletedSecret","cpp");// <-- this should be an LRO

