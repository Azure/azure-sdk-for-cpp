# Troubleshooting Azure Key Vault Certificates SDK Issues

The `Azure::Security::KeyVault::Certificates` package provides APIs for operations on Azure Key Vault for the
`CertificateClient` class. This troubleshooting guide contains steps for diagnosing issues specific to the
`Azure::Security::KeyVault::Certificates` package.

See our [Azure Key Vault SDK Troubleshooting Guide](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/TROUBLESHOOTING.md)
to troubleshoot issues common to the Azure Key Vault SDKs for C++.

## Table of Contents

* [Troubleshooting Azure.Security.KeyVault.Certificates Issues](#troubleshooting-azuresecuritykeyvaultcertificates-issues)
  * [No Certificate with Private Key Found](#no-certificate-with-private-key-found)

## Troubleshooting Azure.Security.KeyVault.Certificates Issues

### No Certificate with Private Key Found

You may see an error similar to the following when using `CertificateClient::ImportCertificate`.
```text
Azure::RequestFailedException
ErrorCode: 400 (Bad Request)

Message:"No certificate with private key found in the specified X.509 certificate content. Please specify X.509 certificate content with only one certificate containing private key."}}
```

Check that your certificate contains a private key using `X509Certificate2::HasPrivateKey`, for example. If it was `true`
but you still see this error, check that you do not use `X509Certificate2::RawData`, which does not contain the
private key. Instead use `X509Certificate2::Export(X509CertificateType::Pkcs12)` method to export a PKCS12 (PFX)-encoded buffer. If you want to import a PEM file, read the file into a `std::vector<std::uint8_t>` buffer and call `CertificateClient::ImportCertificate` with the buffer directly.

See [`X509Certificate` documentation](https://learn.microsoft.com/windows/win32/api/schannel/ns-schannel-x509certificate)
for more information.
