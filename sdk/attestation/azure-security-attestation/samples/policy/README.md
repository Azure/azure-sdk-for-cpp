---
page_type: sample
languages:
- C++
products:
- azure
- azure-attestation
urlFragment: attestation-samples

---

# Policy Samples for the Microsoft Azure Attestation client library for C++

These code samples show how to set and reset attestation policies using the Attestation client library.

## Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* ATTESTATION_AAD_URL - the base URL for an attestation service instance in AAD mode.
* ATTESTATION_ISOLATED_URL - the base URL for an attestation service instance in Isolated mode.
* ISOLATED_SIGNING_KEY - a Base64 encoded ASN.1 DER representation of a private key used when creating the 
ATTESTATION_ISOLATED_URL instance.
* ISOLATED_SIGNING_CERTIFICATE - a Base64 encoded DER X.509 certificate wrapping the public key of the ISOLATED_SIGNING_KEY.

## Samples descriptions

The samples are structured as separate source files, one per scenario. The are:
Sample | What it tests | Notes
-----|-----|-----
Get_Policy | Retrieves the attestation policy for a specific attestation instance. |
Set_Policy | Sets an attestation policy on an AAD attestation instance. | Note: The policy being set in this sample is unsigned.
Set_Sealed_Policy | Sets an attestation policy on an isolated attestation instance | Note: This sample requires the ISOLATED_ environment variables.
Reset_Policy | Resets the attestation policy for an AAD instance to the default value for the attestation type. |
Reset_Sealed_Policy | Resets an attestation policy to the default value on an isolated attestation instance | Note: This sample requires the ISOLATED_ environment variables.

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
