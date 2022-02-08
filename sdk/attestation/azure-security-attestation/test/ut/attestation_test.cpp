// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_client.hpp"
#include "azure/core/test/test_base.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include <gtest/gtest.h>

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

      std::string mode(GetParam());
      if (mode == "Shared")
      {
        std::string shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        m_endpoint = "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (mode == "Aad")
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (mode == "Isolated")
      {
        m_endpoint = GetEnv("ATTESTATION_ISOLATED_URL");
      }
    }

    std::unique_ptr<AttestationClient> CreateClient()
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      auto options = InitClientOptions<Azure::Security::Attestation::AttestationClientOptions>();
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        options.TokenValidationOptions.ValidateNotBeforeTime = false;
        options.TokenValidationOptions.ValidateExpirationTime = false;
      }
      return std::make_unique<Azure::Security::Attestation::AttestationClient>(m_endpoint, options);
    }
    std::unique_ptr<AttestationClient> CreateAuthenticatedClient()
    {
      // `InitClientOptions` takes care of setting up Record&Playback.
      auto options = InitClientOptions<Azure::Security::Attestation::AttestationClientOptions>();
      auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return std::make_unique<AttestationClient>(m_endpoint, credential, options);
    }
  };

  TEST_P(AttestationTests, GetOpenIdMetadata)
  {
    auto attestationClient(CreateClient());

    EXPECT_FALSE(attestationClient->ClientVersion().empty());

    auto openIdMetadata = attestationClient->GetOpenIdMetadata();

    EXPECT_FALSE(openIdMetadata.Value.Issuer.empty());
    EXPECT_FALSE(openIdMetadata.Value.JsonWebKeySetUrl.empty());
    if (!m_testContext.IsPlaybackMode())
    {
      EXPECT_EQ(m_endpoint, openIdMetadata.Value.Issuer);
    }
    EXPECT_EQ(0UL, openIdMetadata.Value.JsonWebKeySetUrl.find(openIdMetadata.Value.Issuer));
    EXPECT_EQ(openIdMetadata.Value.Issuer + "/certs", openIdMetadata.Value.JsonWebKeySetUrl);
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
      EXPECT_TRUE(signer.KeyId.HasValue());
      EXPECT_LE(1UL, signer.CertificateChain.Value().size());
      for (const auto& cert : signer.CertificateChain.Value())
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

    auto attestResponse = client->AttestOpenEnclave(
        report, {AttestationData{runtimeData, AttestationDataType::Binary}});
  }

  TEST_P(AttestationTests, AttestSgxEnclaveWithRuntimeData)
  {
    auto client(CreateClient());
    auto sgxQuote = AttestationCollateral::SgxQuote();
    auto runtimeData = AttestationCollateral::RuntimeData();

    auto attestResponse = client->AttestSgxEnclave(
        sgxQuote, {AttestationData{runtimeData, AttestationDataType::Binary}});
  }

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<std::string>& info)
    {
      return info.param;
    }
  } // namespace

  INSTANTIATE_TEST_SUITE_P(
      Attestation,
      AttestationTests,
      ::testing::Values("Shared", "Aad", "Isolated"),
      GetSuffix);

}}}} // namespace Azure::Security::Attestation::Test
