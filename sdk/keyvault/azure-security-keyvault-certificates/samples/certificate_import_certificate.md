# _Importing certificates

This sample demonstrates how to import a certificate in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault.

### _Creating a CertificateClient

To create a new `CertificateClient` to create, get, update, or delete certificates, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Certificate client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:CertificateSample3CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:CertificateSample3Client
CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## _Importing a PEM certificate

You will need the certificate content in PEM format to perform this operation. One sample is provided in certificate-ImportCertificate.hpp as the GetPemCertificate() string.

Once the import options are setup we can call Import certificate and get back the newly imported certificate.

```cpp Snippet:CertificateSample3ImportPEM
// prepare the options
ImportCertificateOptions options;
options.Value = GetPemCertificate();

options.Policy.Enabled = true;
options.Policy.KeyType = CertificateKeyType::Rsa;
options.Policy.KeySize = 2048;
options.Policy.ContentType = CertificateContentType::Pem;
options.Policy.Exportable = true;

// call import API
auto imported = certificateClient.ImportCertificate(pemName, options).Value;
// get some value from the certificate
std::cout << "Imported pem certificate with name " << imported.Name();
```

## _Importing a PKCS certificate

You will need the certificate content in PKCS format to perform this operation. One sample is provided in certificate-ImportCertificate.hpp as the GetPkcsCertificate() string.

Once the import options are setup we can call Import certificate and get back the newly imported certificate

```cpp Snippet:CertificateSample3ImportPKCS
 // prepare the options
ImportCertificateOptions options;
options.Value = GetPemCertificate();

options.Policy.Enabled = true;
options.Policy.KeyType = CertificateKeyType::Rsa;
options.Policy.KeySize = 2048;
options.Policy.ContentType = CertificateContentType::Pkcs12;
options.Policy.Exportable = true;

// call the import API
auto imported = certificateClient.ImportCertificate(pkcsName, options).Value;
// read something from the certificate
std::cout << "Imported pkcs certificate with name " << imported.Name();
```


## _Deleting the certificates

Call StartDeleteCertificate to delete a certificate. This is a long running operation.

```cpp Snippet:CertificateSample1Delete
 // delete the certificates
auto response1 = certificateClient.StartDeleteCertificate(pemName);
auto response2 = certificateClient.StartDeleteCertificate(pkcsName);
```

## _Purging the deleted certificates

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the certificate before its `ScheduledPurgeDate`, the certificate needs to be purged.

```cpp Snippet:CertificateSample3PurgeCertificate
response1.PollUntilDone(defaultWait);
response2.PollUntilDone(defaultWait);
// purge the certificates
certificateClient.PurgeDeletedCertificate(pkcsName);
certificateClient.PurgeDeletedCertificate(pemName);
```
## _Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-certificates/test/samples/certificate-import-certificate)
