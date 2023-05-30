// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "attestation_collateral.hpp"
#include "azure/attestation/attestation_client.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>

#include <tuple>

#include <gtest/gtest.h>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Core;

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  enum class InstanceType
  {
    Shared,
    AAD,
    Isolated
  };

  class MetadataTests : public Azure::Core::Test::TestBase,
                        public testing::WithParamInterface<InstanceType> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

      InstanceType instanceType(GetParam());
      if (instanceType == InstanceType::Shared)
      {
        std::string shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        m_endpoint = "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (instanceType == InstanceType::AAD)
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (instanceType == InstanceType::Isolated)
      {
        m_endpoint = GetEnv("ATTESTATION_ISOLATED_URL");
      }
    }

    AttestationClient CreateClient()
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      auto options = InitClientOptions<Azure::Security::Attestation::AttestationClientOptions>();
      return AttestationClient::Create(m_endpoint, options);
    }

    AttestationClient CreateAuthenticatedClient()
    {
      // `InitClientOptions` takes care of setting up Record&Playback.
      AttestationClientOptions options = InitClientOptions<AttestationClientOptions>();
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return AttestationClient::Create(m_endpoint, credential, options);
    }
  };

  TEST_P(MetadataTests, GetOpenIdMetadata)
  {
    auto attestationClient(CreateClient());

    auto openIdMetadata = attestationClient.GetOpenIdMetadata();

    EXPECT_TRUE(openIdMetadata.Value.Issuer);
    EXPECT_TRUE(openIdMetadata.Value.JsonWebKeySetUrl);
    if (!m_testContext.IsPlaybackMode())
    {
      EXPECT_EQ(m_endpoint, *openIdMetadata.Value.Issuer);
    }
    EXPECT_EQ(0UL, openIdMetadata.Value.JsonWebKeySetUrl->find(*openIdMetadata.Value.Issuer));
    EXPECT_EQ(*openIdMetadata.Value.Issuer + "/certs", *openIdMetadata.Value.JsonWebKeySetUrl);
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedClaims->size());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedResponseTypes->size());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedTokenSigningAlgorithms->size());
  }

  TEST_P(MetadataTests, GetSigningCertificates)
  {
    auto attestationClient(CreateClient());

    auto attestationSigners = attestationClient.GetTokenValidationCertificates();
    EXPECT_LE(1UL, attestationSigners.Value.Signers.size());
    for (const auto& signer : attestationSigners.Value.Signers)
    {
      EXPECT_TRUE(signer.KeyId);
      EXPECT_LE(1UL, signer.CertificateChain->size());
      for (const auto& cert : *signer.CertificateChain)
      {
        EXPECT_EQ(0UL, cert.find("-----BEGIN CERTIFICATE-----\r\n"));
      }
    }
  }

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<MetadataTests::ParamType>& info)
    {
      std::string rv;
      switch (info.param)
      {
        case InstanceType::Shared:
          rv += "Shared";
          break;
        case InstanceType::AAD:
          rv += "Aad";
          break;
        case InstanceType::Isolated:
          rv += "Isolated";
          break;
        default:
          throw std::runtime_error("Unknown instance type");
      }
      return rv;
    }
  } // namespace

  INSTANTIATE_TEST_SUITE_P(
      Attestation,
      MetadataTests,
      ::testing::Values(InstanceType::Shared, InstanceType::AAD, InstanceType::Isolated),
      GetSuffix);

}}}} // namespace Azure::Security::Attestation::Test
