// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Attestation SDK client
 * for C++ to retrieve the OpenID metadata for an endpoint..
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * operations.
 *
 */

#include <get_env.hpp>

#include <azure/attestation.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;

int main()
{
  try
  {
    // create client
    std::unique_ptr<AttestationClient const> attestationClient(
        AttestationClientFactory::Create(std::getenv("ATTESTATION_AAD_URL")));

    // Retrieve the OpenId metadata from this attestation service instance.
    Azure::Response<OpenIdMetadata> const openIdMetadata = attestationClient->GetOpenIdMetadata();
    std::cout << "Attestation Certificate Endpoint is: " << *openIdMetadata.Value.JsonWebKeySetUrl
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
