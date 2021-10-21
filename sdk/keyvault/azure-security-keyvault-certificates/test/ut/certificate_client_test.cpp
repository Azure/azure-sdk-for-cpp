// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include "certificate_client_base_test.hpp"
#include <azure/core/base64.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <thread>

using namespace std::chrono_literals;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Security::KeyVault::Certificates::Test;

using namespace std::chrono_literals;

TEST_F(KeyVaultCertificateClientTest, CreateCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");
  m_defaultWait = 5s;
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  // create certificate method contains all the checks
  KeyVaultCertificateClientTest::CreateCertificate(certificateName, client, m_defaultWait);

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
    EXPECT_NE(result.Value.RecoveryId.length(), size_t(0));
    EXPECT_TRUE(result.Value.DeletedOn);
    EXPECT_TRUE(result.Value.ScheduledPurgeDate);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, CreateCertificateResumeToken)
{
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff2");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  auto params = CertificateCreateParameters();
  params.Policy.Subject = "CN=xyz";
  params.Policy.ValidityInMonths = 12;
  params.Policy.Enabled = true;

  params.Properties.Enabled = true;
  params.Properties.Name = certificateName;
  params.Policy.ContentType = CertificateContentType::Pkcs12;
  params.Policy.IssuerName = "Self";

  LifetimeAction action;
  action.LifetimePercentage = 80;
  action.Action = CertificatePolicyAction::AutoRenew;
  params.Policy.LifetimeActions.emplace_back(action);
  {

    auto response = client.StartCreateCertificate(certificateName, params);

    auto fromToken
        = CreateCertificateOperation::CreateFromResumeToken(response.GetResumeToken(), client);

    auto result = fromToken.PollUntilDone(m_defaultWait);
    while (!response.IsCompleted())
    {
      response.UpdateProperties();
      std::this_thread::sleep_for(m_defaultWait);
    }

    auto cert = client.GetCertificate(certificateName);
    EXPECT_EQ(cert.Value.Name(), params.Properties.Name);
    EXPECT_EQ(cert.Value.Properties.Enabled.Value(), true);
  }
  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto fromToken
        = DeleteCertificateOperation::CreateFromResumeToken(response.GetResumeToken(), client);
    auto result = fromToken.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), params.Properties.Name);
    EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
    EXPECT_NE(result.Value.RecoveryId.length(), size_t(0));
    EXPECT_TRUE(result.Value.DeletedOn);
    EXPECT_TRUE(result.Value.ScheduledPurgeDate);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  auto cert = CreateCertificate(certificateName, client, m_defaultWait);
  EXPECT_EQ(cert.Name(), cert.Properties.Name);
  EXPECT_EQ(cert.Properties.Name, certificateName);
  EXPECT_EQ(cert.Properties.VaultUrl, m_keyVaultUrl);
  // There should be a version
  EXPECT_NE(cert.Properties.Version, "");

  // x5t
  EXPECT_NE(cert.Properties.X509Thumbprint.size(), 0);
  EXPECT_EQ(cert.Properties.Tags.size(), 0);

  // attributes
  EXPECT_TRUE(cert.Properties.Enabled);
  EXPECT_TRUE(cert.Properties.NotBefore);
  EXPECT_TRUE(cert.Properties.ExpiresOn);
  EXPECT_TRUE(cert.Properties.CreatedOn);
  EXPECT_TRUE(cert.Properties.UpdatedOn);
  EXPECT_TRUE(cert.Properties.RecoverableDays);
  EXPECT_TRUE(cert.Properties.RecoveryLevel);

  // kid, sid, cer
  EXPECT_NE(cert.KeyId, "");
  EXPECT_NE(cert.SecretId, "");
  EXPECT_NE(cert.Cer.size(), 0);

  // policy
  {
    auto const& policy = cert.Policy;

    // Key props
    EXPECT_TRUE(policy.Exportable);
    EXPECT_TRUE(policy.KeyType);
    EXPECT_TRUE(policy.ReuseKey);
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(policy.KeyCurveName);
    EXPECT_TRUE(policy.KeySize);

    // Secret props
    EXPECT_TRUE(policy.ContentType);

    // x509_props
    EXPECT_TRUE(policy.Subject.size() > 0);

    // issuer
    EXPECT_TRUE(policy.IssuerName);

    // attributes
    EXPECT_TRUE(policy.CreatedOn);

    // lifetime_actions
    EXPECT_TRUE(policy.LifetimeActions.size() > 0);
    EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");
  }

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
    EXPECT_NE(result.Value.RecoveryId.length(), size_t(0));
    EXPECT_TRUE(result.Value.DeletedOn);
    EXPECT_TRUE(result.Value.ScheduledPurgeDate);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetCertificateVersion)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu2");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  GetCertificateVersionOptions options;
  options.Version = CreateCertificate(certificateName, client, m_defaultWait).Properties.Version;
  {

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
    EXPECT_TRUE(cert.Properties.Enabled);
    EXPECT_TRUE(cert.Properties.NotBefore);
    EXPECT_TRUE(cert.Properties.ExpiresOn);
    EXPECT_TRUE(cert.Properties.CreatedOn);
    EXPECT_TRUE(cert.Properties.UpdatedOn);
    EXPECT_TRUE(cert.Properties.RecoverableDays);
    EXPECT_TRUE(cert.Properties.RecoveryLevel);

    // kid, sid, cer
    EXPECT_NE(cert.KeyId, "");
    EXPECT_NE(cert.SecretId, "");
    EXPECT_NE(cert.Cer.size(), 0);
  }

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
    EXPECT_NE(result.Value.RecoveryId.length(), size_t(0));
    EXPECT_TRUE(result.Value.DeletedOn);
    EXPECT_TRUE(result.Value.ScheduledPurgeDate);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetDeletedCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CreateCertificate(certificateName, client, m_defaultWait);

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
  }
  {
    auto response = client.GetDeletedCertificate(certificateName);
    EXPECT_EQ(response.Value.Name(), certificateName);
  }
  {
    auto response = client.StartRecoverDeletedCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
  }
  {
    auto response = client.GetCertificate(certificateName);
    EXPECT_EQ(response.Value.Name(), certificateName);
  }
  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, DeleteWrongCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("unknownCert");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  try
  {
    auto response = client.StartDeleteCertificate(certificateName);
    EXPECT_TRUE(false); // we should not reach this line
  }
  catch (Azure::Core::RequestFailedException const& ex)
  {
    EXPECT_EQ(ex.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
    EXPECT_EQ(ex.ErrorCode, "CertificateNotFound");
  }

  try
  {
    auto response = client.StartRecoverDeletedCertificate(certificateName);
    EXPECT_TRUE(false); // we should not reach this line
  }
  catch (Azure::Core::RequestFailedException const& ex)
  {
    EXPECT_EQ(ex.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
    EXPECT_EQ(ex.ErrorCode, "CertificateNotFound");
  }
}

TEST_F(KeyVaultCertificateClientTest, CreateGetIssuer)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CertificateIssuer issuer;
  issuer.Name = "issuer01";
  issuer.Provider = "Test";
  issuer.Properties.Enabled = true;
  issuer.Credentials.AccountId = "keyvaultuser";
  issuer.Credentials.Password = "password";

  AdministratorDetails admin;
  admin.FirstName = "John";
  admin.LastName = "Doe";
  admin.EmailAddress = "admin@microsoft.com";
  admin.PhoneNumber = "4255555555";

  issuer.Organization.AdminDetails.emplace_back(admin);

  {
    auto result = client.CreateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }

  {
    auto result = client.GetIssuer(issuer.Name);
    CheckIssuers(result.Value, issuer);
  }

  {
    auto result = client.DeleteIssuer(issuer.Name);
    CheckIssuers(result.Value, issuer);
  }
}

