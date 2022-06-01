// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 * - ISOLATED_SIGNING_KEY: A Base64 encoded DER encoded RSA private key which matches the private
 * key used when creating the ATTESTATION_ISOLATED_URL.
 * - ISOLATED_SIGNING_CERTIFICATE: A Base64 encoded X.509 certificate wrapping the public key of the
 * ISOLATED_SIGNING_KEY
 *
 */

#include <get_env.hpp>

#include "cryptohelpers.hpp"
#include <azure/attestation.hpp>
#include <azure/core/base64.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/identity.hpp>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// cspell:: words mrsigner mrenclave mitm
using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;
using namespace Azure::Core;
using namespace Azure::Core::Cryptography::_internal;

int main()
{
  try
  {

    // Attestation tokens returned by the service should be issued by the
    // attestation service instance. Update the token validation logic to ensure that
    // the right instance issued the token we received (this protects against a MITM responding
    // with a token issued by a different attestation service instance).
    std::string const endpoint(GetEnvHelper::GetEnv("ATTESTATION_ISOLATED_URL"));

    AttestationAdministrationClientOptions clientOptions;
    clientOptions.TokenValidationOptions.ExpectedIssuer = endpoint;
    clientOptions.TokenValidationOptions.ValidateIssuer = true;

    // Ten seconds of clock drift are allowed between this machine and the attestation service.
    clientOptions.TokenValidationOptions.TimeValidationSlack = 10s;

    // create client
    auto const credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        GetEnvHelper::GetEnv("AZURE_TENANT_ID"),
        GetEnvHelper::GetEnv("AZURE_CLIENT_ID"),
        GetEnvHelper::GetEnv("AZURE_CLIENT_SECRET"));
    std::shared_ptr<AttestationAdministrationClient const> adminClient(
        AttestationAdministrationClientFactory::Create(endpoint, credential, clientOptions));

    std::string const signingKey(GetEnvHelper::GetEnv("ISOLATED_SIGNING_KEY"));
    std::string const signingCert(GetEnvHelper::GetEnv("ISOLATED_SIGNING_CERTIFICATE"));

    // The attestation APIs expect a PEM encoded key and certificate, so convert the Base64 key and
    // certificate to PEM encoded equivalents.
    std::string const pemSigningKey(::Cryptography::PemFromBase64(signingKey, "PRIVATE KEY"));
    std::string const pemSigningCert(::Cryptography::PemFromBase64(signingCert, "CERTIFICATE"));

    std::string const policyToSet(R"(version= 1.0;
authorizationrules 
{
	[ type=="x-ms-sgx-is-debuggable", value==true ]&&
	[ type=="x-ms-sgx-mrsigner", value=="mrsigner1"] => permit(); 
	[ type=="x-ms-sgx-is-debuggable", value==true ]&& 
	[ type=="x-ms-sgx-mrsigner", value=="mrsigner2"] => permit(); 
};)");

    // When setting attestation policy, use the signing key associated with the isolated instance.
    SetPolicyOptions setOptions;
    setOptions.SigningKey = AttestationSigningKey{pemSigningKey, pemSigningCert};

    Azure::Response<AttestationToken<PolicyResult>> const setResult
        = adminClient->SetAttestationPolicy(AttestationType::SgxEnclave, policyToSet, setOptions);

    if (setResult.Value.Body.PolicyResolution == PolicyModification::Updated)
    {
      std::cout << "Attestation policy was updated." << std::endl;
    }

    // To verify that the attestation service received the attestation policy, the service returns
    // the SHA256 hash of the policy token which was sent ot the service. To simplify the customer
    // experience of interacting with the SetPolicy APIs, CreateAttestationPolicyToken API will
    // generate the same token that would be send to the service.
    //
    // To ensure that the token which was sent from the client matches the token which was received
    // by the attestation service, the customer can call CreateAttestationPolicyToken and then
    // generate the SHA256 of that token and compare it with the value returned by the service - the
    // two hash values should be identical.
    auto const setPolicyToken
        = adminClient->CreateAttestationPolicyToken(policyToSet, setOptions.SigningKey);
    Sha256Hash shaHasher;
    std::vector<uint8_t> const policyTokenHash = shaHasher.Final(
        reinterpret_cast<uint8_t const*>(setPolicyToken.RawToken.data()),
        setPolicyToken.RawToken.size());
    std::cout << "Expected token hash: " << Convert::Base64Encode(policyTokenHash) << std::endl;
    std::cout << "Actual token hash:   "
              << Convert::Base64Encode(setResult.Value.Body.PolicyTokenHash) << std::endl;

    // When a policy signer is specified in the Set request, then the signer will be present in the
    // response. This allows a caller to verify that the signing certificate has not been tampered
    // with in transition.
    assert(setResult.Value.Body.PolicySigner);
    if (pemSigningCert == (*setResult.Value.Body.PolicySigner->CertificateChain)[0])
    {
      std::cout << "Policy signer returned matches policy signer set, the request was not tampered."
                << std::endl;
    }
    else
    {
      std::cout << "Policy signer returned DOES NOT match policy signer set." << std::endl;
    }
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Request Failed Exception happened:" << std::endl << e.what() << std::endl;
    if (e.RawResponse)
    {
      std::cout << "Error Code: " << e.ErrorCode << std::endl;
      std::cout << "Error Message: " << e.Message << std::endl;
    }
    return 1;
  }
  return 0;
}
