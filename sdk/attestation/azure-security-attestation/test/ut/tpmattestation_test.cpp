// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "attestation_collateral.hpp"
#include "azure/attestation/attestation_administration_client.hpp"
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

  // cspell: words aikcert
  class TpmAttestationTests : public Azure::Core::Test::TestBase {
  public:
    TpmAttestationTests() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); };

  protected:
    std::shared_ptr<const Azure::Core::Credentials::TokenCredential> m_credential;
    std::unique_ptr<AttestationAdministrationClient> m_adminClient;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      {
        if (m_testContext.GetTestMode() != Azure::Core::Test::TestMode::PLAYBACK)
        {
          // TPM attestation requires a policy document be set. For simplicity, we only run the
          // test against an AAD attestation service instance.
          m_adminClient = std::make_unique<AttestationAdministrationClient>(
              CreateAdminClient(InstanceType::AAD));

          // Set a minimal policy, which will make the TPM attestation code happy.
          m_adminClient->SetAttestationPolicy(
              AttestationType::Tpm,
              "version=1.0; authorizationrules{=> permit();}; issuancerules{};");
        }
      }
    }

    virtual void TearDown() override
    {
      if (m_testContext.GetTestMode() != Azure::Core::Test::TestMode::PLAYBACK)
      {
        // Reset the attestation policy for this instance back to the default.
        if (m_adminClient)
        {
          m_adminClient->ResetAttestationPolicy(AttestationType::Tpm);
        }
      }

      // Make sure you call the base classes TearDown method to ensure recordings are made.
      TestBase::TearDown();
    }

    std::string GetInstanceUri(InstanceType instanceType)
    {
      if (instanceType == InstanceType::Shared)
      {
        std::string shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        return "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (instanceType == InstanceType::AAD)
      {
        return GetEnv("ATTESTATION_AAD_URL");
      }
      else if (instanceType == InstanceType::Isolated)
      {
        return GetEnv("ATTESTATION_ISOLATED_URL");
      }
      throw std::runtime_error("Unkown instance type.");
    }

    AttestationTokenValidationOptions GetTokenValidationOptions()
    {
      AttestationTokenValidationOptions returnValue;
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        returnValue.ValidateNotBeforeTime = false;
        returnValue.ValidateExpirationTime = false;
      }
      else
      {
        returnValue.TimeValidationSlack = 10s;
      }
      return returnValue;
    }

    AttestationClient CreateClient(InstanceType instanceType)
    {
      // `InitClientOptions` takes care of setting up Record&Playback.
      AttestationClientOptions options = InitClientOptions<AttestationClientOptions>();
      options.TokenValidationOptions = GetTokenValidationOptions();
      auto credential = GetTestCredential();
      return AttestationClient::Create(GetInstanceUri(instanceType), credential, options);
    }

    AttestationAdministrationClient CreateAdminClient(InstanceType instanceType)
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      AttestationAdministrationClientOptions options
          = InitClientOptions<AttestationAdministrationClientOptions>();
      options.TokenValidationOptions = GetTokenValidationOptions();
      auto credential = GetTestCredential();
      return AttestationAdministrationClient::Create(
          GetInstanceUri(instanceType), credential, options);
    }
  };

  TEST_F(TpmAttestationTests, AttestTpm_LIVEONLY_)
  {
    auto client(CreateClient(InstanceType::AAD));

    std::string tpmQuote = R"({"payload": { "type": "aikcert" } })";
    auto response(client.AttestTpm(std::vector<uint8_t>(tpmQuote.begin(), tpmQuote.end())));

    Azure::Core::Json::_internal::json parsedResponse(
        Azure::Core::Json::_internal::json::parse(response.Value.TpmResult));
    EXPECT_TRUE(parsedResponse.contains("payload"));
    EXPECT_TRUE(parsedResponse["payload"].is_object());
    EXPECT_TRUE(parsedResponse["payload"].contains("challenge"));
    EXPECT_TRUE(parsedResponse["payload"].contains("service_context"));
  }

}}}} // namespace Azure::Security::Attestation::Test
