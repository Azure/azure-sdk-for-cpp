---
page_type: sample
languages:
- C++
products:
- azure
- azure-attestation
urlFragment: attestation-samples

---

# Attestation Samples for the Microsoft Azure Attestation client library for C++

These code samples show common scenario operations for the Attestation APIs within the Azure Attestation client library.

## Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* ATTESTATION_AAD_URL - the base URL for an attestation service instance in AAD mode.
* ATTESTATION_ISOLATED_URL - the base URL for an attestation service instance in Isolated mode.
* ATTESTATION_LOCATION_SHORT_NAME - the short name for the region in which the
  sample should be run - used to interact with the shared endpoint for that
  region.

## Samples descriptions

The samples are structured as separate source files, one per scenario. The are:
Sample | What it tests | Notes
-----|-----|-----
AttestSgxEnclave | The simplest usage of the AttestSgxEnclave API |
AttestOpenEnclaveShared | Attest an OpenEnclave report using the shared attestation instance |
AttestSgxEnclaveWithRuntimeBinary | Calling AttestSgxEnclave with RuntimeData sent to the service which should be interpreted as binary data |
AttestSgxEnclaveWithRuntimeJson | Calling AttestSgxEnclave with RuntimeData sent to the service which should be interpreted as JSON data |
AttestOpenEnclaveWithDraftPolicy | Calling AttestOpenEnclave with a draft attestation policy which can be used to test attestation policies to determine their effect |

## Additional Information

### Attestation Policy

An attestation policy is a document which defines authorization and claim generation
rules for attestation operations.

The following is an example of an attestation policy document for an SGX enclave:

```text
version= 1.0;
authorizationrules
{
    [ type=="x-ms-sgx-is-debuggable", value==false ] &&
    [ type=="x-ms-sgx-product-id", value==<product-id> ] &&
    [ type=="x-ms-sgx-svn", value>= 0 ] &&
    [ type=="x-ms-sgx-mrsigner", value=="<mrsigner>"]
        => permit();
};
issuancerules {
    c:[type=="x-ms-sgx-mrsigner"] => issue(type="<custom-name>", value=c.value);
};
```

There are two sections to the document: `authorizationrules` and `issuancerules`.
`authorizationrules` are rules which control whether an attestation token
should be issued. `issuancerules` are rules which cause claims to be issued in an
attestation token.

In the example, the attestation service will issue an attestation token if and only if
the SGX enclave is configured as follows:

* Not-Debuggable
* Enclave product ID: `<product-id>`.
* Enclave SVN: `<svn value>` greater or equal to zero.
* Enclave signer: matches `<mrsigner>`.

Assuming a token is issued, this policy will cause a claim named `<custom-name>`
to be issued with a value which matches the `x-ms-sgx-mrsigner` claim.

For more information on authoring attestation policy documents, see: [Authoring an attestation policy](https://docs.microsoft.com/azure/attestation/author-sign-policy)

## Next Steps

For more information about the Microsoft Azure Attestation service, please see our [documentation page](https://docs.microsoft.com/azure/attestation/) .

<!-- LINKS -->
<!-- links are known to be broken, they will be fixed after this initial pull
    request completes. -->
[readme_md]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/attestation/azure-security-attestation/README.md