TEST_F(KeyVaultCertificateClientTest, UpdateIssuer)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CertificateIssuer issuer;
  issuer.Name = "issuer01";
  issuer.Provider = "Test";
  issuer.Properties.Enabled = true;
  issuer.Credentials.AccountId = "keyvaultuser";
  issuer.Credentials.Password = "password";

  AdministratorDetails admin;
  admin.FirstName = "John";
  admin.LastName = "Doe";
  admin.EmailAddress = "admin@microsoft.com";
  admin.PhoneNumber = "4255555555";

  issuer.Organization.AdminDetails.emplace_back(admin);

  {
    auto result = client.CreateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }

  {
    issuer.Credentials.Password = "password2";
    auto result = client.UpdateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }

  {
    auto result = client.DeleteIssuer(issuer.Name);
    CheckIssuers(result.Value, issuer);
  }
}

TEST_F(KeyVaultCertificateClientTest, SetContacts)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one@two.org";
  ctt.Name = "giqu"; // cspell:disable-line
  ctt.Phone = "1234567890";
  contacts.emplace_back(ctt);

  CertificateContact ctt2;

  ctt2.EmailAddress = "two@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  ctt2.Phone = "1234567891";
  contacts.emplace_back(ctt2);

  auto response = client.SetContacts(contacts);

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetContacts)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one@two.org";
  ctt.Name = "giqu"; // cspell:disable-line
  ctt.Phone = "1234567890";
  contacts.emplace_back(ctt);

  CertificateContact ctt2;

  ctt2.EmailAddress = "two@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  ctt2.Phone = "1234567891";
  contacts.emplace_back(ctt2);

  client.SetContacts(contacts);
  auto response = client.GetContacts();

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetContactsPartial)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one1@two.org";
  contacts.emplace_back(ctt);

  CertificateContact ctt2, ctt3;

  ctt2.EmailAddress = "two2@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  contacts.emplace_back(ctt2);

  ctt3.EmailAddress = "two3@three.org";
  ctt3.Phone = "1234567891";

  contacts.emplace_back(ctt3);

  client.SetContacts(contacts);
  auto response = client.GetContacts();

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetContactsDuplicateEmail)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one1@two.org";
  contacts.emplace_back(ctt);

  CertificateContact ctt2, ctt3;

  ctt2.EmailAddress = "two@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  contacts.emplace_back(ctt2);

  ctt3.EmailAddress = "two@three.org";
  ctt3.Phone = "1234567891";

  contacts.emplace_back(ctt3);

  client.SetContacts(contacts);
  auto response = client.GetContacts();

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetCertificatePolicy)
{
  // cspell: disable-next-line
  std::string const certificateName("certPolicy");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CreateCertificate(certificateName, client, m_defaultWait);

  {
    auto response = client.GetCertificatePolicy(certificateName);
    auto const& policy = response.Value;

    // Key props
    EXPECT_TRUE(policy.Exportable);
    EXPECT_TRUE(policy.KeyType);
    EXPECT_TRUE(policy.ReuseKey);
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(policy.KeyCurveName);
    EXPECT_TRUE(policy.KeySize);
    // enabled
    EXPECT_TRUE(policy.Enabled);
    EXPECT_TRUE(policy.Enabled.Value());
    // validity
    EXPECT_TRUE(policy.ValidityInMonths);
    EXPECT_EQ(policy.ValidityInMonths.Value(), 12);
    // Secret props
    EXPECT_TRUE(policy.ContentType);
    EXPECT_EQ(policy.ContentType.Value(), CertificateContentType::Pkcs12);
    // x509_props
    EXPECT_TRUE(policy.Subject.size() > 0);
    EXPECT_EQ(policy.Subject, "CN=xyz");
    // issuer
    EXPECT_TRUE(policy.IssuerName);
    EXPECT_EQ(policy.IssuerName.Value(), "Self");
    // attributes
    EXPECT_TRUE(policy.CreatedOn);
    // lifetime_actions
    EXPECT_TRUE(policy.LifetimeActions.size() > 0);
    EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");
  }

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, UpdateCertificatePolicy)
{
  // cspell: disable-next-line
  std::string const certificateName("updateCertPolicy");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CreateCertificate(certificateName, client, m_defaultWait);

  {
    auto response = client.GetCertificatePolicy(certificateName);
    auto policy = response.Value;

    // Key props
    EXPECT_TRUE(policy.Exportable);
    EXPECT_TRUE(policy.KeyType);
    EXPECT_TRUE(policy.ReuseKey);
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(policy.KeyCurveName);
    EXPECT_TRUE(policy.KeySize);
    // enabled
    EXPECT_TRUE(policy.Enabled);
    EXPECT_TRUE(policy.Enabled.Value());
    // validity
    EXPECT_TRUE(policy.ValidityInMonths);
    EXPECT_EQ(policy.ValidityInMonths.Value(), 12);
    // Secret props
    EXPECT_TRUE(policy.ContentType);
    EXPECT_EQ(policy.ContentType.Value(), CertificateContentType::Pkcs12);
    // x509_props
    EXPECT_TRUE(policy.Subject.size() > 0);
    EXPECT_EQ(policy.Subject, "CN=xyz");
    // issuer
    EXPECT_TRUE(policy.IssuerName);
    EXPECT_EQ(policy.IssuerName.Value(), "Self");
    // attributes
    EXPECT_TRUE(policy.CreatedOn);
    // lifetime_actions
    EXPECT_TRUE(policy.LifetimeActions.size() > 0);
    EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");

    policy.ValidityInMonths = 8;
    policy.Subject = "CN=twa";

    auto updateResponse = client.UpdateCertificatePolicy(certificateName, policy);
    auto const& updatedPolicy = updateResponse.Value;

    // Key props
    EXPECT_TRUE(updatedPolicy.Exportable);
    EXPECT_TRUE(updatedPolicy.KeyType);
    EXPECT_TRUE(updatedPolicy.ReuseKey);
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(updatedPolicy.KeyCurveName);
    EXPECT_TRUE(updatedPolicy.KeySize);
    // enabled
    EXPECT_TRUE(updatedPolicy.Enabled);
    EXPECT_TRUE(updatedPolicy.Enabled.Value());
    // validity
    EXPECT_TRUE(updatedPolicy.ValidityInMonths);
    EXPECT_EQ(updatedPolicy.ValidityInMonths.Value(), 8);
    // Secret props
    EXPECT_TRUE(updatedPolicy.ContentType);
    EXPECT_EQ(updatedPolicy.ContentType.Value(), CertificateContentType::Pkcs12);
    // x509_props
    EXPECT_TRUE(updatedPolicy.Subject.size() > 0);
    EXPECT_EQ(updatedPolicy.Subject, "CN=twa");
    // issuer
    EXPECT_TRUE(updatedPolicy.IssuerName);
    EXPECT_EQ(updatedPolicy.IssuerName.Value(), "Self");
    // attributes
    EXPECT_TRUE(updatedPolicy.CreatedOn);
    // lifetime_actions
    EXPECT_TRUE(updatedPolicy.LifetimeActions.size() > 0);
    EXPECT_NE(updatedPolicy.LifetimeActions[0].Action.ToString(), "");
  }
  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, BackupRestoreCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("certBackup");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CreateCertificate(certificateName, client, m_defaultWait);

  auto certBackup = client.BackupCertificate(certificateName);
  {
    EXPECT_TRUE(certBackup.Value.Certificate.size() > size_t(0));
    std::string text(certBackup.Value.Certificate.begin(), certBackup.Value.Certificate.end());
    EXPECT_EQ(text.find("AzureKeyVaultKeyBackupV1.microsoft.com"), 1);
  }
  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
    std::this_thread::sleep_for(m_defaultWait);
  }
  {
    auto responseRestore = client.RestoreCertificateBackup(certBackup.Value);
    auto certificate = responseRestore.Value;

    EXPECT_EQ(certificate.Name(), certificateName);
    EXPECT_EQ(certificate.Policy.ValidityInMonths.Value(), 12);
    EXPECT_EQ(certificate.Policy.IssuerName.Value(), "Self");
  }
  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetPropertiesOfCertificates)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  {
    auto result = client.GetPropertiesOfCertificates(GetPropertiesOfCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(0));
  }

  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");
  // cspell: disable-next-line
  std::string const certificateName2("magiqStuff2");

  CreateCertificate(certificateName, client, m_defaultWait);
  CreateCertificate(certificateName2, client, m_defaultWait);

  {
    auto result = client.GetPropertiesOfCertificates(GetPropertiesOfCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(2));
    for (CertificateProperties prop : result.Items)
    {
      EXPECT_TRUE(prop.Name == certificateName || prop.Name == certificateName2);
    }
  }

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }

  {
    auto response = client.StartDeleteCertificate(certificateName2);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName2);
    client.PurgeDeletedCertificate(certificateName2);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetPropertiesOfCertificateVersions)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");

  CreateCertificate(certificateName, client, m_defaultWait);
  CreateCertificate(certificateName, client, m_defaultWait);

  {
    auto result = client.GetPropertiesOfCertificateVersions(
        certificateName, GetPropertiesOfCertificateVersionsOptions());
    EXPECT_EQ(result.Items.size(), size_t(2));
    for (CertificateProperties prop : result.Items)
    {
      EXPECT_TRUE(prop.Name == certificateName);
      EXPECT_TRUE(prop.Version.size() > size_t(0));
    }
  }

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetPropertiesOfCertificatesVersionsNoCert)
{
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  try
  {

    auto result = client.GetPropertiesOfCertificateVersions(
        certificateName, GetPropertiesOfCertificateVersionsOptions());
  }
  catch (Azure::Core::RequestFailedException const& ex)
  {
    EXPECT_EQ(ex.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
    EXPECT_EQ(ex.ErrorCode, "CertificateNotFound");
  }
}

TEST_F(KeyVaultCertificateClientTest, GetPropertiesOfIssuers)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  {
    auto result = client.GetPropertiesOfIssuers(GetPropertiesOfIssuersOptions());
    EXPECT_EQ(result.Items.size(), size_t(0));
  }

  CertificateIssuer issuer;
  issuer.Name = "issuer01";
  issuer.Provider = "Test";
  issuer.Properties.Enabled = true;
  issuer.Credentials.AccountId = "keyvaultuser";
  issuer.Credentials.Password = "password";

  AdministratorDetails admin;
  admin.FirstName = "John";
  admin.LastName = "Doe";
  admin.EmailAddress = "admin@microsoft.com";
  admin.PhoneNumber = "4255555555";

  issuer.Organization.AdminDetails.emplace_back(admin);

  CertificateIssuer issuer2;
  issuer2.Name = "issuer02";
  issuer2.Provider = "Test";
  issuer2.Properties.Enabled = true;
  issuer2.Credentials.AccountId = "keyvaultuser";
  issuer2.Credentials.Password = "password";
  issuer2.Organization.AdminDetails.emplace_back(admin);

  {
    auto result = client.CreateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }
  {
    auto result = client.CreateIssuer(issuer2);
    CheckIssuers(result.Value, issuer2);
  }
  {
    auto result = client.GetPropertiesOfIssuers(GetPropertiesOfIssuersOptions());
    EXPECT_EQ(result.Items.size(), size_t(2));

    for (auto oneIssuer : result.Items)
    {
      EXPECT_EQ(oneIssuer.Provider, issuer.Provider.Value());
    }
  }
  {
    client.DeleteIssuer(issuer.Name);
    client.DeleteIssuer(issuer2.Name);
  }
}

