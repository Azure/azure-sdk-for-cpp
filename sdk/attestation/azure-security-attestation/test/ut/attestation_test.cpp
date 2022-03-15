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
        Azure::Nullable<AttestationData> data = {},
        std::string draftPolicy = {})
    {
      EXPECT_TRUE(response.Value.Issuer);
      if (!m_testContext.IsPlaybackMode())
      {
        EXPECT_EQ(m_endpoint, *response.Value.Issuer);
      }

      EXPECT_TRUE(response.Value.Body.SgxMrEnclave);
      EXPECT_TRUE(response.Value.Body.SgxMrSigner);
      EXPECT_TRUE(response.Value.Body.SgxSvn);
      EXPECT_TRUE(response.Value.Body.SgxProductId);
      if (data)
      {
        if (data->DataType == AttestationDataType::Json)
        {
          EXPECT_TRUE(response.Value.Body.RuntimeClaims);
          EXPECT_FALSE(response.Value.Body.EnclaveHeldData);
          // canonicalize the JSON sent to the service before checking with the service output.
          auto sentJson(Azure::Core::Json::_internal::json::parse(data->Data));
          EXPECT_EQ(sentJson.dump(), *response.Value.Body.RuntimeClaims);
        }
        else
        {
          EXPECT_FALSE(response.Value.Body.RuntimeClaims);
          EXPECT_TRUE(response.Value.Body.EnclaveHeldData);
          // If we expected binary, the EnclaveHeldData in the response should be the value sent.
          EXPECT_EQ(data->Data, *response.Value.Body.EnclaveHeldData);
        }
      }
      if (!draftPolicy.empty())
      {
        EXPECT_TRUE(response.Value.Body.PolicyClaims);
        auto policyClaims(
            Azure::Core::Json::_internal::json::parse(*response.Value.Body.PolicyClaims));
        EXPECT_TRUE(policyClaims.contains("custom-name"));
      }
    }
  };

  TEST_P(AttestationTests, SimpleAttest)
  {
    auto client(CreateClient());
    client->RetrieveResponseValidationCollateral();

    AttestationType type = std::get<1>(GetParam());
    if (type == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();
      auto attestResponse
          = client->AttestOpenEnclave(report);
      ValidateAttestResponse(attestResponse);

      attestResponse = client->AttestOpenEnclave(report);
      ValidateAttestResponse(attestResponse);
    }
    else if (type == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
      auto attestResponse = client->AttestSgxEnclave(quote);
      ValidateAttestResponse(attestResponse);
    }
  }

  TEST_P(AttestationTests, AttestWithRuntimeData)
  {
    auto client(CreateClient());
    auto runtimeData = AttestationCollateral::RuntimeData();

    AttestationType type = std::get<1>(GetParam());
    AttestOptions options;
    client->RetrieveResponseValidationCollateral();
    AttestationData data{runtimeData, AttestationDataType::Binary};
    options.RuntimeData = data;
    if (type == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();
      auto attestResponse = client->AttestOpenEnclave(report, options);
      ValidateAttestResponse(attestResponse, data);
    }
    else if (type == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
      auto attestResponse = client->AttestSgxEnclave(quote, options);
      ValidateAttestResponse(attestResponse, data);
    }
  }

  TEST_P(AttestationTests, AttestWithDraftPolicy)
  {
    // Attestation clients don't need to be authenticated, but they can be.
    auto client(CreateAuthenticatedClient());
    auto runtimeData = AttestationCollateral::RuntimeData();

    client->RetrieveResponseValidationCollateral();

    AttestationType type = std::get<1>(GetParam());

    AttestOptions options;
    options.DraftPolicyForAttestation = R"(version= 1.0;
authorizationrules
{
    [ type=="x-ms-sgx-is-debuggable", value==true] &&
    [ type=="x-ms-sgx-product-id", value!=0 ] &&
    [ type=="x-ms-sgx-svn", value>= 0 ] &&
    [ type=="x-ms-sgx-mrsigner", value == "4aea5f9a0ed04b11f889aadfe6a1d376213a29a95a85ce7337ae6f7fece6610c"]
        => permit();
};
issuancerules {
    c:[type=="x-ms-sgx-mrsigner"] => issue(type="custom-name", value=c.value);
};)";
    if (type == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();

      auto attestResponse = client->AttestOpenEnclave(report, options);
      // Because a draft policy was set, the resulting token is unsigned.
      ValidateAttestResponse(
          attestResponse, Azure::Nullable<AttestationData>(), *options.DraftPolicyForAttestation);

      options.DraftPolicyForAttestation = R"(version= 1.0;
authorizationrules
{
    [ type=="x-ms-sgx-is-debuggable", value==false ] &&
    [ type=="x-ms-sgx-product-id", value==0 ] &&
    [ type=="x-ms-sgx-svn", value>= 0 ]
        => permit();
};
issuancerules {
    c:[type=="x-ms-sgx-mrsigner"] => issue(type="custom-name", value=c.value);
};)";
      EXPECT_THROW(
          client->AttestOpenEnclave(report, options),
          Azure::Core::RequestFailedException);
    }
    else if (type == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
      auto attestResponse = client->AttestSgxEnclave(quote, options);
      ValidateAttestResponse(
          attestResponse, Azure::Nullable<AttestationData>(), *options.DraftPolicyForAttestation);

      options.DraftPolicyForAttestation = R"(version= 1.0;
authorizationrules
{
    [ type=="x-ms-sgx-is-debuggable", value==false ] &&
    [ type=="x-ms-sgx-product-id", value==0 ] &&
    [ type=="x-ms-sgx-svn", value>= 0 ]
        => permit();
};
issuancerules {
    c:[type=="x-ms-sgx-mrsigner"] => issue(type="custom-name", value=c.value);
};)";
      EXPECT_THROW(
          client->AttestSgxEnclave(quote, options),
          Azure::Core::RequestFailedException);
    }
  }

  TEST_P(AttestationTests, AttestWithRuntimeDataJson)
  {
    auto client(CreateClient());
    auto runtimeData = AttestationCollateral::RuntimeData();
    client->RetrieveResponseValidationCollateral();

    AttestationType type = std::get<1>(GetParam());
    AttestationData data{runtimeData, AttestationDataType::Json};
    if (type == AttestationType::OpenEnclave)
    {
      auto report = AttestationCollateral::OpenEnclaveReport();
      auto attestResponse = client->AttestOpenEnclave(report, {data});
      ValidateAttestResponse(attestResponse, data);
    }
    else if (type == AttestationType::SgxEnclave)
    {
      auto quote = AttestationCollateral::SgxQuote();
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
