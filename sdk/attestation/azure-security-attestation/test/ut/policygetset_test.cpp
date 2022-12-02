// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../src/private/crypto/inc/crypto.hpp"
#include "attestation_collateral.hpp"
#include "azure/attestation/attestation_administration_client.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include <azure/attestation/attestation_client_models.hpp>
#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/core/test/test_base.hpp>
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <tuple>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Security::Attestation::_detail;

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  enum class TestCaseType
  {
    GetPolicy,
    ModifyPolicyUnsecured,
    ModifyPolicySecured,
    ModifyPolicyIsolated,
  };

  enum class ServiceInstanceType
  {
    Shared,
    AAD,
    Isolated
  };

  struct PolicyTestParam
  {
    TestCaseType TestType;
    ServiceInstanceType InstanceType;
    AttestationType TeeType;
  };

  class PolicyTests : public Azure::Core::Test::TestBase,
                      public testing::WithParamInterface<PolicyTestParam> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;
    virtual std::string GetAssetsPath() override { return "assets.json"; }
    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      ServiceInstanceType type = GetParam().InstanceType;
      if (type == ServiceInstanceType::Shared)
      {
        std::string const shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        m_endpoint = "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (type == ServiceInstanceType::AAD)
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (type == ServiceInstanceType::Isolated)
      {
        m_endpoint = GetEnv("ATTESTATION_ISOLATED_URL");
      }
    }

    Azure::Security::Attestation::AttestationTokenValidationOptions GetTokenValidationOptions()
    {
      AttestationTokenValidationOptions returnValue{};

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

    AttestationAdministrationClient CreateClient()
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      AttestationAdministrationClientOptions options
          = InitClientOptions<AttestationAdministrationClientOptions>();
      options.TokenValidationOptions = GetTokenValidationOptions();

      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = CreateClientSecretCredential(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return AttestationAdministrationClient::Create(m_endpoint, credential, options);
    }

    bool ValidateSetPolicyResponse(
        AttestationAdministrationClient const& client,
        Response<AttestationToken<PolicyResult>> const& result,
        Azure::Nullable<std::string> policyToValidate,
        Azure::Nullable<AttestationSigningKey> const& signingKey = {})
    {
      EXPECT_EQ(result.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);

      // SetPolicy responses should have updated or reset the policy value.
      if (policyToValidate)
      {
        EXPECT_EQ(PolicyModification::Updated, result.Value.Body.PolicyResolution);

        // The attestation service only returns the PolicySigner and PolicySigningHash on
        // SetPolicy calls, not ResetPolicy calls.

        // Now check the policy signer if appropriate.
        if (signingKey)
        {
          EXPECT_TRUE(result.Value.Body.PolicySigner);
          EXPECT_FALSE(result.Value.Body.PolicySigner->CertificateChain->empty());

          // When the test case type is secured, playback mode, the signing certificate was the
          // certificate retrieved at the time the recordings were made, and it will *not* match
          // the dummy value provided for the recorded tests.
          if (!m_testContext.IsPlaybackMode())
          {
            auto signerCertificate = Cryptography::ImportX509Certificate(
                (*result.Value.Body.PolicySigner->CertificateChain)[0]);
            auto expectedCertificate
                = Cryptography::ImportX509Certificate(signingKey->PemEncodedX509Certificate);
            EXPECT_EQ(expectedCertificate->GetThumbprint(), signerCertificate->GetThumbprint());
          }
        }
        else
        {
          EXPECT_FALSE(result.Value.Body.PolicySigner);
        }

        // The returned PolicyTokenHash value is the hash of the entire policy JWS that was sent
        // to the service. In playback mode, the JWS which is calculated for the tests is
        // different from the JWS which was recorded (because the signing certificate is
        // different).
        //
        // So skip verifying the PolicyTokenHash in playback mode.
        if (!m_testContext.IsPlaybackMode())
        {
          AttestationToken<void> sentToken
              = client.CreateAttestationPolicyToken(policyToValidate, signingKey);

          Azure::Core::Cryptography::_internal::Sha256Hash hasher;
          std::vector<uint8_t> rawTokenHash = hasher.Final(
              reinterpret_cast<const uint8_t*>(sentToken.RawToken.data()),
              sentToken.RawToken.size());
          EXPECT_EQ(result.Value.Body.PolicyTokenHash, rawTokenHash);
        }
      }
      else
      {
        EXPECT_EQ(PolicyModification::Removed, result.Value.Body.PolicyResolution);
      }

      return true;
    }

    void SetPolicyTest(Azure::Nullable<AttestationSigningKey> const& signingKey = {})
    {
      auto adminClient(CreateClient());

      std::string policyToSet(AttestationCollateral::GetMinimalPolicy());
      SetPolicyOptions setOptions;
      setOptions.SigningKey = signingKey;
      auto setResponse
          = adminClient.SetAttestationPolicy(GetParam().TeeType, policyToSet, setOptions);

      EXPECT_TRUE(ValidateSetPolicyResponse(adminClient, setResponse, policyToSet, signingKey));

      // Make sure that the policy we set can be retrieved (we've checked the hash in
      // ValidateSetPolicyResponse, but this doesn't hurt)
      auto getResponse = adminClient.GetAttestationPolicy(
          GetParam().TeeType, GetPolicyOptions{GetTokenValidationOptions()});
      EXPECT_EQ(policyToSet, getResponse.Value.Body);
    }

    void ResetPolicyTest(Azure::Nullable<AttestationSigningKey> const& signingKey = {})
    {
      auto adminClient(CreateClient());

      SetPolicyOptions setOptions;
      setOptions.SigningKey = signingKey;
      setOptions.TokenValidationOptionsOverride = GetTokenValidationOptions();

      auto setResponse = adminClient.ResetAttestationPolicy(GetParam().TeeType, setOptions);

      EXPECT_TRUE(ValidateSetPolicyResponse(
          adminClient, setResponse, Azure::Nullable<std::string>(), signingKey));

      // The policy had better not be the minimal policy after we've reset it.
      auto getResponse = adminClient.GetAttestationPolicy(GetParam().TeeType);
      EXPECT_NE(AttestationCollateral::GetMinimalPolicy(), getResponse.Value.Body);
    }

    /** @brief Tests for the `GetAttestationPolicy` API.
     *
     * These tests are relatively straightforward. Call the API on the provided TeeType and verify
     * that the returned policy makes sense as an attestation policy (starts with the text "version"
     * - beyond that, we can't verify the response).
     *
     * Note that VSM/VBS/TPM policies can be empty, so if we encounter an empty policy, verify that
     * the policy came from TPM attestation.
     *
     * One additional check is performed in live mode: We verify that the issuer of the returned
     * attestation token matches the endpoint. This check cannot be run against recorded collateral
     * because the `m_endpoint` value is mocked on recorded clients.
     */
    void GetPolicyTest()
    {
      auto adminClient(CreateClient());

      AttestationType attestationType(GetParam().TeeType);
      {
        auto policy = adminClient.GetAttestationPolicy(attestationType);

        // The policy should have a value, and the token should have been issued by the service.
        // Note that if the policy *doesn't* have a body, then the attestation type must be TPM
        // since TPM attestation is the only attestation type which allows empty policy
        // documents.
        if (policy.Value.Body.empty())
        {
          EXPECT_EQ(AttestationType::Tpm, attestationType);
        }
        else
        {
          EXPECT_EQ(0UL, policy.Value.Body.find("version"));
        }

        // In playback mode, the endpoint is a mocked value so the Issuer in the result will not
        // match.
        if (!m_testContext.IsPlaybackMode())
        {
          EXPECT_EQ(m_endpoint, *policy.Value.Issuer);
        }
      }

      {
        GetPolicyOptions gpOptions;
        EXPECT_FALSE(gpOptions.TokenValidationOptionsOverride);
      }
    }

    /** @brief Tests for policy modification using an unsecured JWS.
     *
     * Forwards to the `SetPolicyTest` and `ResetPolicyTest` with a non-present AttestationSigingKey
     * parameter.
     */
    void ModifyPolicyUnsecuredTest()
    {
      SetPolicyTest();
      ResetPolicyTest();
    }

    /** @brief Tests for policy modification using a secured JWS with an emphemerally generated key.
     *
     * Forwards to the `SetPolicyTest` and `ResetPolicyTest` with a newly created
     * AttestationSigingKey parameter.
     */
    void ModifyPolicySecuredTest()
    {
      auto rsaKey(Cryptography::CreateRsaKey(2048));
      auto signingCert(
          Cryptography::CreateX509CertificateForPrivateKey(rsaKey, "CN=TestSetPolicyCertificate"));

      auto signingKey(
          AttestationSigningKey{rsaKey->ExportPrivateKey(), signingCert->ExportAsPEM()});

      SetPolicyTest(signingKey);
      ResetPolicyTest(signingKey);
    }

    /** @brief Tests for policy modification using a secured JWS with a predefined key.
     *
     * Forwards to the `SetPolicyTest` and `ResetPolicyTest` with an AttestationSigingKey parameter
     * defined in new-testresources.ps1.
     *
     * Note that this is a live-only test, because
     */
    void ModifyPolicyIsolatedTest()
    {
      // In PlaybackMode, the values of ISOLATED_SIGNING_CERTIFICATE and ISOLATED_SIGNING_KEY are
      // replaced with dummy values which cannot be converted into actual certificates. So skip
      // the isolated mode tests when we are in playback or record mode (there's no point in
      // recording something that cannot work).
      if (m_testContext.IsLiveMode())
      {
        std::string isolatedCertificate(GetEnv("ISOLATED_SIGNING_CERTIFICATE"));
        std::string isolatedKey(GetEnv("ISOLATED_SIGNING_KEY"));

        AttestationSigningKey signingKey;
        signingKey.PemEncodedPrivateKey = Cryptography::PemFromBase64(isolatedKey, "PRIVATE KEY");
        signingKey.PemEncodedX509Certificate
            = Cryptography::PemFromBase64(isolatedCertificate, "CERTIFICATE");

        SetPolicyTest(signingKey);
        ResetPolicyTest(signingKey);
      }
    }

  public:
    static std::vector<PolicyTestParam> GetTestInputs()
    {
      std::vector<PolicyTestParam> returnCases;
      std::vector<TestCaseType> testTypes{
          TestCaseType::GetPolicy,
          TestCaseType::ModifyPolicyUnsecured,
          TestCaseType::ModifyPolicySecured,
          TestCaseType::ModifyPolicyIsolated};
      for (auto const& testCaseType : testTypes)
      {
        std::vector<ServiceInstanceType> typeNameList;

        switch (testCaseType)
        {
          case TestCaseType::GetPolicy:
            typeNameList.emplace_back(ServiceInstanceType::AAD);
            typeNameList.emplace_back(ServiceInstanceType::Isolated);
            typeNameList.emplace_back(ServiceInstanceType::Shared);
            break;
          case TestCaseType::ModifyPolicyIsolated:
            typeNameList.emplace_back(
                ServiceInstanceType::AAD); // The isolated key will work in AAD mode.
            typeNameList.emplace_back(ServiceInstanceType::Isolated);
            break;
          case TestCaseType::ModifyPolicySecured:
          case TestCaseType::ModifyPolicyUnsecured:
            typeNameList.emplace_back(ServiceInstanceType::AAD);
            break;
          default:
            throw std::runtime_error("Unknown TestCaseType in GetTestInputs");
        }

        for (auto const& value : typeNameList)
        {
          for (auto const& type :
               {AttestationType::SgxEnclave, AttestationType::OpenEnclave, AttestationType::Tpm})
          {
            returnCases.emplace_back(PolicyTestParam{testCaseType, value, type});
          }
        }
      }
      return returnCases;
    }
  }; // namespace Test

  TEST_P(PolicyTests, PolicyTests)
  {
    switch (GetParam().TestType)
    {
      // Tests for the GetAttestationPolicy APIs.
      case TestCaseType::GetPolicy:
        GetPolicyTest();
        break;

      // Modify attestation policies using an unsecured attestation JWS. This exercises the
      // SetPolicy and ResetPolicy APIs.
      case TestCaseType::ModifyPolicyUnsecured:
        ModifyPolicyUnsecuredTest();
        break;

      // Modify attestation policies using an ephemeral secured attestation JWS. This exercises the
      // SetPolicy and ResetPolicy APIs.
      case TestCaseType::ModifyPolicySecured:
        ModifyPolicySecuredTest();
        break;

      // Modify attestation policies using a predefined signing key and certificate.
      // The key and certificate were created at test resource creation time.
      // Exercises the SetPolicy and ResetPolicy APIs.
      case TestCaseType::ModifyPolicyIsolated:
        ModifyPolicyIsolatedTest(); // LIVE-ONLY test!
        break;
      default:
        ASSERT_FALSE(true) << "Unknown test parameter";
    }
  } // namespace Test

  TEST_P(PolicyTests, CreateAdministrationClients)
  {
    // `InitTestClient` takes care of setting up Record&Playback.
    auto options
        = InitClientOptions<Azure::Security::Attestation::AttestationAdministrationClientOptions>();
    {
      AttestationAdministrationClient client
          = AttestationAdministrationClient::Create(this->m_endpoint, m_credential, options);
      EXPECT_EQ(m_endpoint, client.Endpoint());
    }
    {
      AttestationAdministrationClient const client
          = AttestationAdministrationClient::Create(this->m_endpoint, m_credential, options);
      EXPECT_EQ(m_endpoint, client.Endpoint());
    }
  }

  namespace {
    std::string GetTestName(testing::TestParamInfo<PolicyTests::ParamType> const& testInfo)
    {
      std::string testName;
      int suffixVotes = 0;
      switch (testInfo.param.TestType)
      {
        case TestCaseType::GetPolicy:
          testName += "GetPolicy";
          break;
        case TestCaseType::ModifyPolicyIsolated:
          testName += "ModifyIsolatedKey";
          break;
        case TestCaseType::ModifyPolicySecured:
          testName += "ModifyGeneratedKey";
          suffixVotes++;
          break;
        case TestCaseType::ModifyPolicyUnsecured:
          testName += "ModifyUnsecured";
          break;
        default:
          throw std::runtime_error("Unknown test case type");
      }
      testName += "_";
      switch (testInfo.param.InstanceType)
      {
        case ServiceInstanceType::AAD:
          testName += "AAD";
          suffixVotes++;
          break;
        case ServiceInstanceType::Isolated:
          testName += "Isolated";
          break;
        case ServiceInstanceType::Shared:
          testName += "Shared";
          break;
        default:
          throw std::runtime_error("Unknown instance type");
      }

      testName += "_";
      testName += testInfo.param.TeeType.ToString();
      if (suffixVotes == 2)
      {
        testName += "_LIVEONLY_";
      };
      //+"_LIVEONLY_";
      return testName;
    }
  } // namespace

  INSTANTIATE_TEST_SUITE_P(
      Policy,
      PolicyTests,
      testing::ValuesIn(PolicyTests::GetTestInputs()),
      GetTestName);

}}}} // namespace Azure::Security::Attestation::Test