TEST_F(KeyVaultCertificateClientTest, GetDeletedCertificates)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  {
    auto result = client.GetDeletedCertificates(GetDeletedCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(0));
  }
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");
  // cspell: disable-next-line
  std::string const certificateName2("magiqStuff2");

  CreateCertificate(certificateName, client, m_defaultWait);
  CreateCertificate(certificateName2, client, m_defaultWait);

  {
    auto response = client.StartDeleteCertificate(certificateName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
  }
  {
    auto response = client.StartDeleteCertificate(certificateName2);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName2);
  }
  {
    auto result = client.GetDeletedCertificates(GetDeletedCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(2));
    for (auto cert : result.Items)
    {
      EXPECT_TRUE(cert.Name() == certificateName || cert.Name() == certificateName2);
    }
  }
  {
    client.PurgeDeletedCertificate(certificateName);
    client.PurgeDeletedCertificate(certificateName2);
  }
}

TEST_F(KeyVaultCertificateClientTest, DownloadImportPkcs)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  {
    auto result = client.GetPropertiesOfCertificates(GetPropertiesOfCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(0));
  }

  // cspell: disable-next-line
  std::string const pkcs("pemCert");
  std::string const importName("pkcsCert2");
  auto originalCertificate
      = CreateCertificate(pkcs, client, m_defaultWait, "CN=xyz", CertificateContentType::Pkcs12);

  {
    auto result = client.DownloadCertificate(pkcs);
    auto params = ImportCertificateOptions();
    params.Value = result.Value.Certificate;

    params.Policy.Enabled = true;
    params.Policy.KeyType = CertificateKeyType::Rsa;
    params.Policy.KeySize = 2048;
    params.Policy.ContentType = CertificateContentType::Pkcs12;
    params.Policy.Exportable = true;

    auto imported = client.ImportCertificate(importName, params).Value;

    EXPECT_EQ(imported.Properties.Name, importName);
    EXPECT_EQ(imported.Policy.ContentType.Value(), originalCertificate.Policy.ContentType.Value());
    EXPECT_EQ(imported.Policy.Enabled.Value(), originalCertificate.Policy.Enabled.Value());
    EXPECT_EQ(imported.Policy.KeySize.Value(), originalCertificate.Policy.KeySize.Value());
    EXPECT_EQ(imported.Policy.Subject, originalCertificate.Policy.Subject);
    EXPECT_EQ(imported.Cer, originalCertificate.Cer);
  }
  {
    auto response = client.StartDeleteCertificate(pkcs);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), pkcs);
    client.PurgeDeletedCertificate(pkcs);
  }
  {
    auto response = client.StartDeleteCertificate(importName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), importName);
    client.PurgeDeletedCertificate(importName);
  }
}

