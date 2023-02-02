# Azure Key Vault Certificates client library for C++

Azure Key Vault is a cloud service that provides secure storage and automated management of certificates used throughout a cloud application. Multiple certificates, and multiple versions of the same certificate, can be kept in the Azure Key Vault. Each certificate in the vault has a policy associated with it which controls the issuance and lifetime of the certificate, along with actions to be taken as certificates near expiry.

The Azure Key Vault certificates client library enables programmatically managing certificates, offering methods to get certificates, policies, issuers, and contacts.

[Source code][certificate_client_src] | [API reference documentation][api_reference] | [Product documentation][keyvault_docs]

## Getting started

### Install the package
Install the Azure Key Vault certificates client library for C++ with vcpkg:

```cmd
vcpkg install azure-security-keyvault-certificates-cpp
```

### Prerequisites
* An [Azure subscription][azure_sub].
* An existing Azure Key Vault. If you need to create an Azure Key Vault, you can use the Azure Portal or [Azure CLI][azure_cli].

If you use the Azure CLI, replace `<your-resource-group-name>` and `<your-key-vault-name>` with your own, unique names:

```PowerShell
az keyvault create --resource-group <your-resource-group-name> --name <your-key-vault-name>
```

## Key concepts
### KeyVault Certificate
A `KeyVaultCertificate` is the fundamental resource within Azure Key Vault. You'll use certificates to encrypt and verify encrypted or signed data.

### CertificateClient
With a `CertificateClient` you can get certificates from the vault, create new certificates and
new versions of existing certificates, update certificate metadata, and delete certificates. You
can also manage certificate issuers, contacts, and management policies of certificates. This is
illustrated in the examples below.

### Thread safety
We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

<!-- CLIENT COMMON BAR -->
[Replaceable HTTP transport adapter](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#http-transport-adapter) |
[Long-running operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#long-running-operations) |
<!-- CLIENT COMMON BAR -->

## Examples

For detailed samples please review the samples provided.

#### Creating a CertificateClient

To create a new `CertificateClient` to create, get, update, or delete certificates, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Certificate client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:CertificateSample1CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:CertificateSample1Client
CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

### Start creating a Certificate

Call StartCreateCertificate to start creating a new certificate, with specified properties and policy.

```cpp Snippet:CertificateSample1Create
std::string certificateName = "Sample1";
CertificateCreateOptions options;
... 
// start the create process
auto response = certificateClient.StartCreateCertificate(certificateName, options);
```

### Getting a Certificate once completed

Call PollUntilDone to poll the status of the creation. Once the opperation has completed we will call GetCertificate to get the newly created certificate.

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

### Updating certificate properties

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

### Deleting a Certificate

Call StartDeleteCertificate to delete a certificate. This is a long running operation.

```cpp Snippet:CertificateSample1Delete
auto response = certificateClient.StartDeleteCertificate(certificateName);

```

### Purging a deleted certificate

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the certificate before its `ScheduledPurgeDate`, the certificate needs to be purged.

```cpp Snippet:CertificateSample1Purge
auto result = response.PollUntilDone(defaultWait);
certificateClient.PurgeDeletedCertificate(certificateName);
```

### Getting properties of Certificates

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

### Creating a new certificate version 

Repeat the create certificate procedure, for an existing certificate it will create a new version of it.

### Getting the versions of a certificate 

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
### Deleting multiple certificates

Now we will delete the certificates. Since this is a long running operation we need to wait for the operation to finish

```cpp Snippet:CertificateSample2Delete
// delete the certificates
auto response1 = certificateClient.StartDeleteCertificate(certificateName1);
auto response2 = certificateClient.StartDeleteCertificate(certificateName2);
response1.PollUntilDone(defaultWait);
response2.PollUntilDone(defaultWait);
```

### Getting the deleted certificates

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

### Importing a PEM certificate

You will need the certificate content in PEM format to perform this operation. One sample is provided in certificate-ImportCertificate sample.

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

### Importing a PKCS certificate

You will need the certificate content in PKCS format to perform this operation. One sample is provided in certificate-ImportCertificate sample.

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

## Troubleshooting

When you interact with the Azure Key Vault Secrets client library using the C++ SDK, errors returned by the service correspond to the same HTTP status codes returned for requests.

For example, if you try to retrieve a key that doesn't exist in your Azure Key Vault, a `404` error is returned, indicating "Not Found".

```cpp
try
{
  Secret secret = client.GetSecret("some_secret").Value;
}
catch (const Azure::Core::RequestFailedException& ex)
{
  std::cout << std::underlying_type<Azure::Core::Http::HttpStatusCode>::type(ex.StatusCode);
}
```

You will notice that additional information is logged, like the client request ID of the operation.

### Additional Documentation

- For more extensive documentation on Azure Key Vault, see the [API reference documentation][keyvault_rest].

# Next steps

Several Azure Key Vault secrets client library samples are available to you in this GitHub repository. These samples provide example code for additional scenarios commonly encountered while working with Azure Key Vault:

* [Certificate-Basic-Operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-certificates/samples/certificate_basic_operations.md):
  * create a certificate
  * get a certificate
  * update a certificate
  * delete a certificate
  * purge a certificate

* [Certificates-Get-Certificates](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-certificates/samples/certificate_get_certificates.md):
  * create certificates
  * get properties of certificates
  * get properties of certificate versions
  * delete a certificate
  * get deleted certificates
  * purge a certificate

* [Certificates-Import-Certificate](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-certificates/samples/certificate_import_certificate.md):
  * Import a PEM certificate
  * import a PKCS certificate

## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit the [Contributor License Agreement](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[api_reference]: https://azure.github.io/azure-sdk-for-cpp/keyvault.html
[certificate_client_src]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-certificates
[code_of_conduct]: https://opensource.microsoft.com/codeofconduct/
[keyvault_docs]: https://docs.microsoft.com/azure/key-vault/
[keyvault_rest]: https://docs.microsoft.com/rest/api/keyvault/
[contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_sub]: https://azure.microsoft.com/free/
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests