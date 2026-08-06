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

  enum class PlutonInstanceType
  {
    Shared,
    AAD,
    Isolated
  };

  // cspell: words plutonattestation
  static const std::string PlutonApiVersion = "2026-03-11preview";

  class PlutonAttestationTests : public Azure::Core::Test::TestBase {
  public:
    PlutonAttestationTests() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); };

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
          m_adminClient = std::make_unique<AttestationAdministrationClient>(
              CreateAdminClient(PlutonInstanceType::AAD));

          // Set a minimal policy for Pluton attestation.
          m_adminClient->SetAttestationPolicy(
              AttestationType::Pluton,
              "version=1.0; authorizationrules{=> permit();}; issuancerules{};");
        }
      }
    }

    virtual void TearDown() override
    {
      if (m_testContext.GetTestMode() != Azure::Core::Test::TestMode::PLAYBACK)
      {
        if (m_adminClient)
        {
          m_adminClient->ResetAttestationPolicy(AttestationType::Pluton);
        }
      }

      // Make sure you call the base classes TearDown method to ensure recordings are made.
      TestBase::TearDown();
    }

    std::string GetInstanceUri(PlutonInstanceType instanceType)
    {
      if (instanceType == PlutonInstanceType::Shared)
      {
        std::string shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        return "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (instanceType == PlutonInstanceType::AAD)
      {
        return GetEnv("ATTESTATION_AAD_URL");
      }
      else if (instanceType == PlutonInstanceType::Isolated)
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

    AttestationClient CreateClient(PlutonInstanceType instanceType)
    {
      // `InitClientOptions` takes care of setting up Record&Playback.
      AttestationClientOptions options = InitClientOptions<AttestationClientOptions>();
      options.ApiVersion = PlutonApiVersion;
      options.TokenValidationOptions = GetTokenValidationOptions();
      auto credential = GetTestCredential();
      return AttestationClient::Create(GetInstanceUri(instanceType), credential, options);
    }

    AttestationAdministrationClient CreateAdminClient(PlutonInstanceType instanceType)
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      AttestationAdministrationClientOptions options
          = InitClientOptions<AttestationAdministrationClientOptions>();
      options.ApiVersion = PlutonApiVersion;
      options.TokenValidationOptions = GetTokenValidationOptions();
      auto credential = GetTestCredential();
      return AttestationAdministrationClient::Create(
          GetInstanceUri(instanceType), credential, options);
    }
  };

  TEST_F(PlutonAttestationTests, AttestPluton_LIVEONLY_)
  {
    auto client(CreateClient(PlutonInstanceType::AAD));

    std::string plutonPayload = R"({"payload": { "type": "pluton" } })";
    auto response(
        client.AttestPluton(std::vector<uint8_t>(plutonPayload.begin(), plutonPayload.end())));

    // Verify the response contains bytes.
    EXPECT_FALSE(response.Value.PlutonResult.empty());

    // Parse the response to verify it's valid JSON with expected structure.
    Azure::Core::Json::_internal::json parsedResponse(
        Azure::Core::Json::_internal::json::parse(response.Value.PlutonResult));
    EXPECT_TRUE(parsedResponse.contains("payload"));
    EXPECT_TRUE(parsedResponse["payload"].is_object());
    EXPECT_TRUE(parsedResponse["payload"].contains("challenge"));
    EXPECT_TRUE(parsedResponse["payload"].contains("service_context"));
  }

}}}} // namespace Azure::Security::Attestation::Test