TEST_F(KeyVaultCertificateClientTest, DownloadImportPem)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  {
    auto result = client.GetPropertiesOfCertificates(GetPropertiesOfCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(0));
  }

  // cspell: disable-next-line
  std::string const pem("pemCert");
  std::string const importName("pemCert2");
  auto originalCertificate
      = CreateCertificate(pem, client, m_defaultWait, "CN=xyz", CertificateContentType::Pem);

  {
    auto result = client.DownloadCertificate(pem);
    auto params = ImportCertificateOptions();
    params.Value = result.Value.Certificate;

    params.Policy.Enabled = true;
    params.Policy.KeyType = CertificateKeyType::Rsa;
    params.Policy.KeySize = 2048;
    params.Policy.ContentType = CertificateContentType::Pem;
    params.Policy.Exportable = true;

    auto imported = client.ImportCertificate(importName, params).Value;

    EXPECT_EQ(imported.Properties.Name, importName);
    EXPECT_EQ(imported.Policy.ContentType.Value(), originalCertificate.Policy.ContentType.Value());
    EXPECT_EQ(imported.Policy.Enabled.Value(), originalCertificate.Policy.Enabled.Value());
    EXPECT_EQ(imported.Policy.KeySize.Value(), originalCertificate.Policy.KeySize.Value());
    EXPECT_EQ(imported.Policy.Subject, originalCertificate.Policy.Subject);
    EXPECT_EQ(imported.Cer, originalCertificate.Cer);
  }
  {
    auto response = client.StartDeleteCertificate(pem);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), pem);
    client.PurgeDeletedCertificate(pem);
  }
  {
    auto response = client.StartDeleteCertificate(importName);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), importName);
    client.PurgeDeletedCertificate(importName);
  }
}

