// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * - LOCATION_SHORT_NAME:  Specifies the short name of an Azure region to use for shared mode operations.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/identity.hpp>
#include <azure/attestation.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;

void SampleGetOpenIdMetadata()
{
  AttestationClientOptions clientOptions;
  // create client
  AttestationClient attestationClient(std::getenv("ATTESTATION_AAD_URL"), clientOptions);

// Retrieve the OpenId metadata from this attestation service instance.
Azure::Response<AttestationOpenIdMetadata> openIdMetadata = attestationClient.GetOpenIdMetadata();
std::cout << "Attestation Certificate Endpoint is: " << openIdMetadata.Value.JsonWebKeySetUrl.Value() << std::endl;


}

int main()
{
  try
  {
    SampleGetOpenIdMetadata();
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Attestation Client Exception happened:" << std::endl
              << e.Message << std::endl;
    return 1;
  }
  return 0;
}
