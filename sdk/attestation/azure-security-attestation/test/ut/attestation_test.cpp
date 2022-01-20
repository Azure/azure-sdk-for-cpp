// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include <azure/core/test/test_base.hpp>
#include <azure/attestation/attestation_client.hpp>
#include <azure/identity/client_secret_credential.hpp>

#include "attestation_collateral.hpp"

using namespace Azure::Security::Attestation;

namespace Azure { namespace Security { namespace Attestation { namespace Test {

class AttestationTests : public Azure::Core::Test::TestBase,
                           public testing::WithParamInterface<std::string> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      if (GetParam() == "Shared")
      {
        m_endpoint = "https://sharedwus.wus.attest.azure.net";
      }
      else if (GetParam() == "Aad")
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (GetParam() == "Isolated")
      {
        m_endpoint = GetEnv("ATTESTATION_ISOLATED_URL");
      }

    }

    std::unique_ptr<AttestationClient> CreateClient() {
        // `InitTestClient` takes care of setting up Record&Playback.
      auto options = InitClientOptions<Azure::Security::Attestation::AttestationClientOptions>();
      return std::make_unique<
          Azure::Security::Attestation::AttestationClient>(
          m_endpoint, options);
    }
    std::unique_ptr<AttestationClient> CreateAuthenticatedClient()
    {
      // `InitClientOptions` takes care of setting up Record&Playback.
      auto options = InitClientOptions<Azure::Security::Attestation::AttestationClientOptions>();
      auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return std::make_unique<AttestationClient>(
          m_endpoint, credential, options);
    }
};

  TEST_P(AttestationTests, GetOpenIdMetadata) 
  {
    auto attestationClient(CreateClient());

    auto openIdMetadata = attestationClient->GetOpenIdMetadata();

    EXPECT_FALSE(openIdMetadata.Value.Issuer.empty());
    EXPECT_FALSE(openIdMetadata.Value.JsonWebKeySetUrl.empty());
    EXPECT_EQ(m_endpoint, openIdMetadata.Value.Issuer);
    EXPECT_EQ(0UL, openIdMetadata.Value.JsonWebKeySetUrl.find(openIdMetadata.Value.Issuer));
    EXPECT_EQ(m_endpoint + "/certs", openIdMetadata.Value.JsonWebKeySetUrl);
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedClaims.size());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedResponseTypes.size());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedTokenSigningAlgorithms.size());
  }

  TEST_P(AttestationTests, GetSigningCertificates)
  {
    auto attestationClient(CreateClient());

    auto attestationSigners = attestationClient->GetAttestationSigningCertificates();
    EXPECT_LE(1UL, attestationSigners.Value.size());
    for (const auto& signer : attestationSigners.Value)
    {
      EXPECT_FALSE(signer.KeyId.empty());
      EXPECT_LE(1UL, signer.CertificateChain.size());
      for (const auto& cert : signer.CertificateChain)
      {
        EXPECT_EQ(0UL, cert.find("-----BEGIN CERTIFICATE-----\r\n"));
      }
    }
  }

  TEST_P(AttestationTests, SimpleAttestOpenEnclave)
  {
    auto client(CreateClient());
    auto report = AttestationCollateral::OpenEnclaveReport();

    auto attestResponse = client->AttestOpenEnclave(report);
  }

  TEST_P(AttestationTests, SimpleAttestSgxEnclave)
  {
    auto client(CreateClient());
    auto sgxQuote = AttestationCollateral::SgxQuote();

    auto attestResponse = client->AttestSgxEnclave(sgxQuote);
  }

  TEST_P(AttestationTests, AttestOpenEnclaveWithRuntimeData)
  {
    auto client(CreateClient());
    auto report = AttestationCollateral::OpenEnclaveReport();
    auto runtimeData = AttestationCollateral::RuntimeData();

    auto attestResponse
        = client->AttestOpenEnclave(report, {{runtimeData, DataType::Binary}});
  }

  TEST_P(AttestationTests, AttestSgxEnclaveWithRuntimeData)
  {
    auto client(CreateClient());
    auto sgxQuote = AttestationCollateral::SgxQuote();
    auto runtimeData = AttestationCollateral::RuntimeData();

    auto attestResponse
        = client->AttestSgxEnclave(sgxQuote, {{runtimeData, DataType::Binary}});
  }


  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<std::string>& info)
    {
      return info.param;
    }
  }
  
  INSTANTIATE_TEST_SUITE_P(
      Attestation,
      AttestationTests,
      ::testing::Values("Shared", "Aad", "Isolated"),
      GetSuffix);

}}}} // namespace Azure::Security::Attestation::Test