TEST_F(KeyVaultCertificateClientTest, UpdateCertificate)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");
  auto certificate = CreateCertificate(certificateName, client, m_defaultWait);

  {
    certificate.Properties.Enabled = false;
    CertificateUpdateOptions updateOptions;
    updateOptions.Properties = certificate.Properties;
    auto updatedCert = client.UpdateCertificateProperties(updateOptions).Value;
    EXPECT_FALSE(updatedCert.Properties.Enabled.Value());
  }
  {
    auto response = client.StartDeleteCertificate(certificate.Properties.Name);
    auto result = response.PollUntilDone(m_defaultWait);
    EXPECT_EQ(result.Value.Name(), certificateName);
    client.PurgeDeletedCertificate(certificateName);
  }
}

// the api implementation is correct according to swagger and other languages.
// the issue revolves around the fact that to merge a certificate it needs to not be issued by self
// which causes some issues on the automation side as the issuer needs to approve and i need to find
// an issuer that would autoapprove requests, that is not self.
TEST_F(KeyVaultCertificateClientTest, DISABLED_MergeCertificate)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  {
    auto result = client.GetPropertiesOfCertificates(GetPropertiesOfCertificatesOptions());
    EXPECT_EQ(result.Items.size(), size_t(0));
  }

  // cspell: disable-next-line
  std::string pkcsToMerge = "aaaaa";
  // cspell: disable-next-line
  std::string mergeTarget = "baaab";
  // cspell: disable-next-line
  std::string mergeTarget2 = "ccaac";
  auto mergeParams = MergeCertificateOptions();

  {
    auto certificate = CreateCertificate(pkcsToMerge, client, 1s, "CN=bbb");
    auto result = client.DownloadCertificate(pkcsToMerge);
    // mergeParams.Certificates.emplace_back(Azure::Core::Convert::Base64Encode(certificate.Cer));
  }
  {
    auto response = client.StartDeleteCertificate(pkcsToMerge);
    auto result = response.PollUntilDone(m_defaultWait);
    client.PurgeDeletedCertificate(pkcsToMerge);
  }
  {
    // CreateCertificate(mergeTarget, client, 1s, "CN=bbb");
    auto params = CertificateCreateParameters();
    params.Policy.Subject = "CN=bbb";
    params.Policy.ValidityInMonths = 12;
    params.Policy.Enabled = true;

    params.Properties.Enabled = true;
    params.Properties.Name = mergeTarget;
    params.Policy.ContentType = CertificateContentType::Pkcs12;
    params.Policy.IssuerName = "sss";

    auto response = client.StartCreateCertificate(mergeTarget, params);
    auto result = response.PollUntilDone(100ms);

    bool cont = true;
    while (cont)
    {
      try
      {
        auto merged = client.MergeCertificate(mergeTarget, mergeParams);
        cont = false;
      }
      catch (...)
      {
      }
    }
  }
}
