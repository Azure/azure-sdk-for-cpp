// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/certificates/certificate_client.hpp"

using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure;
using namespace Azure::Core;

Response<KeyVaultCertificateWithPolicy> CertificateClient::GetCertificate(
    std::string const& name,
    Context const& context) const
{
  context.ThrowIfCancelled();
  (void)name;
  return Response<KeyVaultCertificateWithPolicy>({}, nullptr);
}