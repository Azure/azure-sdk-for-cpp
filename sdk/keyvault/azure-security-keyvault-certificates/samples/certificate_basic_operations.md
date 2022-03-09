# Creating, getting, updating, and deleting certificates

This sample demonstrates how to :
* create a certificate
* get a certificate 
* update a certificate
* delete a certificate 
* purge a certificate

in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault.

## Creating a CertificateClient

To create a new `CertificateClient` to create, get, update, or delete certificates, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Certificate client for C++ currently supports both the `ClientSecretCredential` and the `ChallengeClientSecretCredential` for authenticating.

The ChallengeClientSecretCredential is useful for multi tenant authentication.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:CertificateSample1CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
    auto credential
      = std::make_shared<Azure::Identity::ChallengeClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:CertificateSample1Client
CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Certificate

Call StartCreateCertificate to create a new certificate, with specified properties and policy.

```cpp Snippet:CertificateSample1Create
std::string certificateName = "Sample1";
CertificateCreateOptions options;
... 
// start the create process
auto response = certificateClient.StartCreateCertificate(certificateName, options);
```

## Getting a Certificate

Call PollUntilDone to poll the status of the creation. Once the opperation has completed we will call GetCertificate

```cpp Snippet:CertificateSample1Get
// wait for complete to get the certificate
auto pollResponse = response.PollUntilDone(defaultWait).Value;
// check the status of the poll response
if (!pollResponse.Error && pollResponse.Status.Value() == "completed")
{
// get the certificate
certificate = certificateClient.GetCertificate(certificateName).Value;
std::cout << "Created certificate with policy. Certificate name : " << certificate.Name();
}
```

## Updating certificate properties

Call UpdateCertificateProperties to change one of the certificate properties.


```cpp Snippet:CertificateSample1UpdateCertificateProperties
CertificateUpdateOptions updateOptions;
updateOptions.Properties = certificate.Properties;
updateOptions.Properties.Enabled = false;

auto updatedCertificate
        = certificateClient
            .UpdateCertificateProperties(
                certificateName, certificate.Properties.Version, updateOptions)
            .Value;

std::cout << "After update certificate is enabled : "
          << (updatedCertificate.Properties.Enabled.Value() ? "true" : "false");
```

## Deleting a Certificate

Call StartDeleteCertificate to delete a certificate. This is a long running operation.

```cpp Snippet:CertificateSample1Delete
auto response = certificateClient.StartDeleteCertificate(certificateName);

```

## Purging a deleted certificate

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the certificate before its `ScheduledPurgeDate`, the certificate needs to be purged.

```cpp Snippet:CertificateSample1Purge
auto result = response.PollUntilDone(defaultWait);
certificateClient.PurgeDeletedCertificate(certificateName);
```
## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-certificates/test/samples/certificate-basic-operations)
