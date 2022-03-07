# Creating, get properties, get versions, delete, get deleted and purge certificates

This sample demonstrates how to:
* create certificates
* get properties of certificates
* get properties of certificate versions
* delete a certificate
* get deleted certificates
* purge

in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault.

### Creating a CertificateClient

To create a new `CertificateClient` to create, get, update, or delete certificates, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Certificate client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:CertificateSample2CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:CertificateSample2Client
CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Certificate

Call StartCreateCertificate to create a new certificate, with specified properties and policy.
Call PollUntilDone to poll the status of the creation. Once the opperation has completed we can continue.

```cpp Snippet:CertificateSample2Create
std::string certificateName = "Sample1";
CertificateCreateOptions options
... 
// start the create process
auto response = certificateClient.StartCreateCertificate(certificateName, options);
// wait for complete to get the certificate
auto pollResponse = response.PollUntilDone(defaultWait).Value;
```

## Getting properties of Certificates

Call GetPropertiesOfCertificates to retrieve information about certificates from Key Vault.

```cpp Snippet:CertificateSample2GetProperties
 // get properties of certificates
for (auto certificates = certificateClient.GetPropertiesOfCertificates();
    certificates.HasPage();
    certificates.MoveToNextPage())
{ 
    // go through every certificate of each page returned
    // the number of results returned for in a  page is not guaranteed
    // it can be anywhere from 0 to 25
    std::cout << "Found " << certificates.Items.size() << " certificates.";
    
    for (auto oneCertificate : certificates.Items)
    {
        std::cout << "Certificate name : " << oneCertificate.Name;
    }
}
```

## Creating a new certificate version 

Repeat the create certificate procedure, for an existing certificate it will create a new version of it.

## Getting the versions of a certificate 

To get information about certificate versions call GetPropertiesOfCertificateVersions.

```cpp Snippet:CertificateSample2GetProperties
// get properties of all the versions of a certificate
for (auto certificateVersions
    = certificateClient.GetPropertiesOfCertificateVersions(certificateName1);
    certificateVersions.HasPage();
    certificateVersions.MoveToNextPage())
{ 
    // go through every certificate of each page returned
    // the number of results returned for in a  page is not guaranteed
    // it can be anywhere from 0 to 25

    std::cout << "Found " << certificateVersions.Items.size()
            << " certificate versions for certificate " << certificateName1;
}
```
## Deleting the certificates 

Now we will delete the certificates. Since this is a long running operation we need to wait for the operation to finish

```cpp Snippet:CertificateSample2Delete
// delete the certificates
auto response1 = certificateClient.StartDeleteCertificate(certificateName1);
auto response2 = certificateClient.StartDeleteCertificate(certificateName2);
response1.PollUntilDone(defaultWait);
response2.PollUntilDone(defaultWait);
```

## Getting the deleted certificates 

After the certificates are deleted , but not yet purged we can call GetDeletedCertificates

```cpp Snippet:CertificatesSample2GetDeleted
// get properties of deleted certificates
for (auto deletedCertificates = certificateClient.GetDeletedCertificates();
    deletedCertificates.HasPage();
    deletedCertificates.MoveToNextPage())
{
    // go through every certificate of each page returned
    // the number of results returned for in a  page is not guaranteed
    // it can be anywhere from 0 to 25
    std::cout << "Found " << deletedCertificates.Items.size() << " deleted certificates.";
}
```

## Purging the deleted certificates

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the certificate before its `ScheduledPurgeDate`, the certificate needs to be purged.

```cpp Snippet:certificateSample2Purge
 // purge the certificates
{
  certificateClient.PurgeDeletedCertificate(certificateName1);
  certificateClient.PurgeDeletedCertificate(certificateName2);
}
```
## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-certificates/test/samples/certificate-get-certificates)
