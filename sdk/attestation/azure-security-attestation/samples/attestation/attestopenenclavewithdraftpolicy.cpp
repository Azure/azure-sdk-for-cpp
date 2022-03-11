// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * - LOCATION_SHORT_NAME:  Specifies the short name of an Azure region to use for shared mode
 * operations.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account. Used for authenticated calls to the
 * attestation service.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request. Used for authenticated calls to
 * the attestation service.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request. Used for authenticated calls to
 * the attestation service.
 * - AZURE_CLIENT_SECRET: The client secret. Used for authenticated calls to the attestation
 * service.
 *
 */

#include "get_env.hpp"

#include "attestation_collateral.hpp"
#include <azure/attestation.hpp>
#include <azure/core/base64.hpp>
#include <azure/identity.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Core;

using namespace std::chrono_literals;
// cspell: words MRENCLAVE MRSIGNER

int main()
{
  try
  {
    std::cout << "In function: SampleAttestSgxEnclaveSimple" << std::endl;
    // create client
    AttestationClient attestationClient(std::getenv("ATTESTATION_AAD_URL"));

    std::vector<uint8_t> openEnclaveReport = AttestationCollateral::OpenEnclaveReport();

    AttestOptions options;
    options.DraftPolicyForAttestation = R"(version= 1.0;
authorizationrules
{
    [ type=="x-ms-sgx-is-debuggable", value==false ] &&
    [ type=="x-ms-sgx-product-id", value=="product-id" ] &&
    [ type=="x-ms-sgx-svn", value>= 0 ] 
        => permit();
};
issuancerules {
    c:[type=="x-ms-sgx-mrsigner"] => issue(type="custom-name", value=c.value);
};)";
    Azure::Response<AttestationToken<AttestationResult>> sgxResult
        = attestationClient.AttestOpenEnclave(openEnclaveReport, options);

    std::cout << "SGX Quote MRSIGNER is: "
              << Convert::Base64Encode(*sgxResult.Value.Body.SgxMrSigner) << std::endl;
    std::cout << "SGX Quote MRENCLAVE is: "
              << Convert::Base64Encode(*sgxResult.Value.Body.SgxMrEnclave) << std::endl;

    std::cout << "Policy claims: " << *sgxResult.Value.Body.PolicyClaims << std::endl;
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Request Failed Exception happened:" << std::endl << e.what() << std::endl;
    if (e.StatusCode != Azure::Core::Http::HttpStatusCode::None)
    {
      std::cout << "Error Code: " << e.ErrorCode << std::endl;
      std::cout << "Error Message: " << e.Message << std::endl;
    }
    return 0;
  }
}
