// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include "certificate_client_base_test.hpp"
#include <cstddef>
#include <gtest/gtest.h>

#include <string>

using namespace std::chrono_literals;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Security::KeyVault::Certificates::Test;

TEST_F(KeyVaultCertificateClientTest, GetCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  {
    auto response = client.GetCertificate(certificateName);
    CheckValidResponse(response);
    auto cert = response.Value;
    EXPECT_EQ(cert.Name(), cert.Properties.Name);
    EXPECT_EQ(cert.Properties.Name, certificateName);
    EXPECT_EQ(cert.Properties.VaultUrl, m_keyVaultUrl);
    // There should be a version
    EXPECT_NE(cert.Properties.Version, "");

    // x5t
    EXPECT_NE(cert.Properties.X509Thumbprint.size(), 0);
    EXPECT_EQ(cert.Properties.Tags.size(), 0);

    // attributes
    EXPECT_TRUE(cert.Properties.Enabled.HasValue());
    EXPECT_TRUE(cert.Properties.NotBefore.HasValue());
    EXPECT_TRUE(cert.Properties.ExpiresOn.HasValue());
    EXPECT_TRUE(cert.Properties.CreatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.UpdatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.RecoverableDays.HasValue());
    EXPECT_TRUE(cert.Properties.RecoveryLevel.HasValue());

    // kid, sid, cer
    EXPECT_NE(cert.KeyId, "");
    EXPECT_NE(cert.SecretId, "");
    EXPECT_NE(cert.Cer.size(), 0);

    // policy
    {
      auto const& policy = cert.Policy;

      // Key props
      EXPECT_TRUE(policy.Exportable.HasValue());
      EXPECT_TRUE(policy.KeyType.HasValue());
      EXPECT_TRUE(policy.ReuseKey.HasValue());
      // Recording uses RSA with no curve-name. Use RSA key when running LIVE
      EXPECT_FALSE(policy.KeyCurveName.HasValue());
      EXPECT_TRUE(policy.KeySize.HasValue());

      // Secret props
      EXPECT_TRUE(policy.ContentType.HasValue());

      // x509_props
      EXPECT_TRUE(policy.Subject.size() > 0);

      // issuer
      EXPECT_TRUE(policy.Issuer.Name.HasValue());

      // attributes
      EXPECT_TRUE(policy.CreatedOn.HasValue());

      // lifetime_actions
      EXPECT_TRUE(policy.LifetimeActions.size() > 0);
      EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");
    }
  }
}

TEST_F(KeyVaultCertificateClientTest, GetCertificateVersion)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu");
  std::string const certificateVersion("8d532937fea74df58d9b18fca036ad51");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  {
    GetCertificateOptions options;
    options.Version = certificateVersion;
    auto response = client.GetCertificateVersion(certificateName, options);
    CheckValidResponse(response);
    auto cert = response.Value;
    EXPECT_EQ(cert.Name(), cert.Properties.Name);
    EXPECT_EQ(cert.Properties.Name, certificateName);
    EXPECT_EQ(cert.Properties.VaultUrl, m_keyVaultUrl);
    // There should be a version
    EXPECT_NE(cert.Properties.Version, "");

    // x5t
    EXPECT_NE(cert.Properties.X509Thumbprint.size(), 0);
    EXPECT_EQ(cert.Properties.Tags.size(), 0);

    // attributes
    EXPECT_TRUE(cert.Properties.Enabled.HasValue());
    EXPECT_TRUE(cert.Properties.NotBefore.HasValue());
    EXPECT_TRUE(cert.Properties.ExpiresOn.HasValue());
    EXPECT_TRUE(cert.Properties.CreatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.UpdatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.RecoverableDays.HasValue());
    EXPECT_TRUE(cert.Properties.RecoveryLevel.HasValue());

    // kid, sid, cer
    EXPECT_NE(cert.KeyId, "");
    EXPECT_NE(cert.SecretId, "");
    EXPECT_NE(cert.Cer.size(), 0);
  }
}
