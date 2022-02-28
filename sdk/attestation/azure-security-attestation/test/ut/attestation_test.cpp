// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "attestation_collateral.hpp"
#include "azure/attestation/attestation_client.hpp"
#include <azure/core/internal/json/json.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <gtest/gtest.h>
#include <tuple>

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

  class AttestationTests
      : public Azure::Core::Test::TestBase,
        public testing::WithParamInterface<std::tuple<InstanceType, AttestationType>> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

      InstanceType instanceType(std::get<0>(GetParam()));
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
      AttestationClientOptions options;
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        options.TokenValidationOptions.ValidateNotBeforeTime = false;
        options.TokenValidationOptions.ValidateExpirationTime = false;
      }
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return InitTestClient<AttestationClient, AttestationClientOptions>(
          m_endpoint, credential, options);
    }

    void ValidateAttestResponse(
        Azure::Response<AttestationToken<AttestationResult>> const& response,
        Azure::Nullable<AttestationData> data = Azure::Nullable<AttestationData>())
    {
      EXPECT_TRUE(response.Value.Issuer);
      if (!m_testContext.IsPlaybackMode())
      {
        EXPECT_EQ(m_endpoint, response.Value.Issuer.Value());
      }
      EXPECT_TRUE(response.Value.Body.SgxMrEnclave);
      EXPECT_TRUE(response.Value.Body.SgxMrSigner);
      EXPECT_TRUE(response.Value.Body.SgxSvn);
      EXPECT_TRUE(response.Value.Body.SgxProductId);
      if (data)
      {
        if (data.Value().DataType == AttestationDataType::Json)
        {
          EXPECT_TRUE(response.Value.Body.RuntimeClaims);
          EXPECT_FALSE(response.Value.Body.EnclaveHeldData);
          // canonicalize the JSON sent to the service before checking with the service output.
          auto sentJson(Azure::Core::Json::_internal::json::parse(data.Value().Data));
          EXPECT_EQ(sentJson.dump(), response.Value.Body.RuntimeClaims.Value());
        }
        else
        {
          EXPECT_FALSE(response.Value.Body.RuntimeClaims);
          EXPECT_TRUE(response.Value.Body.EnclaveHeldData);
          // If we expected binary, the EnclaveHeldData in the response should be the value sent.
          EXPECT_EQ(data.Value().Data, response.Value.Body.EnclaveHeldData.Value());

        }
      }
    }
  };

  TEST_P(AttestationTests, GetOpenIdMetadata)
  {
    auto attestationClient(CreateClient());

    EXPECT_FALSE(attestationClient->ClientVersion().empty());

    auto openIdMetadata = attestationClient->GetOpenIdMetadata();

    EXPECT_TRUE(openIdMetadata.Value.Issuer.HasValue());
    EXPECT_TRUE(openIdMetadata.Value.JsonWebKeySetUrl.HasValue());
    if (!m_testContext.IsPlaybackMode())
    {
      EXPECT_EQ(m_endpoint, openIdMetadata.Value.Issuer.Value());
    }
    EXPECT_EQ(
        0UL,
        openIdMetadata.Value.JsonWebKeySetUrl.Value().find(openIdMetadata.Value.Issuer.Value()));
    EXPECT_EQ(
        openIdMetadata.Value.Issuer.Value() + "/certs",
        openIdMetadata.Value.JsonWebKeySetUrl.Value());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedClaims.Value().size());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedResponseTypes.Value().size());
    EXPECT_NE(0UL, openIdMetadata.Value.SupportedTokenSigningAlgorithms.Value().size());
  }

  TEST_P(AttestationTests, GetSigningCertificates)
  {
    auto attestationClient(CreateClient());

    auto attestationSigners = attestationClient->GetAttestationSigningCertificates();
    EXPECT_LE(1UL, attestationSigners.Value.Signers.size());
    for (const auto& signer : attestationSigners.Value.Signers)
    {
      EXPECT_TRUE(signer.KeyId.HasValue());
      EXPECT_LE(1UL, signer.CertificateChain.Value().size());
      for (const auto& cert : signer.CertificateChain.Value())
      {
        EXPECT_EQ(0UL, cert.find("-----BEGIN CERTIFICATE-----\r\n"));
      }
    }
  }

  TEST_P(AttestationTests, SimpleAttest)
  {
    auto client(CreateClient());

    if (std::get<1>(GetParam()) == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();
      auto attestResponse = client->AttestOpenEnclave(report);
      ValidateAttestResponse(attestResponse);
    }
    else if (std::get<1>(GetParam()) == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
      auto attestResponse = client->AttestSgxEnclave(quote);
      ValidateAttestResponse(attestResponse);
    }

    auto report = AttestationCollateral::OpenEnclaveReport();
    auto attestResponse = client->AttestOpenEnclave(report);
  }

  TEST_P(AttestationTests, SimpleAttestSgxEnclave)
  {
    auto client(CreateClient());
    auto sgxQuote = AttestationCollateral::SgxQuote();

    auto attestResponse = client->AttestSgxEnclave(sgxQuote);
  }

  TEST_P(AttestationTests, AttestWithRuntimeData)
  {
    auto client(CreateClient());
    auto runtimeData = AttestationCollateral::RuntimeData();

    if (std::get<1>(GetParam()) == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();
      AttestationData data{runtimeData, AttestationDataType::Binary};
      auto attestResponse = client->AttestOpenEnclave(report, {data});
      ValidateAttestResponse(attestResponse, data);
    }
    else if (std::get<1>(GetParam()) == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
      AttestationData data{runtimeData, AttestationDataType::Binary};
      auto attestResponse = client->AttestSgxEnclave(quote, {data});
      ValidateAttestResponse(attestResponse, data);
    }
  }
  TEST_P(AttestationTests, AttestWithRuntimeDataJson)
  {
    auto client(CreateClient());
    auto runtimeData = AttestationCollateral::RuntimeData();

    if (std::get<1>(GetParam()) == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();
      AttestationData data{runtimeData, AttestationDataType::Json};
      auto attestResponse = client->AttestOpenEnclave(report, {data});
      ValidateAttestResponse(attestResponse, data);
    }
    else if (std::get<1>(GetParam()) == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
      AttestationData data{runtimeData, AttestationDataType::Json};
      auto attestResponse = client->AttestSgxEnclave(quote, {data});
      ValidateAttestResponse(attestResponse, data);
    }
  }

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<AttestationTests::ParamType>& info)
    {
      std::string rv = std::get<1>(info.param).ToString();
      rv += "_";
      switch (std::get<0>(info.param))
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
      AttestationTests,
      ::testing::Combine(
          ::testing::Values(InstanceType::Shared, InstanceType::AAD, InstanceType::Isolated),
          ::testing::Values(AttestationType::OpenEnclave, AttestationType::SgxEnclave)),
      GetSuffix);

}}}} // namespace Azure::Security::Attestation::Test
