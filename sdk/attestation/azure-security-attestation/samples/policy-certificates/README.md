---
page_type: sample
languages:
- C++
products:
- azure
- azure-attestation
urlFragment: attestation-samples

---

# Policy Management Certificate Samples for the Microsoft Azure Attestation client library for C++

These code samples show how to enumerate, add and remove attestation policy management certificates using the Attestation client library.

## Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* ATTESTATION_AAD_URL - the base URL for an attestation service instance in AAD mode.
* ATTESTATION_ISOLATED_URL - the base URL for an attestation service instance in Isolated mode.
* ISOLATED_SIGNING_KEY - a Base64 encoded ASN.1 DER representation of a private key used when creating the 
ATTESTATION_ISOLATED_URL instance.
* ISOLATED_SIGNING_CERTIFICATE - a Base64 encoded DER X.509 certificate wrapping the public key of the ISOLATED_SIGNING_KEY.

Note: The Policy Management Certificate samples depend on the OpenSSL library to perform basic cryptographic 
operations on X.509 certificates.

## Samples descriptions

The samples are structured as separate source files, one per scenario. The are:
Sample | What it tests | Notes
-----|-----|-----
Get_Policy_Certificates| Enumerates the policy management certificates for the specified attestation service instance. |
Add_Policy_Certificates| Adds and removes a policy management certificates for the specified attestation service instance. |

## Additional Information

## Next Steps

For more information about the Microsoft Azure Attestation service, please see our [documentation page](https://docs.microsoft.com/azure/attestation/) .

<!-- LINKS -->
<!-- links are known to be broken, they will be fixed after this initial pull
    request completes. -->
[readme_md]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/attestation/azure-security-attestation/README.md
