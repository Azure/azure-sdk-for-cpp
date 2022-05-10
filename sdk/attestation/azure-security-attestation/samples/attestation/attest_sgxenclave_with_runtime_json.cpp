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
 *
 */

#include <get_env.hpp>

#include "attestation_collateral.hpp"
#include <azure/attestation.hpp>
#include <azure/core/base64.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Core;

using namespace std::chrono_literals;

int main()
{
  try
  {
    std::cout << "In function: SampleAttestSgxEnclaveWithJSONRuntimeData" << std::endl;

    // create client
    std::string const endpoint(std::getenv("ATTESTATION_AAD_URL"));
    AttestationClient const attestationClient(AttestationClient::Create(endpoint));

    std::vector<uint8_t> const sgxEnclaveQuote = AttestationCollateral::SgxQuote();

    // Set the RunTimeData in the request to the service. Ask the service to interpret the
    // RunTimeData as a JSON object when it is returned in the resulting token.
    AttestEnclaveOptions attestOptions;

    attestOptions.RunTimeData
        = AttestationData{AttestationCollateral::RunTimeData(), AttestationDataType::Json};

    Azure::Response<AttestationToken<AttestationResult>> const sgxResult
        = attestationClient.AttestSgxEnclave(sgxEnclaveQuote, attestOptions);

    std::cout << "SGX Quote MRSIGNER is: "
              << Convert::Base64Encode(*sgxResult.Value.Body.SgxMrSigner) << std::endl;
    std::cout << "SGX Quote MRENCLAVE is: "
              << Convert::Base64Encode(*sgxResult.Value.Body.SgxMrEnclave) << std::endl;

    std::cout << "Attestation Token runtimeData is " << *sgxResult.Value.Body.RunTimeClaims
              << std::endl;
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
