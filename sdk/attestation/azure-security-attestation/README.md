# Azure Attestation Package client library for C++

Microsoft Azure Attestation is a unified solution for remotely verifying the trustworthiness of a platform and integrity of the binaries running inside it. The service supports attestation of the platforms backed by Trusted Platform Modules (TPMs) alongside the ability to attest to the state of Trusted Execution Environments (TEEs) such as Intel(tm) Software Guard Extensions (SGX) enclaves and Virtualization-based Security (VBS) enclaves.

Attestation is a process for demonstrating that software binaries were properly instantiated on a trusted platform. Remote 
relying parties can then gain confidence that only such intended software is running on trusted hardware. 

Azure Attestation enables cutting-edge security paradigms such as Azure Confidential computing and Intelligent Edge protection. Customers have been requesting the ability to independently verify the location of a machine, the posture of a virtual machine (VM) on that machine, and the environment within which enclaves are running on that VM. Azure Attestation will empower these and many additional customer requests.

Azure Attestation receives evidence from compute entities, turns them into a set of claims, validates them against configurable policies, and produces cryptographic proofs for claims-based applications (for example, relying parties and auditing authorities).

# Getting started
For the best development experience, we recommend that developers use the [CMake projects in Visual Studio](https://docs.microsoft.com/cpp/build/cmake-projects-in-visual-studio?view=vs-2019) to view and build the source code together with its dependencies.


## Prerequisites

- [Azure Subscription][azure_subscription]. Sign up for a [free trial](https://azure.microsoft.com/pricing/free-trial/) or use your [MSDN subscriber benefits](https://azure.microsoft.com/pricing/member-offers/msdn-benefits-details/).
- An existing [Azure Attestation instance][azure_attestation]. If you need to create an attestation instance, you can use the [Azure Cloud Shell][azure_cloud_shell] to create one with this Azure CLI command. Replace `<your-resource-group-name>` and `<your-instance-name>` with your own, unique names:

```bash
az attestation create --resource-group <your-resource-group-name> --name <your-service instance name>
```

## Download & Install

### Install Dependencies

#### Windows

On Windows, dependencies are managed by [vcpkg](https://github.com/microsoft/vcpkg). You can reference the [Quick Start](https://github.com/microsoft/vcpkg#quick-start-windows) to quickly set yourself up.
After Vcpkg is initialized and bootstrapped, you can install the dependencies:
```BatchFile
vcpkg.exe install curl:x64-windows-static
vcpkg.exe install openssl:x64-windows-static
```
#### POSIX Platforms

You can use the package manager on different POSIX platforms to install the dependencies. The dependencies to be installed are:

  - CMake 3.13.0 or higher.
  - OpenSSL.
  - libcurl.

### Build from Source

First, download the repository to your local folder:
```BatchFile
git clone https://github.com/Azure/azure-sdk-for-cpp.git
```

#### Windows

##### Use CMake to generate the solution file

In a new folder you created under the root directory:
```BatchFile
cmake .. -A x64 -DCMAKE_TOOLCHAIN_FILE=<YOUR_VCPKG_INSTALL_DIR>/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

The built library will be in `.\sdk\<ProjectDir>\<Configuration>\` respectively for Azure Core and Azure Attestation. e.g. `azure_core.lib` will be in `.\sdk\core\azure-core\Debug` for debug configuration.

##### Use Visual Studio's Open by folder feature
Open the root folder of the library with Visual Studio's Open folder feature.

If Vcpkg is not globally integrated, then you need to open CMakeSettings.json and change the `Make toolchain file to be <YOUR_VCPKG_INSTALL_DIR>/scripts/buildsystems/vcpkg.cmake` and save.
Then you can build Azure Storage libraries by selecting the target in Visual Studio, or simply build all.
The libraries will be in `<ProjectRoot>\out\build\<Configuration>\sdk\<LibraryName>` respectively.

#### POSIX Platforms

You can run the following command in a new folder created under the downloaded code's root folder to build the code.

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```
Then you can consume the built library with the header files.
make/ninja install is work in progress.


### Via NuGet
WIP
TODO when ready.

### Via vcpkg
The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Storage packages via vcpkg:

```cmd
> vcpkg install azure-security-attestation-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-security-attestation-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-security-attestation)

```

## Dependencies

  - [Azure Core SDK](https://github.com/Azure/azure-sdk-for-cpp/blob/main/README.md)
  - OpenSSL

### Authenticate the client

Many of the APIs supported by the Azure Attestation service require authentication (some do not, if an API does not require
authentication, the documentation for that API will reflect that the attestation service instance does not require authentication).

To interact with the authenticated APIs supported by the Azure Attestation service, your client must present an Azure Active Directory bearer token to the service.

The simplest way of providing a bearer token is to use the  `DefaultAzureCredential` authentication method by providing client secret credentials is being used in this getting started section, but you can find more ways to authenticate with [azure-identity][azure_identity].

# Key concepts

The Microsoft Azure Attestation service runs in two separate modes: "Isolated" and "AAD". When the service is running in "Isolated" mode, the customer needs to
provide additional information beyond their authentication credentials to verify that they are authorized to modify the state of an attestation instance.

There are four major client types provided in this SDK:

- [SGX and TPM enclave attestation.](#attestation)
- [MAA Attestation Token signing certificate discovery and validation.](#attestation-token-signing-certificate-discovery-and-validation)  
- [Attestation Policy management.](#policy-management)
- [Attestation policy management certificate management](#policy-management-certificate-management) (yes, policy management management).

Each attestation instance operates in one of three separate modes of operation:

- Isolated mode.
- AAD mode.
- Shared mode.

## Isolated Mode Attestation Instances.
In "Isolated" mode, the customer indicates that they want to ensure that Microsoft administrators cannot influence the inputs or outputs of an attestation service instance. When the attestation service instance is running in Isolated mode, the customer is expected to provide additional proof that they are authorized to make changes to the attestation service instance.

## AAD Mode Attestation Instances.

In "AAD" mode, access to the service is controlled solely by Azure Role Based Access Control. When the 

## Shared Mode Attestation Instances.
Each region in which the Microsoft Azure Attestation service is available supports a "shared" instance, which
can be used to attest SGX enclaves which only need verification against the azure baseline (there are no policies applied to the
shared instance). 

The following APIs are available in the shared instance:
- AttestSgxEnclave
- AttestOpenEnclave
- GetAttestationPolicy
- GetPolicyManagementCertificates (always returns an empty set)

The following APIs are not available in the shared instance:
- AttestTPMEnclave
- SetAttestationPolicy
- ResetAttestationPolicy
- AddPolicyManagementCertificate
- RemovePolicyManagementCertificate

The APIs available in the shared instance do not require AAD authentication.

## Attestation

SGX or TPM attestation is the process of validating evidence collected from a trusted execution environment to ensure that it 
meets both the Azure baseline for that environment and customer defined policies applied to that environment.

## Attestation token signing certificate discovery and validation

Most responses from the MAA service are expressed in the form of a JSON Web Token. This token will be signed by a signing certificate
issued by the MAA service for the specified instance. If the MAA service instance is running in a region where the service runs in an SGX enclave, then
the certificate issued by the server can be verified using the [oe_verify_attestation_certificate() API](https://openenclave.github.io/openenclave/api/enclave_8h_a3b75c5638360adca181a0d945b45ad86.html).

## Policy Management

Each attestation service instance has a policy applied to it which defines additional criteria which the customer has defined.

For more information on attestation policies, see [Attestation Policy](https://docs.microsoft.com/azure/attestation/author-sign-policy)

## Policy Management certificate management

When an attestation instance is running in "Isolated" mode, the customer who created the instance will have provided
a policy management certificate at the time the instance is created. All policy modification operations require that the customer sign
the policy data with one of the existing policy management certificates. The Policy Management Certificate Management APIs enable
clients to add, remove or enumerate the policy management certificates.

## Examples

- [Instantiate a synchronous attestation client](#create-a-synchronous-attestation-client)
- [Retrieve token validation certificates](#retrieve-token-certificates)
- [Attest an SGX enclave](#attest-an-sgx-enclave)
- [Instantiate a synchronous administrative client](#create-a-synchronous-administrative-client)
- [Get attestation policy](#retrieve-current-attestation-policy-for-openenclave)
- [Set unsigned attestation policy](#set-unsigned-attestation-policy-aad-clients-only)
- [Set signed attestation policy](#set-signed-attestation-policy)
- [List policy management certificates](#list-attestation-signing-certificates)
- [Add policy management certificate](#add-attestation-signing-certificate)

### Create a synchronous attestation client

The `AttestationClientBuilder` class is used to create instances of the attestation client:

```cpp readme-sample-create-synchronous-client
```

### Retrieve Token Certificates

Use `listAttestationSigners` to retrieve the set of certificates, which can be used to validate the token returned from the attestation service.
Normally, this information is not required as the attestation SDK will perform the validation as a part of the interaction with the
attestation service, however the APIs are provided for completeness and to facilitate customer's independently validating
attestation results.

```cpp readme-sample-getSigningCertificates
```

### Attest an SGX Enclave

Use the `AttestSgxEnclave` method to attest an SGX enclave.

```cpp readme-sample-attest-sgx-enclave
```

### Create a synchronous administrative client

All administrative clients are authenticated.

```cpp readme-sample-create-admin-client
AttestationAdministrationClientBuilder attestationBuilder = new AttestationAdministrationClientBuilder();
// Note that the "policy" calls require authentication.
AttestationAdministrationClient client = attestationBuilder
    .endpoint(endpoint)
    .credential(new DefaultAzureCredentialBuilder().build())
    .buildClient();
```

### Retrieve current attestation policy for OpenEnclave

Use the `getAttestationPolicy` API to retrieve the current attestation policy for a given TEE.

```java readme-sample-getCurrentPolicy
String currentPolicy = client.getAttestationPolicy(AttestationType.OPEN_ENCLAVE);
System.out.printf("Current policy for OpenEnclave is: %s\n", currentPolicy);
```

### Set unsigned attestation policy (AAD clients only)

When an attestation instance is in AAD mode, the caller can use a convenience method to set an unsigned attestation
policy on the instance.

```java readme-sample-set-unsigned-policy
// Set the listed policy on an attestation instance. Please note that this particular policy will deny all
// attestation requests and should not be used in production.
PolicyResult policyResult = client.setAttestationPolicy(AttestationType.OPEN_ENCLAVE,
    "version=1.0; authorizationrules{=> deny();}; issuancerules{};");
System.out.printf("Policy set for OpenEnclave result: %s\n", policyResult.getPolicyResolution());
```

### Set signed attestation policy

For isolated mode attestation instances, the set or reset policy request must be signed using the key that is associated
with the attestation signing certificates configured on the attestation instance.

```java readme-sample-set-signed-policy
// Set the listed policy on an attestation instance using a signed policy token.
PolicyResult policyResult = client.setAttestationPolicy(AttestationType.SGX_ENCLAVE,
    new AttestationPolicySetOptions()
        .setAttestationPolicy("version=1.0; authorizationrules{=> permit();}; issuancerules{};")
            .setAttestationSigner(new AttestationSigningKey(certificate, privateKey)));
System.out.printf("Policy set for Sgx result: %s\n", policyResult.getPolicyResolution());
```

### List attestation signing certificates

When an attestation instance is in `Isolated` mode, the policy APIs need additional proof of authorization. This proof is
provided via the `AttestationSigningKey` parameter passed into the set and reset policy APIs.

Each `Isolated` mode instance has a set of certificates, which determine whether a caller has the authority to set an
attestation policy. When an attestation policy is set, the client presents a signed "token" to the service, which is signed
by the key in the `AttestationSigningKey`. The signed token, including the certificate in the `AttestationSigningKey` is
sent to the attestation service, which verifies that the token was signed with the private key corresponding to the
public key in the token. The set or reset policy operation will only succeed if the certificate in the token is one of
the policy management tokens. This interaction ensures that the client is in possession of the private key associated with
one of the policy management certificates and is thus authorized to perform the operation.

```java readme-sample-listPolicyCertificates
AttestationSignerCollection signers = client.listPolicyManagementCertificates();
System.out.printf("Instance %s contains %d signers.\n", endpoint, signers.getAttestationSigners().size());
for (AttestationSigner signer : signers.getAttestationSigners()) {
    System.out.printf("Certificate Subject: %s", signer.getCertificates().get(0).getSubjectDN().toString());
}
```

### Add attestation signing certificate

Adds a new certificate to the set of policy management certificates. The request to add the policy management certificate
must be signed with the private key associated with one of the existing policy management certificates (this ensures that
the caller is authorized to update the set of policy certificates).

Note: Adding the same certificate twice is not considered an error - if the certificate is already present, the addition is
ignored (this possibly surprising behavior is there because retries could cause the addition to be executed multiple times)

```java readme-sample-addPolicyManagementCertificate
System.out.printf("Adding new certificate %s\n", certificateToAdd.getSubjectDN().toString());
PolicyCertificatesModificationResult modificationResult = client.addPolicyManagementCertificate(
    new PolicyManagementCertificateOptions(certificateToAdd,
        new AttestationSigningKey(isolatedCertificate, isolatedKey)));
System.out.printf("Updated policy certificate, certificate add result: %s\n",
    modificationResult.getCertificateResolution());
System.out.printf("Added certificate thumbprint: %s\n", modificationResult.getCertificateThumbprint());
```

### Remove attestation signing certificate

Removes a certificate from the set of policy management certificates. The request to remove the policy management certificate
must be signed with the private key associated with one of the existing policy management certificates (this ensures that
the caller is authorized to update the set of policy certificates).

Note: Removing a non-existent certificate is not considered an error - if the certificate is not present, the removal is
ignored (this possibly surprising behavior is there because retries could cause the removal to be executed multiple times)

```java readme-sample-removePolicyManagementCertificate
System.out.printf("Removing existing certificate %s\n", certificateToRemove.getSubjectDN().toString());
PolicyCertificatesModificationResult modificationResult = client.deletePolicyManagementCertificate(
    new PolicyManagementCertificateOptions(certificateToRemove,
        new AttestationSigningKey(isolatedCertificate, isolatedKey)));
System.out.printf("Updated policy certificate, certificate remove result: %s\n",
    modificationResult.getCertificateResolution());
System.out.printf("Removed certificate thumbprint: %s\n", modificationResult.getCertificateThumbprint());
```

## Troubleshooting

Troubleshooting information for the MAA service can be found [here](https://docs.microsoft.com/azure/attestation/troubleshoot-guide)

## Next steps

For more information about the Microsoft Azure Attestation service, please see our [documentation page](https://docs.microsoft.com/azure/attestation/).

## Contributing

For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].
This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor 
License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your
contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate 
the PR appropriately (e.g., label, comment). Simply follow the instructions provided by the bot. You will only need to
do this once across all repos using our CLA.

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
[style-guide-msft]: https://docs.microsoft.com/style-guide/capitalization
[azure_attestation]: https://docs.microsoft.com/azure/attestation
[azure_identity]: https://github.com/Azure/azure-sdk-for-java/tree/main/sdk/identity/azure-identity
[azure_subscription]: https://azure.microsoft.com/
[azure_cli]: https://docs.microsoft.com/cli/azure
[rest_api]: https://docs.microsoft.com/rest/api/attestation/
[azure_create_application_in_portal]: https://docs.microsoft.com/azure/active-directory/develop/howto-create-service-principal-portal
[azure_cloud_shell]: https://shell.azure.com/bash
[microsoft_code_of_conduct]: https://opensource.microsoft.com/codeofconduct/
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2Fsdk%2Fattestation%2Fazure-security-attestation%2FREADME.png)
