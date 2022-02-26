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

  enum class InstanceType
  {
    Shared,
    AAD,
    Isolated
  };

  struct PolicyTestParam
  {
    TestCaseType TestType;
    InstanceType InstanceType;
    AttestationType TeeType;
  };

  class PolicyTests : public Azure::Core::Test::TestBase,
                      public testing::WithParamInterface<PolicyTestParam> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

      if (GetParam().InstanceType == InstanceType::Shared)
      {
        std::string const shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        m_endpoint = "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (GetParam().InstanceType == InstanceType::AAD)
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (GetParam().InstanceType == InstanceType::Isolated)
      {
        m_endpoint = GetEnv("ATTESTATION_ISOLATED_URL");
      }
    }

    std::unique_ptr<AttestationAdministrationClient> CreateClient()
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      Azure::Security::Attestation::AttestationAdministrationClientOptions options;
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        options.TokenValidationOptions.ValidateNotBeforeTime = false;
        options.TokenValidationOptions.ValidateExpirationTime = false;
      }
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return InitTestClient<
          Azure::Security::Attestation::AttestationAdministrationClient,
          Azure::Security::Attestation::AttestationAdministrationClientOptions>(
          m_endpoint, credential, options);
    }

    bool ValidateSetPolicyResponse(
        std::unique_ptr<AttestationAdministrationClient> const& client,
        Response<AttestationToken<PolicyResult>> const& result,
        Azure::Nullable<std::string> policyToValidate,
        Azure::Nullable<AttestationSigningKey> const& signingKey
        = Azure::Nullable<AttestationSigningKey>())
    {
      EXPECT_EQ(result.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);

      // SetPolicy responses should have updated or reset the policy value.
      EXPECT_TRUE(result.Value.Body.PolicyResolution);
      if (policyToValidate)
      {
        EXPECT_EQ(PolicyModification::Updated, result.Value.Body.PolicyResolution.Value());

        // The attestation service only returns the PolicySigner and PolicySigningHash on SetPolicy
        // calls, not ResetPolicy calls.

        // Now check the policy signer if appropriate.
        if (signingKey)
        {
          EXPECT_TRUE(result.Value.Body.PolicySigner);
          EXPECT_FALSE(result.Value.Body.PolicySigner.Value().CertificateChain.Value().empty());

          // When the test case type is secured, playback mode, the signing certificate was the
          // certificate retrieved at the time the recordings were made, and it will *not* match the
          // dummy value provided for the recorded tests.
          if (!m_testContext.IsPlaybackMode())
          {
            auto signerCertificate = Cryptography::ImportX509Certificate(
                result.Value.Body.PolicySigner.Value().CertificateChain.Value()[0]);
            auto expectedCertificate
                = Cryptography::ImportX509Certificate(signingKey.Value().PemEncodedX509Certificate);
            EXPECT_EQ(expectedCertificate->GetThumbprint(), signerCertificate->GetThumbprint());
          }
        }
        else
        {
          EXPECT_FALSE(result.Value.Body.PolicySigner);
        }

        EXPECT_TRUE(result.Value.Body.PolicyTokenHash);

        // The returned PolicyTokenHash value is the hash of the entire policy JWS that was sent to
        // the service. In playback mode, the JWS which is calculated for the tests is different
        // from the JWS which was recorded (because the signing certificate is different).
        //
        // So skip verifying the PolicyTokenHash in playback mode.
        if (!m_testContext.IsPlaybackMode())
        {
          AttestationToken<std::nullptr_t> sentToken
              = client->CreateSetAttestationPolicyToken(policyToValidate, signingKey);

          Azure::Core::Cryptography::_internal::Sha256Hash hasher;
          std::vector<uint8_t> rawTokenHash = hasher.Final(
              reinterpret_cast<const uint8_t*>(sentToken.RawToken.data()),
              sentToken.RawToken.size());
          EXPECT_EQ(result.Value.Body.PolicyTokenHash.Value(), rawTokenHash);
        }
      }
      else
      {
        EXPECT_EQ(PolicyModification::Removed, result.Value.Body.PolicyResolution.Value());
      }

      return true;
    }

    void SetPolicyTest(
        Azure::Nullable<AttestationSigningKey> const& signingKey
        = Azure::Nullable<AttestationSigningKey>())
    {
      auto adminClient(CreateClient());

      std::string policyToSet(AttestationCollateral::GetMinimalPolicy());
      SetPolicyOptions setOptions;
      setOptions.SigningKey = signingKey;
      auto setResponse
          = adminClient->SetAttestationPolicy(GetParam().TeeType, policyToSet, setOptions);

      EXPECT_TRUE(ValidateSetPolicyResponse(adminClient, setResponse, policyToSet, signingKey));

      // Make sure that the policy we set can be retrieved (we've checked the hash in
      // ValidateSetPolicyResponse, but this doesn't hurt)
      auto getResponse = adminClient->GetAttestationPolicy(GetParam().TeeType);
      EXPECT_EQ(policyToSet, getResponse.Value.Body);
    }

    void ResetPolicyTest(
        Azure::Nullable<AttestationSigningKey> const& signingKey
        = Azure::Nullable<AttestationSigningKey>())
    {
      auto adminClient(CreateClient());

      SetPolicyOptions setOptions;
      setOptions.SigningKey = signingKey;
      auto setResponse = adminClient->ResetAttestationPolicy(GetParam().TeeType, setOptions);

      EXPECT_TRUE(ValidateSetPolicyResponse(
          adminClient, setResponse, Azure::Nullable<std::string>(), signingKey));

      // The policy had better not be the minimal policy after we've reset it.
      auto getResponse = adminClient->GetAttestationPolicy(GetParam().TeeType);
      EXPECT_NE(AttestationCollateral::GetMinimalPolicy(), getResponse.Value.Body);
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
        std::vector<InstanceType> typeNameList;

        switch (testCaseType)
        {
          case TestCaseType::GetPolicy:
            typeNameList.emplace_back(InstanceType::AAD);
            typeNameList.emplace_back(InstanceType::Isolated);
            typeNameList.emplace_back(InstanceType::Shared);
            break;
          case TestCaseType::ModifyPolicyIsolated:
            typeNameList.emplace_back(InstanceType::AAD); // The isolated key will work in AAD mode.
            typeNameList.emplace_back(InstanceType::Isolated);
            break;
          case TestCaseType::ModifyPolicySecured:
          case TestCaseType::ModifyPolicyUnsecured:
            typeNameList.emplace_back(InstanceType::AAD);
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
  };

  TEST_P(PolicyTests, PolicyTests)
  {
    switch (GetParam().TestType)
    {
      // Tests for the GetAttestationPolicy APIs.
      case TestCaseType::GetPolicy: {
        auto adminClient(CreateClient());

        EXPECT_FALSE(adminClient->ClientVersion().empty());

        AttestationType attestationType(GetParam().TeeType);
        {
          auto policy = adminClient->GetAttestationPolicy(attestationType);

          // The policy should have a value, and the token should have been issued by the service.
          // Note that if the policy *doesn't* have a body, then the attestation type must be TPM
          // since TPM attestation is the only attestation type which allows empty policy documents.
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
            EXPECT_EQ(m_endpoint, policy.Value.Issuer.Value());
          }
        }

        {
          GetPolicyOptions gpOptions;
          EXPECT_FALSE(gpOptions.TokenValidationOptions);
        }
        break;
      }

      // Modify attestation policies using an unsecured attestation JWS. This exercises the
      // SetPolicy and ResetPolicy APIs.
      case TestCaseType::ModifyPolicyUnsecured: {
        SetPolicyTest();
        ResetPolicyTest();
      }

      // Modify attestation policies using an ephemeral secured attestation JWS. This exercises the
      // SetPolicy and ResetPolicy APIs.
      case TestCaseType::ModifyPolicySecured: {
        auto rsaKey(Cryptography::CreateRsaKey(2048));
        auto signingCert(Cryptography::CreateX509CertificateForPrivateKey(
            rsaKey, "CN=TestSetPolicyCertificate"));

        auto signingKey(
            AttestationSigningKey{rsaKey->ExportPrivateKey(), signingCert->ExportAsPEM()});

        SetPolicyTest(signingKey);
        ResetPolicyTest(signingKey);
        break;
      }

      // Modify attestation policies using a predefined signing key and certificate.
      // The key and certificate were created at test resource creation time.
      // Exercises the SetPolicy and ResetPolicy APIs.
      case TestCaseType::ModifyPolicyIsolated: {
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
        break;
      }
      default:
        ASSERT_FALSE(true) << "Unknown test parameter";
    }
  }

  namespace {
    std::string GetTestName(testing::TestParamInfo<PolicyTests::ParamType> const& testInfo)
    {
      std::string testName;
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
        case InstanceType::AAD:
          testName += "AAD";
          break;
        case InstanceType::Isolated:
          testName += "Isolated";
          break;
        case InstanceType::Shared:
          testName += "Shared";
          break;
        default:
          throw std::runtime_error("Unknown instance type");
      }

      testName += "_";
      testName += testInfo.param.TeeType.ToString();
      return testName;
    }
  } // namespace

  INSTANTIATE_TEST_SUITE_P(
      Policy,
      PolicyTests,
      testing::ValuesIn(PolicyTests::GetTestInputs()),
      GetTestName);

}}}} // namespace Azure::Security::Attestation::Test
