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
using namespace std::chrono_literals;

namespace Azure { namespace Security { namespace Attestation { namespace Samples {

  /** @brief Simple call to attestation specifying a predefined SGX quote.
   *
   * Note that calls to AttestSgxEnclave are anonymous and do not require authentication.
   */
  void SampleAttestSgxEnclaveSimple()
  {
    std::cout << "In function: " __FUNCTION__ << std::endl;
    AttestationClientOptions clientOptions;
    // create client
    AttestationClient attestationClient(std::getenv("ATTESTATION_AAD_URL"), clientOptions);

    std::vector<uint8_t> sgxEnclaveQuote = AttestationCollateral::SgxQuote();

    Azure::Response<AttestationToken<AttestationResult>> sgxResult
        = attestationClient.AttestSgxEnclave(sgxEnclaveQuote);

    std::cout << "SGX Quote MRSIGNER is: "
              << Azure::Core::Convert::Base64Encode(sgxResult.Value.Body.SgxMrSigner.Value())
              << std::endl;
    std::cout << "SGX Quote MRENCLAVE is: "
              << Azure::Core::Convert::Base64Encode(sgxResult.Value.Body.SgxMrEnclave.Value())
              << std::endl;
  }

  /** @brief Simple call to attestation specifying a predefined SGX quote.
   *
   * Note that calls to AttestSgxEnclave are anonymous and do not require authentication.
   */
  void SampleAttestSgxEnclaveWithJSONRuntimeData()
  {
    std::cout << "In function: " __FUNCTION__ << std::endl;
    AttestationClientOptions clientOptions;
    // create client
    AttestationClient attestationClient(std::getenv("ATTESTATION_AAD_URL"), clientOptions);

    std::vector<uint8_t> sgxEnclaveQuote = AttestationCollateral::SgxQuote();

    // Set the RuntimeData in the request to the service. Ask the service to interpret the
    // RuntimeData as a JSON object when it is returned in the resulting token.
    AttestOptions attestOptions;
    attestOptions.RuntimeData
        = AttestationData{AttestationCollateral::RuntimeData(), AttestationDataType::Json};

    Azure::Response<AttestationToken<AttestationResult>> sgxResult
        = attestationClient.AttestSgxEnclave(sgxEnclaveQuote, attestOptions);

    std::cout << "SGX Quote MRSIGNER is: "
              << Azure::Core::Convert::Base64Encode(sgxResult.Value.Body.SgxMrSigner.Value())
              << std::endl;
    std::cout << "SGX Quote MRENCLAVE is: "
              << Azure::Core::Convert::Base64Encode(sgxResult.Value.Body.SgxMrEnclave.Value())
              << std::endl;

    std::cout << "Attestation Token runtimeData is " << sgxResult.Value.Body.RuntimeClaims.Value()
              << std::endl;
  }

}}}} // namespace Azure::Security::Attestation::Samples

int main()
{
  try
  {
    Azure::Security::Attestation::Samples::SampleAttestSgxEnclaveSimple();
    Azure::Security::Attestation::Samples::SampleAttestSgxEnclaveWithJSONRuntimeData();
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Attestation Client Exception happened:" << std::endl << e.Message << std::endl;
    return 1;
  }
  return 0;
}
