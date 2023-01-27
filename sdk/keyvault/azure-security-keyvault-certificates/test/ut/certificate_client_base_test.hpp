// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */

#include <gtest/gtest.h>

#include "../src/private/certificate_serializers.hpp"
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/certificates.hpp>
#include <chrono>
#include <thread>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Certificates {
        namespace Test {

  /**
   * @brief A certificate downloaded X509 data.
   *
   */
  struct DownloadCertificateResult final
  {
    /**
     * @brief Certificate data.
     *
     */
    std::string Certificate;

    /**
     * @brief Content Type.
     *
     */
    CertificateContentType ContentType;
  };

  class KeyVaultCertificateClientTest : public Azure::Core::Test::TestBase,
                                        public ::testing::WithParamInterface<int> {
  public:
    KeyVaultCertificateClientTest() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }
  private:
    std::unique_ptr<Azure::Security::KeyVault::Certificates::CertificateClient> m_client;

  protected:
    std::shared_ptr<Core::Credentials::TokenCredential> m_credential;
    std::string m_keyVaultUrl;
    std::chrono::milliseconds m_defaultWait = 20s;

    // Required to rename the test propertly once the test is started.
    // We can only know the test instance name until the test instance is run.
    Azure::Security::KeyVault::Certificates::CertificateClient const& GetClientForTest(
        std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    // Runs before every test.
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      m_keyVaultUrl = GetEnv("AZURE_KEYVAULT_URL");

      // Options and credential for the client
      CertificateClientOptions options;
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      // `InitTestClient` takes care of setting up Record&Playback.
      m_client = InitTestClient<
          Azure::Security::KeyVault::Certificates::CertificateClient,
          Azure::Security::KeyVault::Certificates::CertificateClientOptions>(
          m_keyVaultUrl, m_credential, options);

      // Update default time depending on test mode.
      UpdateWaitingTime(m_defaultWait);
    }

  public:
    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating. This also return the name with suffix if the "AZURE_LIVE_TEST_SUFFIX" exists.
    std::string GetTestName(bool sanitize = true)
    {
      return Azure::Core::Test::TestBase::GetTestNameSuffix(sanitize);
    }

    template <class T>
    static inline void CheckValidResponse(
        Azure::Response<T>& response,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok)
    {
      auto const& rawResponse = response.RawResponse;
      EXPECT_EQ(
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              rawResponse->GetStatusCode()),
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              expectedCode));
    }

    static void CheckIssuers(CertificateIssuer const& data, CertificateIssuer const& issuer)
    {
      EXPECT_EQ(data.Name, issuer.Name);
      EXPECT_EQ(data.Provider.Value(), issuer.Provider.Value());
      EXPECT_TRUE(data.Properties.Enabled.Value());
      EXPECT_TRUE(data.IdUrl);

      EXPECT_EQ(data.Credentials.AccountId.Value(), issuer.Credentials.AccountId.Value());
      EXPECT_FALSE(data.Credentials.Password);

      auto adminRemote = data.Organization.AdminDetails[0];
      auto adminLocal = issuer.Organization.AdminDetails[0];

      EXPECT_EQ(adminLocal.EmailAddress.Value(), adminRemote.EmailAddress.Value());
      EXPECT_EQ(adminLocal.FirstName.Value(), adminRemote.FirstName.Value());
      EXPECT_EQ(adminLocal.LastName.Value(), adminRemote.LastName.Value());
      EXPECT_EQ(adminLocal.PhoneNumber.Value(), adminRemote.PhoneNumber.Value());
    }

    static inline void CheckContactsCollections(
        std::vector<CertificateContact> contacts,
        std::vector<CertificateContact> results)
    {
      EXPECT_EQ(results.size(), contacts.size());

      for (auto c2 : results)
      {
        bool found = false;
        for (auto c1 : contacts)
        {
          if (c1.EmailAddress == c2.EmailAddress && c1.Name.HasValue() == c2.Name.HasValue()
              && c1.Phone.HasValue() == c2.Phone.HasValue())
          {
            found = true;
            break;
          }
        }
        EXPECT_TRUE(found);
      }

      for (auto c1 : contacts)
      {
        bool found = false;
        for (auto c2 : results)
        {
          if (c1.EmailAddress == c2.EmailAddress && c1.Name.HasValue() == c2.Name.HasValue()
              && c1.Phone.HasValue() == c2.Phone.HasValue())
          {
            found = true;
            break;
          }
        }
        EXPECT_TRUE(found);
      }
    }

    static inline KeyVaultCertificateWithPolicy CreateCertificate(
        std::string const& name,
        CertificateClient const& client,
        std::chrono::milliseconds defaultWait,
        std::string const& subject = "CN=xyz",
        CertificateContentType certificateType = CertificateContentType::Pkcs12)
    {
      CertificateCreateOptions options;
      options.Policy.Subject = subject;
      options.Policy.ValidityInMonths = 12;
      options.Policy.Enabled = true;

      options.Properties.Enabled = true;
      options.Properties.Name = name;
      options.Policy.ContentType = certificateType;
      options.Policy.IssuerName = "Self";

      LifetimeAction action;
      action.LifetimePercentage = 80;
      action.Action = CertificatePolicyAction::AutoRenew;
      options.Policy.LifetimeActions.emplace_back(action);

      auto response = client.StartCreateCertificate(name, options);
      auto pollResult = response.PollUntilDone(defaultWait);
      EXPECT_EQ(pollResult.Value.Name, name);
      EXPECT_TRUE(pollResult.Value.Status.HasValue());
      EXPECT_EQ(pollResult.Value.Status.Value(), "completed");
      EXPECT_EQ(pollResult.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
      // get the certificate
      auto result = client.GetCertificate(name);

      EXPECT_EQ(result.Value.Name(), options.Properties.Name);
      EXPECT_EQ(result.Value.Properties.Name, options.Properties.Name);
      EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
      EXPECT_EQ(result.Value.Policy.IssuerName.Value(), options.Policy.IssuerName.Value());
      EXPECT_EQ(result.Value.Policy.ContentType.Value(), options.Policy.ContentType.Value());
      EXPECT_EQ(result.Value.Policy.Subject, options.Policy.Subject);
      EXPECT_EQ(
          result.Value.Policy.ValidityInMonths.Value(), options.Policy.ValidityInMonths.Value());
      EXPECT_EQ(result.Value.Policy.Enabled.Value(), options.Policy.Enabled.Value());
      EXPECT_EQ(result.Value.Policy.LifetimeActions.size(), size_t(1));
      EXPECT_EQ(result.Value.Policy.LifetimeActions[0].Action, action.Action);
      EXPECT_EQ(
          result.Value.Policy.LifetimeActions[0].LifetimePercentage.Value(),
          action.LifetimePercentage.Value());
      EXPECT_EQ(result.Value.Policy.KeyUsage.size(), size_t(2));
      auto keyUsage = result.Value.Policy.KeyUsage;
      EXPECT_TRUE(
          (keyUsage[0] == CertificateKeyUsage::DigitalSignature
           && keyUsage[1] == CertificateKeyUsage::KeyEncipherment)
          || (keyUsage[1] == CertificateKeyUsage::DigitalSignature
              && keyUsage[0] == CertificateKeyUsage::KeyEncipherment));
      return result.Value;
    }

    Azure::Response<DownloadCertificateResult> DownloadCertificate(
        std::string const& name,
        CertificateClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      {
        KeyVaultCertificateWithPolicy certificate;
        auto response = client.GetCertificate(name, context);
        certificate = response.Value;

        Azure::Core::Url url(certificate.SecretIdUrl);
        auto secretRequest
            = client.CreateRequest(Azure::Core::Http::HttpMethod::Get, {url.GetPath()});

        auto secretResponse = client.SendRequest(secretRequest, context);
        auto secret = _detail::KeyVaultSecretSerializer::Deserialize(*secretResponse);

        DownloadCertificateResult result{secret.Value, secret.ContentType.Value()};
        return Azure::Response<DownloadCertificateResult>(
            std::move(result), std::move(secretResponse));
      }
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Certificates::Test
