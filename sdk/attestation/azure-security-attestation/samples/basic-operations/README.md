---
page_type: sample
languages:
- C++
products:
- azure
- azure-attestation
urlFragment: attestation-samples

---

# _Attestation Samples for the Microsoft Azure Attestation client library for C++

These code samples show basic and low level operations using the Azure Attestation client library.

## _Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* ATTESTATION_AAD_URL - the base URL for an attestation service instance in AAD mode.
* ATTESTATION_ISOLATED_URL - the base URL for an attestation service instance in Isolated mode.
* ATTESTATION_LOCATION_SHORT_NAME - the short name for the region in which the
  sample should be run - used to interact with the shared endpoint for that
  region.

## _Samples descriptions

The samples are structured as separate source files, one per scenario. The are:
Sample | What it tests | Notes
-----|-----|-----
CreateClient | Demonstrates creating an Attestation client. |
CreateAdminClient | Demonstrates creating an Attestation Administration client, which can be used to modify attestation policies. |
GetOpenIdMetadata | Retrieves OpenID metadata for an attestation service instance.|
GetSigningCertificates | Retrieves the attestation signing certificates for an attestation instance. |

## _Additional Information

<!-- LINKS -->
<!-- links are known to be broken, they will be fixed after this initial pull
    request completes. -->
[readme_md]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/attestation/azure-security-attestation/README.md
