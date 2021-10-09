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

using namespace std::chrono_literals;

// NOTE:
// Disabling test as the createCertificate operation is currently broken. See:
// https://github.com/Azure/azure-sdk-for-cpp/issues/2938

TEST_F(KeyVaultCertificateClientTest, CreateCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");

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

  auto response = client.StartCreateCertificate(certificateName, params);
  auto result = response.PollUntilDone(m_defaultWait);

  EXPECT_EQ(result.Value.Name(), params.Properties.Name);
  EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
}

TEST_F(KeyVaultCertificateClientTest, GetCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu");

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
    auto result = response.PollUntilDone(m_defaultWait);
  }

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
      EXPECT_TRUE(policy.IssuerName.HasValue());

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
  std::string const certificateName("vivazqu2");

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
  GetCertificateOptions options;
  {
    auto response = client.StartCreateCertificate(certificateName, params);
    auto result = response.PollUntilDone(m_defaultWait);
    options.Version = result.Value.Properties.Version;
  }
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

TEST_F(KeyVaultCertificateClientTest, BackupCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("certBackup");

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

  auto response = client.StartCreateCertificate(certificateName, params);
  response.PollUntilDone(m_defaultWait);

  auto certBackup = client.BackupCertificate(certificateName);

  EXPECT_EQ(certBackup.Value.Certificate.size(), 33401);

  std::string text(certBackup.Value.Certificate.begin(), certBackup.Value.Certificate.end());

  EXPECT_EQ(text.find("AzureKeyVaultKeyBackupV1.microsoft.com"), 1);
}

TEST_F(KeyVaultCertificateClientTest, RestoreCertificate)
{
  // cspell: disable-next-line
  std::string const rawcertificate(
      "&AzureKeyVaultKeyBackupV1.microsoft."
      "comeyJraWQiOiIyYWZmNmFhMS03NmJkLTQ0YTctYTczNC02ZjVhZDBiNWE4OTgiLCJhbGciOiJSU0EtT0FFUC0yNTYiL"
      "CJlbmMiOiJBMjU2Q0JDLUhTNTEyIn0."
      "Ei9gCOYaUlxbB38zQugMBjC2zdb9h8ip1xzs7QUeGyOlx24m2xBW5kbAlWHYq4KtKEExISKJ8d5-bAV-"
      "vkgdB6QLqlk32-hEnJlHKP9An96pmLwWbe4Z8LpeSIZW7_T04xWX9-V2H1VUdJu3HFCJD-axz-"
      "pTqO7p5osPVCkIbDowIijekvyre2WX1usxHxvxScw5X_Am7volHqGmRvabTPkIMN13Gk3ENK0jgWHWkYqXoe-"
      "Jp0Mpty5nQwHmumtiZYh4-Oe8B03wL1ho_aC121v5t9b8y8Hw0LNtT-IZ5njiwPe8GA0z1nGlM0AwnNOLn-"
      "k9MmqOwDeQrf7f85mXZw.Ta5OPdj1v6h3nBFO6OxvJg."
      "l9aNijl2xXHSvQNPVe1156zYgBoudAD5p5r8Op6PmM5qjOYED27355zhd-_"
      "zNJwOt8ssQ8mNKr1BKrcdb3cbNbJkzMFl_zkjoaqCw17pbKXgBhLVZXsyFHYrkYYeFm12CrZNoEh5IWJx-B-"
      "rrkcScI75HgbQ2iOzMB6WNagRAzB-YgWh6-odBIrjT-"
      "PxG2GLTXmF8wnl3F8XJ5GUYNp9j4Q42zfFIswvF4KZxaNNnwapL8Id3BLo17bpMxWIIlBSw1jPN-"
      "T1Vko0eqWSgJoeXLKJaCdCVCZ5CMRW5PUFSduCnxVnRmyt3EJJeEPXhXQVqzPiKBGJpSEvP0-"
      "nWBrnM7N5kwCPnkRNQxGJXKyAwE624cvNinzW2mh4eeWav4BkLzLHsfLCV8cJazA3T6AliICGvdUFcTrZRKQtNOlVtmH"
      "XIAT8mW1Z4qnrtUlRMaMGixIeNKArf9C9MA5-F2EjrTd5pHG6UHCHkg1MYEhIo5-"
      "t5rjadillwHhzlgmfBBRlaIUzcDRysMozKs3HMCTuzOMdjTO06LziD3D7RcaXO1M19-n2vk6hV68ogO-"
      "hh6O9zX3j62C9YkeytAVBd54UpHC5pdVvPKew0redgCdQ2jTt-AgsJ2Xo_LCUytwn4U9bF4_"
      "8bt8Gkm1aefZGLxMt3CA42J4FKQFmgLkEjvwpdFZQMIMtNH6gxs0av2DoA-uf9WzYBe1csviHoTDWyyQG_"
      "472UzmfkeXMI55Vjc77BYG1hIYDxJQ3gxz6pu_GMe1OEU3sM-suVz6CkCUynX_"
      "SakYTiayaEKikAKR6ZkG6wCnrCcTIFwxw52mpBTr2PGk0t6Py-UxkzgX05JrgdeH0oj68U0N1S43CCekY2B-"
      "J2VIbKJ5t7-NhTGalifCMl7gvmsgH1sjCdelPGxmfwLkHcb9B-"
      "fCz5tawDjY46pjjjkcaPdGO2xaLZgCIKkrEXqh1AE8ZT3IXgulD1Zkre9btpm4151LQqcStCzTUWJ0R69-"
      "WPdTOEZ3s2seAb6her1xFTrHFKOOR_YPsrzeK6X-B3Tk2qF9DXHQ1HuoO_"
      "t4H1jvkOZvInp5Er7ElNSjIJeIrxJe1UJqAo4Y-pygULYjUPyNb_"
      "1IjiVcsusjOGwP4e8AIV04ChYyWJELuU1KWkctsZi_8Lh7zBb4zz3LmNa7E2avCGiR_"
      "pUi0nd9eCXUCwtnLqjzmxnY8DQ4RRAz8RqyFrM_3rq-V0t0k394FlqnVftUvXi_ppwHGvNYK6ZEazzYPtZx5JaAjnt_"
      "AK51ypU2_"
      "o56QUucOyblJ8JouBeFX1D29ua25WNpvLhfhcuMZl9qmqPATjGVfTVF7qaFzZVYgCy2Wg4ACe2VTi906leMtM1H8dCwi"
      "DxPPPqY1xuY8DEPPPDZcSrTDwipURrHDFgGQdJdi3Rx1MnItOZQgNWlr3cu-"
      "bKCJ6L8pHmQAaNUzajhT4jeIPYKynToRj0DI_"
      "dLaRmL9FiNRKPsjPXn4QP5bCKD8gRGJcAyQSDI68F5ATWJETkY0dfJjK8Q6CBeJIEtg4AY4wh-"
      "AadMYkyNW3F1ovJ9r5vyKVws5BMVOSPbvScg73dCI8u0K0cxwBBu6eMSnLIicMUcCPx8VFZMBBJmFiUUha5trarPx0mo"
      "U1I6soyPI5tC8h0WfzpTHpwGbIJvGa-R4sw6EjMIeRGK4T_d0HabSE4H00eIZcqph-JIz7so2zKPhU9XEv_cxm9zs7-"
      "IUO5QlZ3Bn5E7kVbjvPtXPZO2r1gLAhfBoV5l5V3g47-_9V5flvdr2fYY1WD-c3dnLXaduaMOZ_"
      "SY6Q8qwS1uyDBJjetahjGndPM5w4Gm8meFNwgvFrfFgA5ILW6k4I44yZQfBbuKW604KqZhSiktyJIbaPC_"
      "ZEUvtbd8cePrLqiaLQh5cUN57aAEUeM2_2Ba2bq0DBwvqT3_JpzeAH4hcH8Hh796fm91S6RsCa4y7sKzUID9N9n_"
      "1OQNX31XHn0pud3JgX1hMMxsk0LDhhezgEJumQUog-"
      "qfilyyO27xvo4YUBuxhFU9H0TiygDY8qoyO8bLNZvgNbWogoKfkga2cmVRgo20Xqtmi0igM9s4E0qMHdhAiIvq_"
      "M1X3oy9S3muHyRVrVELnStcgvl0zPp_NEljeA_r1mfRY4GakW3oM8jf5RwevCFCn2GtjfIyMUFF_"
      "SWJV0asYM9zTi4qVdYwZg8XVDJZlflRXppcdqGg9dGD3mqFL5Bch3gzRdx2kUZsVp1-fjEPuwfQukYh5_"
      "2S0C95DDxTZMoWl83W1SMU_e0KsgCaz7uHXMSoOgyKR0XX5GbOgJAo_Of85NwRfjg8eoq__"
      "xuoeTz5NN2m7TGqPAmVK0xwb-wFVwqzxsyqWmF7m6IdNw02rI_"
      "ciYkyGrtbS1ggWwrCbYPKm7OTtQW6lw6766EB7QFfH_H2fdZOMHI8r8gW79DeRE1sBwvaJZ9mUU2_"
      "ApFbpY3uLgFx7cpDl-YtHYzxnUhZw7KCVtfC-CzWSxdrN1Z-"
      "ytalpZN8vIsGr2B0fOnYgM8IHFc8WFZPh5T8U2oWDTRS5weFu4ep6sArDR-FYvhXR2VmWuFwz0QLPcxacWOc-"
      "gdF09AvA-Q16874G7sQ8jeslBXLMPP83147WfXbBGvjPbjepmQZZe2XgxsqFhUPRO6Q66UXPvN4OT-"
      "m9aGLUdGdinTlEIqGbLgK9ISP9XMLbtukKCh2NqepEjF6zanoSXHMvEyFK3EwHuHsrs3_Jiv4_WQ_a-"
      "XezYoJrQQ6buix11pDbHWvLX-_lIRh-wLI-Jzm5WsSwca6PcHfJvAgHF6If7G5VNfqy-FKFrWAwuuiK-xsfIo_"
      "hK6i37pJhRmHs9UsUw1A3qus3XTjI1vgbtKT7jykJUiFSr8kZzCibpNQE_XGTG-HDdXSS-"
      "VeS6obDWR9ec6M68IqaZVo8021F9gkdFJ7-S1gC_dhUwrrmmPFhJE_"
      "iD8IAcrs7XiR0ivirpUeHLyj0MAd6qSOdnfY64YdPYW_BhhzyZp90TSl7pLfq-"
      "X8eaU1ub5d59sMRyJLIzTYu7klxaJ9j9rgADjz0cXEZi8Bb9Dm5RMfjXiT2ouws3SGjjT2x9XjOTP2GhncTOpgkP5X_-"
      "1pA32RSacTOHWzODzOxshXqmR935R9xzbVXzC3lgNYnc4jsmBocrK3v7icNIzr2zB3_"
      "9t4ACXVTy4Qa1JBao2xZ6iG8FP-_WNNbfgbFnSczI9Xj4I1wQcdvxdph74fenOu4392cPmrJG5O9imKvS3_"
      "MqBdCNuRjZj2eb_SlGF9TBeRvgATMXNQccOoMc9RPVkqEj56a_4vIq-rl_0T88tH9LrI-1gDu-cOQxPIblmJmG_"
      "0QRL0E5k4xuAVbu9yvuixn2YX7vLA-bOcMlpudsUAxBLbwBJPCr08ZGCuexzkBnqYI8Q_cA_1nvFIF1HRg4o7C_"
      "cwecNBKt4ci_7ot6W-Vz3upRI18qDkMVvdnBRfDo8jTJ4IJOek2YbLSC3awng3P-"
      "T2EXFh56N37G3vBdNvpmQVqntdEdbhMmEi6y2pfuWKsAocIKdFtdwKhBolGD6AX6pD6KFTfwxvlhaJ7g_QIHcoa-"
      "EpkFLbAu9VDMlR8zdmJfuWlaHw1OdGxOgmUbOuWwgeaCRMH1p-4R_cpGmRw6X8fjD-qXDPpHEplsclFLoRqRk8_"
      "dnWjBubYVih9EEE2njsK5LJAnoHWv0ea0jnRZdDtYvuOT3vgsghPrD3fIDeh_"
      "yvMAZzR8h654BRylvD8r1zWR6MkCU9E72IfV6ILbf4y80ja6lO_"
      "gUMMwaOZn5BJtszUQW7qAVNa2L2HQYUqJ6fBydWQNPbTM85oWqSL3ADet0EdN8dvJSlnlWSk3f_vmahUv7Zsx3-"
      "MmQeSyG1Pg7z_B8aG_8vcQGx1E2PwGq9xsv5BDUDNtM-HOZcrRgskzwhQt8xDseAuBTqHgBZA4PnqGiRHOScRjmDC_"
      "8qUB8KP63OBi_"
      "SIVjndL2JACRV00y5LreGmy8ah2v4aUxmt30aLMD3QiAKP3pRpUfbp3TeoogG79VQeFjDDHwxCNW6WpA2Xj_"
      "NQ5DnSfNfUKG_HrTBK2DVvQSkt-"
      "hx2EihHb09nWg8HGHyjYyO9P9swlqkzKoAoMp9qV074WZ3TF5KiPYRtOHHli5DoexNvsELSSb85zHvrfZGTnM9rYc2bf"
      "DRc5Hveq8DVRHEqYtAotusXaoHVhbkkn5eAVshlK1ZIyw7wq0Fvdxv6Ry2p3ahUDMoipVKV7zJyOPf4CTdiIX2lWwwl8"
      "o1IALCAE9FPiA3hmod99F_b-b74TpnBU9knbxM17bDYzfzP-"
      "5dPyyyTL2p7whcDbqEtLXl1S5OVFj3kF25JG20OAhumsuxyBCO5yQufB_"
      "KqiBNE6wnbfln8ufs9CnWYIdjdKuo60uAGfV2IxNdSHiDAViiZzf_nfa-Cm9w15QLm0dhc_"
      "00Ul1nf45KJ0lZ3754OKfVts2OQ003WA1JeR2LKRlXaavcpdPIU9KcpW5AFilgAr-"
      "w94cFRaCwxNdmCBLCnw1o0KVOWRrQeRdidoIiLLo3VqXLl2aGFVxJeDuTdL0Q-"
      "tXrpQac3fkRid6TSuu5FCTkQaPLRwJMoSM64_P9fXg-4lhhpqwiyBZp2cAe4oNAmdd9xHTX3gn9lTdTPi8HLJUm-"
      "hCMvE8_pt_dTaiR-xCJ2SGVE5W6uEEdvvk484YQa47CWsndCu8TcD2Mb2x_"
      "NqRWtJBkMhEXruCPrdCh5rNA1T2uD5KGzeLnse3GBEuCj5ygmSKq_ncTn4-"
      "iud7U9HNaEpKOtJD9NufAuwTnMMQTPylEM5ZPY4lvZM_rLEpgtgujiKz_FTcGMPA78AJ5gXubJSGCPHtO__"
      "OEB3gG61S4GJQK5IrQKlCcd-DXf53PIQlBMRm-GM3mKrxJwbBm09lKAv40St6IH85oVFBJZt_eb5tDRW9HyVAbG_"
      "Gtuppcw5r7xCeEcnelTc7BRHS0eUgOzxc5xjfwMq5MLbftkE_"
      "NtFSio5gY6h4KgbxIgZNpmEy35RpDi58eNuSISwh2If9Mhn_"
      "9velKrq8uPlSvXkgvBB3HCE3FySJ469CJf1rzdKOcVCI9fZYYtaJflkWWNJcMD6TsxcuQ-"
      "Qyz0GrUJtEXYPwHyzRsco7Z4OGA2QMPqJjci5WOXYh6hz7pcWwhBh884bBJGe08sg-"
      "mxg9yagSNHflWpEWrbrLD9HcRgpu87KP561W5pyOsueWbeN9vS2ya83uf9BwTMxA9ssEFyQ2JXGw-"
      "O3yWKeKcpMGiGJkk7M8-zh3Y79934oCoD8psPcNYA9RziXrk_wBquC_O7OOLoTKyVh586bi_2dhYDTbWCW-"
      "EszBJNu6CeNgpoRWhKpOHlH3PR4ER02HXhrKIHxYkOtA5ka23rWn__Dy-"
      "pX9NTAmROMgsbVBzi5IDDpCeH1qCa2OFCy304-ieEwGKzcIxswhMY_5WvI6rI9IUrj3UqK2sXNCp6Cm-stCzMVRcS0-"
      "XfmcZvtE_DAXXt2JYp-TmWpPmR50yqfl_oXY2x05m9mfF_i10Kk0Q9s994ZX1wGdv2vo0QUMi7rtVMpuxYsSO_"
      "bTclHGs23TB1A9BHSSQw4m0i94f0CAmT_avRpLZB_7WJ1DkN1jQB0Io2FIH7nStqfTrG-"
      "3bAVGPkuTBgwYkYiNjd0qgduYv-P_Gq683TIOqnlinS2M5hCIhZkLJJa2MQTetWonwGbKcORC_xSNA-"
      "qcs4o2wqrqwYvJp6FkD5W0vsuNbsedFo1lwKM7vBQpaGQOmovJTTPwX7qrfzmNhh5UrXMSTiRD5tGCTkExhCTcfrv_"
      "Kx59MRWGR-8eoGuHMWWTrITDeNe8G-"
      "KEbx1hfqXYPAMUFCLu3fDBIuv8dRVLuSiPhcyTxtHgOgEPdnjz4PegbYquIZXF4AnamWwINkZg78CLuKfG8MewPGDYYh"
      "B8ARtS3YT1WcsSm3frY9t3XFTJ7RGQ647wFOxgTeonQm5CC-kk31fX7wjzF67LeLa_"
      "z24Mhwzljofj7EiRhTIduA3Vk8YXoECvtWzhtxP5uKP8vzXVr5x2LP1OyF3FGD1MOx8WYWU0negZI7AAPWVeZuOHZ3tr"
      "NUGKLvgprAr8uoIpl60tSbzA1AVAW4IAo4nbmAxAZhsebakzoP4SWRHHjWaE18TuxuK5dcJC-as2fdBpt0B2Akd6KE-"
      "QeXNtxTRheV0VDqs0ismQlucO2eR_"
      "pn2XHh2Yc41vOZ0n4k1HDGA4NUitdpq4aYvxtHE1jcnnwqdp8HPd8lntG4Hp8RPjdhZ6pProi_"
      "4fBT7ofeUrcFfI9SB5gS6shPme2QZYaCHOnM_fNWI74RhTuG9F14wGcPOP2dagp-"
      "CD5ny93qzUHiEFP3HuPyv0gTb0Pt8MJDjhGTjLmwnVbXCNjPNI3GeXxk13GBNkM_Eff-_5bFgUqZzX6pANe3zfsO3Mf-"
      "XRIbU4eDgEo0NwaAqmQBEbPJVdk2uKg4fLE2S7LQ-00E1U5xgko6I56k1Vr3enpOYJL46YQLHJj1M231in-TUiXCj-"
      "iNgq4hfedC6qKaZI7nP9shfrmZ0RDtohKD01o5H_GQPN8Zhmy-5wKp5dPSH_"
      "lcHsl5HQMgaOcMewtXKVs1CRKbSRLvCTtzZnyo3D-2nyCM-KG4ySUiand6xJglFAMmO4c0X_"
      "aW6P5HihhBgNTY6o649i7RuyuKAR6VnLLZK6VsaiJGQhJFeoEw3zvdRVzr9N-p2C4Nv2jrstomVdAUaBsFRJoFU6-"
      "Z4pD9_vp9Ivr2Vq9el7wcZcwXsymavEflh34aX0wOFL0Gh05EC_NNLtWzPKVqcWSQALcSM-"
      "k2I7MYLTw9EQvZNozHSrpVVRnsmB48uUIfIPrjfu4LvYFtgXtc_xkTTddiZzy1SujbnMySMMJHkgEuEl9_"
      "G7yrxACjqQsF8bUDUeG7ydOzLLVRpXWGOUvTGvwOYOJ-"
      "YH4CA1aO3craLq2NrOaP2gz4w8K4PaGRNffLswusqWAwlowSE4uGO5_CLSN1GaGQ-"
      "44AeiZ9vLLYYliAakkygfGMlMYzQrvK3J-FcpX3NRHr8cLnoAPUi3Yvq383Iu-"
      "5FNBTXn8aABi0cA39rWBp91K1VeNvv3_k1-"
      "28nQgzLEYIYexniKjXcQq3HwFZQNoi9ym1CbDSqGz7oZAOz3QF0N7GUGsNHXkA_"
      "yf3q2bSvDZUM8mexwQOBJpAMyQwr2_4ooCRH7B_"
      "WOgOc3FKGb3y9iq9ksDmtkZD019rNtWGYL2tg63eOvUtetN7xI5iXFwcrEI84nOwLWysnXVhSKXSI8caR1nf6XddA48E"
      "-7A8-VUmWmKyWnUly9jklxj2LYiNb_"
      "p5UzljgGfmmt76kuxuRXIOf4ZC8ugENy746nmvHYM9Yh9i2AFeYkeGqrze7qHDr-pn-"
      "GWWRzSbM2J2VmKvW72Lsx64UUNWkjS94QMeb-eCAnrlvws4Sh3E2O-Zzr2e3MbdwMVBbKIzFI-"
      "JArX5CIqwVo6RC63GRzWCLz6S7LPkwvnbKnbRN-qy1mq5osJNan-"
      "UEAxW2vWvynxnvu9GBNODq8xFH0pORvZfiRkw6oUiNZkXliV1jSSLis7JXWpfHtj-"
      "yl6YupaFnCkv3GvINtQfmpbeBKhD9HPsKfXD7NAK3qYgJMcjhlbrQjhv-cB1oEpirqse_"
      "sNw03qQPDYh4l9MGwJ13n7KgUjakfoQ-0jmB2xND6CKWIMLV_"
      "7nRmRYSYgC2JrsAKOd7fy9FmoVhoh8kJzxd8YAcbcoRMHeHW6Jq7C68BjADSJ24D0KBzqjmQ6BCN1dC4LJx5pigXF-"
      "7ADyVPJUtjNk9Oe8bNTK-2CvP1Um95WUvNuzqUG9g8seg9LfAn8oAAqcYX_0g9d8OI_v6n1_"
      "Po38yl32uZgLrgi9eFNRMT_k02G8y5ldVLHk2F1UJjPnBpGqLDinf5_62kiqKp8FkqQtnOnezdFKKOY64W_"
      "bqWK81fzkQoo5AS6eLtLiT3aMdt2fZv-AYGyy_4ZgCiPCmFSYw6v3JjWBkAqMtDhsQN3-eZjmSN11dAspa4Of_"
      "6MuIlFRZ8cN_-YKXAe719Dm4yExD5Ts6w06U-XMeAqF5k4wRnq4hZg1lCY6R-"
      "6oFUTF2I8uUqwxZBDPiFq26phFdM6oL28ZgDdbEdRNDNa_bQnISitnt2Wk1ZqasuS8XtN-T76Gurf_"
      "yNYmdTOwDAdbBbMEezAMdD2doAHZ7D7Ygo8nVda4yqGLm49s9Q9aCqmzBvHKh703eTcsDyc53-tE-"
      "cGpGJBj61AcO8KqgYHxxnrBVIqqvCjRB_3X_Dnd7EWJAo6HEWMscqELs4ZjFDq1_"
      "rWrD1EhcPKpyf3QwxelDd1fvaHtEnfq60kNmkIiI-KU2pFyRGW0WUGBWqpP3tkuUhxc6NjOvMjld-"
      "9NVomPFXJdjY9R279DShF9HR2gxb2_AxFd1fbLE2xjL3-o-vTVQhuY2ZJ_"
      "9qOktv3W8gHyXLEBuZumKMfRk392DA8iTY140pqiqYYZW7XskediIKQov2DX8seg888TmESxofBSn0lb1lZABfC9JS2r"
      "gG1Qu-MFEUKtMxn5-Jhy4-5ZqQumYv4JSG9E43YiZ-"
      "mpwwNLZPTomLzT9N437GGP990aDde74sK62OMH4LV0rzYBRFhoHOviAmEGgHKQ3O99ZUIzQkRgsvaq9cbE7qicTXdvUW"
      "pNb1m--96Nj07CR40h0EblcsFCGNbP-4zN8WxkpFaodt_"
      "cAVXtWPtQz2VPkwgp9UaHeD17M7R0SInCwuHykNK29vIm9iOhiyHb6Un5BviYPXwgoeVNu2srLvEmvllSd0Uqw8qOLnh"
      "EPhNqzIpgwYvxKziOd82Vu-KVbrDOGC3q4BB1GK1ygf2vvI7L8ZrQuTgZLi0CQD9bZDTpqHkjA3SHpBemeDDxU_vzP-"
      "NyvjTMq7BiiTUHoOAypJLnDsmIC0_ihJvcA_"
      "oagpkbKEfOjuLlLVOsXDPwZFWGzs4yiNFEM0ZkK9HvAV96OgIESvJ3sFmkRBqu51gZwFcm-_D-haV_F_"
      "SOMxY6DGh5iYk7nH8jBftic-EC9aPuB7rUUfajVo1SoHJXSoQnIfcwn8_"
      "x2wIt2yTA1CD6yEhjPRtRkAhBaLQsgRen5HqRSJdfVgBxk3TeU6wYeJEF_BkLOuh-"
      "uxuPIyLpY4pnW1YjUduVXBHXf1Oc7DR3AtojEviCdpVfAr3dTuFjQ3FbElUrQZ1xczB-"
      "fOe9iYINcnyUZ0HgAP7ft5TNbWWPRRavsFI6nkEO7UXUdZ4SmlQkjGV6omjuAytD7ulQkHQltOYNCtIUnKFyzynacqdY"
      "zNlM7OOuCwuSxtewFWgG0N5pQmyI_"
      "COiXuPSTEFZ0U0s4lJd6AwQvQHd8BFTFTzx1wFsBUvytPhOcrZ9wfdya6yDEVbgqGOWVzooiNSqVsMslP2-"
      "Wmg2TlBA24q-I0jHpm9gFYbb2lotpLX_rQFK7fS1uiSrWizv25xlieye-XHo3wRsL8hZ8w00FUgId4OuwM0H1-"
      "7qNkwZCitmW3lMPwKmq7G_LqPSEd-RWrwFMytIP1YrfkiRg7tYgdd3B3R50F-vQi9zBI2AcNpX1urE9PahWIcpL_"
      "MjzYgCQPUD5AmCYxOYYJe13U3gk49L3trZlH9W51h9CSnB6RDcFgwGuT6MCscBJjmmESqVBudyIYKRG6eUFVpVnYNimj"
      "EPlXk2nJ1Bh6GgIG6KYRb7plNuQaN_dmhHwy-zdOfm_16H_"
      "NdgZj8AOQoMcKXPsidNPqDseWNH6r5LCUdSNVwo0NfY4vUatPk3t-nQ3YSOQ8UVaKPQg44WR6qjoinyg0wemp_R_"
      "ETrcbZMyeDE7tasEj5RFQfnkGyGMYzAzebIS5qlIzDIyemqD8PE5noNRi4vWL7IkLbHtNJpFQvlVylzN_eK-"
      "xanJCUOarbNIwsFQbkiCyNjdapMErcxl17Mdsd6x7DkAkvEqnHTy_l7JvGZ3Yb_meSeJTvBZFEM2ozm_UdUtLn-"
      "KSC9omflFYN7zSjAm3IbM6PKXVXOC43CMuBDp-JGgZB-"
      "0QDTnOlBZjaRxTGqcTiWst2QNgl3wWiJFtNa1r7JPBWv13YKKlqQ0XpP6r0JeHnN3iUoTjT5gzwzy639cM2IlLClhZH_"
      "6p9DEsLPpxNpfvNmE_U8AJ6JWvA4gPvd0MNzCsD-X3g_Fcv1cExXAdlQ5kZ9atteE0nTHjVX1mVEtRJTlE9O_"
      "WbQuY19Ln0UQYHzHK0J7q165nvN7qBcpd7Q0bnE63aSaWO0vmmjSFK2fl6z1QqcqMjtbXo3TMv2IBAi8-C3GV-"
      "YyJmsDGyX1KvUlOm_VwMhnZ0sDOGrH8kN4tG_Rk-G-98ep_Uba8BuuOtLAbgBm760tk1jVEuii-ARACD7sA_pbHRz-"
      "orfScWOdvOUB8CY_37izfljrYeBo_VQTlKQSpTjwgvD04rG9ShfGYU38nohhTNDJAtyAJ2-1XAerd9tMZCuhxRzSiqC-"
      "F72FrV1Wui9p7hbwclKSSuqz5PyVn6nA7BcpUFsMiwFSZophyGndRpaSQEL8vndL5oJZHVZ923a4MYvlYJYh515Ac2lI"
      "vg2dbdDSY9HAmilOlXkkMk2m39My0HkBHZpXuPr6NoSKCGqqLkdUJC37NZrBVchX_"
      "hZwXV3T01h9QNNnYz8aYmbItPTNMDMprjd67QgufRh2H35t6KqcR-OR33npX1VFmwG81k21E_"
      "zQMsz3l75DBYIq0gmibz2jiChmilazbu3ds0k1Q_"
      "JO6yfb5o45y4XeW4l82u6nvz2xaLjxlKMwwwDKPvS034IQHnp5tQHMliD_"
      "BBiSjX97kKbak1SA7X50JO1dUHxYKjuaD0vMI3rfoq5JrDdcQzteIuwIsVUcSfb5l8oDMJbu74-yi-"
      "wUNiMlMGmqvYmtTLMCPCuq7SjLTXk-njB7B165VnI-Us3nf80JP-fz9XcOWS3t2aCbUjP1RTyZaPshXJ3wDzxt-"
      "YaioDuLfEubHQkbhDXD6YexmZE63ubfSQ1L8Rzh87Bmw3vXI6GTXolx4D3ACEAxPHQ21cAtPQEmxMpSMpyopllAw85ee"
      "vu3SQR9xSr0s9CDbvS3hRwNc474H5N6WHnJivc7yI-S2BV48ESim5fHg65v-"
      "oqBAqDNOyjBrf7Tpkp4xMuNX5FtPh4PlUfLG_ojT-zHkjLfErRXlsM47QFw6eTjE-"
      "6AC3VslLCqbPMjSwbHzIp5hh5oGhSoyWQkLLpTK3gUptPo3qE9gIR26zKMTQYuG8C0wk9E54DkxwVW3utWaXc_"
      "C3C5SmXxSbTm1u-"
      "TrSLFzkTqMMFF2pRMSODTLB2FHqFA6GdYdVVzgEEmpP1XVvdtqEXFM8hdOLXkrmULtROEdF47yRRdrcTZq2FEUMwfAi7"
      "LyEmtP3-tgSSyUL8tQTgnjZJiBB1U3ES4HEhq6Jf9EshVO2rveR2m8v_Vw6cIHLukpnP-"
      "XlTbzxOUQC5OMm0GSy85VhEbKlVFFAhDKnfd4Bj00w6FAJ9IZScUnGV3baOKu-"
      "lVsco79Djbu8MPIVBSa50m3bxC5kk7oXmThfK-cdLr_n1L8-zEl_d-"
      "lYotbnmkXRIaKtbMG8Q8FHlao8Br12IzQxrsxf8mPU69KJ8MJoLCjDzkE82Z1S6WdvSNFODpN2AIiDi7kKla7dg1oMQL"
      "sDYzxa_R65Y9oFSUL-obymZEK1FzDl6ugl5vKoozvrAxxKYSSrN52VvzKVRN820_kVSusgZFsQZPDP_ccvtT4R0k_"
      "hiWlcGm-3a3mZH63IpIuCyDxtDaYAK5FylBvWSG9rDAse0oexyF7zhCN8FxHUraCM5pznDyfW1-"
      "gB8rZ6WGoOfa63AEvJUzA4x0yqdTOmj8prgoNYvhEY-igblMMPqgqv5_Hgi11DlofJ5XU5x9j3M4G3yy_"
      "0Dj79RAFbPrabF2gSee_otnTW-XfMLc0qkpAfm-LjpqrrDDWHYRP3cnuYdns4o0mt919sdMRKR1NXo2aP5zuq-"
      "qEESberyE5dbAZtWnycdU2po_svf6NvjlX_qcyKwAERy_"
      "IndVuqENGVgLPTP6Ktz2TsXzNe5eVlfUY0viRPc8BC4PU7148NFM9YMOq15b_5_"
      "yNoeGH4qGFpgRELXGOMochTurzSvtUrdwR6iJYUbatj4WrbeIQscSs4sIGvw0kKjYBdXj-"
      "itPW3b93v9Bh5kaNgEXX5OgjfJXrvA9rRb8GI2aW0m7BgEY2w_cLrfoE1TSlx2_f_"
      "6ccUSwvLttWcCaVwpO6zX3hHkD5-PqElA3f4udDobs9ATaA7MiPnE-8QiL_ZOgJaiz_Yr7LbU5ziewSo_"
      "Ss9HZn0aTfMT0G901vMRk24xkfKxUjXt0ag6eaifFled_qO39AhBnfWGxfMr9BTATH9zBwVP1VZx4XFB3B325e2CRb_"
      "QuFx3pJ6PQxKdfyg7AGwOr13ZfXFmfzIxYjiGYJc1y9vz1-oN4E9FgUHMKXDIotadiFMkja3kAba_9pny-QDvTR_"
      "e1wk3M2X7dUadAxWbF65BQNqYQsMoliLEHukQhvHfZx_0-QCXETOyUwiOP1lzdBBV4V5ki95-"
      "7S2a4nJzroUQAZL6asS5RiE2jJthzOefQlu6KnWuj7TXdjoaucnrpxza7oVB0O9Lf9x032MrTyWRd-"
      "brylV10FmfXu3Rv02bMQdEvJmw2AGae-mZomKlBthJv2qDvx82uAW6Dsv0mwX7Lo3JbDpdGAJarVaMLD-"
      "dsoEKhC7CmOfbhMTgwEBn4OhS_hsyZ7bxHzgVwL9pwnwCRPkwicAIz2Zyw8LM8CBv9K3rJaM27Fd3Vp_wmzyau2Mmou_"
      "LoC4U1PzVEL9oJ4qrFBpKx53jnhSkw87hvgkUFL_Ff-"
      "UV9jebr1KSKglK6wrVVRcMLe1Sm9OOc4VLSwYK5mEkEVtoQUEcAKnTMud--nJ-Zg-"
      "t6IVBdKo6eLkXzrz0W3vSQ8IqMFji-WUaETt0LOzWy_5mwWmaAIq3thjoy6-"
      "2YAqB9STda4JIHcz5T6G8Vm7CPVxoi3vGB33hVM4qTteItuljgPSx4zMzgrk9uJdDgz8gC15HvKLpBQuAfZ87d_"
      "ZwJJsQnYeBKI9M-5YED80OV9oVMXTp7UB2VRM1qG9itYBpdamNmU3XSMb3d9_p84yqBTwdOmvgTecBOpWS0mPwB_fu-"
      "tiW0n4hZQwYweiIqTfwt8uJVeHB3FJUL6-mvaJRAQ-DVIu5IjL5XRTrmmFCBT578BAC5WotASCkB98OI-c_"
      "530n5nvIDFcViiXDorjUyOHXTVTeZwSWcDv424dukAhqYiKXVlWM95Xi4yxrnr0l3bdszmkrnD9-"
      "LeE9c9M73JfJWIoU1X-e_3G0LO_MsNcjWhBGcbSPMXoKFeBiRw4eEUS2JjOtm8y99WjzvG7QKcJghujNrDQEbSo-"
      "t7DUntxEgxFI9PY-RoK2SABFeZkZCtdZcSlXrt2C1Dd0Q1joyfGHNxqdk3sYv-_zuMaDiZ499Q1ga8QblqTp9EqScu-"
      "Hqs3rPUy_-1i_dZF6hu1KibuDyLXyezF-qoU30ABCIP8qMavBdtziKEgarM8MfxdQH9xdCx7Ak8Em1v0rIg6isu-"
      "UJHrZWltl3oWNVGyGvJCX-7Q6nYCwSyA4pEq0BOF7HXt9MPmnDAoSPo-IQ_"
      "4V16o3cDEiw1j3FNDPSi7Q7t1azwYmJQGfcSI3HreUyUm2Ek1elM0lRyaNLxQcb9sHcDILXLcv7aqNW8y5nrgEuK0PU4"
      "h01SWMLuQhxJgUjKNZr6GcbzPLTCiRFgpx9D1VJy4oMDnez41v2opf6iOsmlcGaDg0L4Ll7dijFE3xZqsPISwGrr_"
      "H8ePKlJ-O4TW7xrVbU5Ad9DLP0WutCjtGmTIKtcthHYL09TUllE2DF_HsQbv_"
      "Drv6WhZHEEesFPccAFTqMbpXQVGvkOZZUfcSLV6blJxhVz8VnrTixFtp0tgf6xmRvHHoTsfEVZN-"
      "b10lUyoGy0TFkeK41KrXNhiA7s96p6Plh_iMKNlr4xfh_HMp118c7aiLw9_mR-"
      "PKPBvo7APOcy9uhzwb5I7UPOmAK8GH7qd-t6NVO4x9ADIdLF_uFg-nNNTQNain5qcoX2V6xLVg4pLG4E8n_"
      "DdOX75zRv6CJzNaFZvE9uE7VaS8qnpp-5EymWeKTR7jdg9Shk3mHrk2b-sRRE11TXRrvHZsIOeQrYQvBl-"
      "wgFaLScOcnzfb_W6-MzG5qoRpWSyslpAe_niXmmx7WWLrmIgI_"
      "IgxAIxejK9zbkePeZjlqtemQrXsrexM0HxobCVkNjI7HGj6n65DBbrAom44rV7lpIq6TzEYsYFR21ThkvTV6whFjmZn6"
      "Y5igkedCSdRW8iLVPz-lRfS1uDtnNF3-2iwo8wvdKcyh2t5gc0h_mAbcBMILs9eERQX8AV1S4acSt8A_"
      "C47XvuA6CELZaVemt7iZctBliAzo-fCf3uDnX_"
      "aF7QEEfNEca14xOWHOeLTfjmTQLLpFpHeh6M9u3RtbTSdbHqRM2STD9V6d1T6CSYamwLbE464wtHNhjEbhgkw_"
      "hPj2OH9dZ8En4rZ6x54RrdgaZ5UKN2vCXDQGl2pttIofOXJTeVIeYsyRXHZbPnL0GwmKIuaKiRdz16FRBFI_"
      "8bb3rlwq6dJGwB5BWOGHLU3_87i-7pLU89UK_"
      "ml4U136UG9L6B1KClKXY5qPoBOVHiC9KDuKrp8ceuy8g9lzUVj6jvKV5r1Gd0mgRhjMzp_"
      "ZbCJiZfdFLIWA1ZI1MBMMYyIhkNhXZSgRlD8Pu-S6_"
      "oJ0lL05McJCKzUFDD04vRgPTS1RVrvCMqXSbVkHQt5egSQLTR1O_sHFTd40McQmt-"
      "KdGGRGw7LwhDWPKQ3CXttUQ6PFo8JeOM6tjCrpieYK4p7maXdGIiZ0iuwqldNKJThbNs3m2-"
      "C3BiZg5G2eqfIDrDZ9a0ItL1KKwRQMOXND48cFftRbKRUG3epxeoa6LdxGzmIWkMdLaJKPA1TDq77w0r9_"
      "Xfc75Ft15S02Ny5pYMHEC0Us9ZyWqx9uMveqiyBWRDFmUjnl3FEp6qykykDeGc8Ve5e3H2cwPIltOCc1MntdDlv_"
      "ikSQlQ41go4JUlLGkFu3JSt5xCgucwuBvNgSULb_jdJx25-"
      "fimN8g1ChhYTlTO4ln8DcuI4mOBDt7LTY8jTu7sUEAue6YdwAMh-G7R_8rYxn_"
      "DtV9mkYS6eABu0iQf75y5fQvv0cGW1JSDfFWpDO2cWvNvqI2YnUE4Hnsrbsjyj_"
      "FLb7cX3k7L9qTD0ui1TZd2ye7TPffdvoKuWg4VLTYraSleb_fDuAMoGoQYeFscKzd-"
      "xWRY9zK0wy5gWoD49fzzeEdUgi0jO5dLmEjj7V-ZGoZ0qpJnt-uydVIuI-vUdQfLfO3zHRJS4vRU4SpXG-"
      "ckXuHXNBRiQ_0ipSEg5rwjPeCc1hsrhGO0blfHLgVZF-ImVzSGzH82yu6WewFgMUb9VZSyaexIphNOgsxG_"
      "w2qF48oz2wFIoLERFIMwWLIyLx9pTPO5iUb49UP-wpJxDrtyHLeeJNiZ6CN66R_Sfo3YW3WC1OHppS_qrkoL_"
      "IMqu0iz5QBHXTvYy9sTGKzLc3GrTqivZ1T1g5fQ_rRnkkKGK5h7VFiAXxDH9YhFsdP759x_qUJFBmEl5DL_GyrwopWR_"
      "eNUa4GnadvBo7bKfTeHaF4DW7Ju2k38jxM26v6rVcWrM2eLamAeup-E3JzKZljy80vAdXs_"
      "cNXZOLwwZvxaBGZUNpnhw1X0kh0CCaIt6MJ1Bd7oKIa-"
      "aH0iyYHhkoGevrFwQRk5CcqG0tcPFvPbGBZyAb6qsLrPfYlowm4z6fBbFjHtew_"
      "MgYPiHaCArybDBII66u82131zXGVV6mj6h8ISRa-moagg_HPBx0DAgs-mQfB2zSGnop1UQHTgzjGPIoma_"
      "2jfvYoIxdKQaAmruYKtvvg_WO1q020XnS2CC_8YkX_lUETDfy-"
      "i0vEytyCdkfbKYKETvkFT9vO4ppp7P7LyqE4TITelU77Fm__qLVe6FM_"
      "QxqlBs4gYwUOi2YBB3HK9qPxovYBJoh4U40DXuC98UmAEEEImZ-IjdOuzS0IPkfBkfA-"
      "gB3XNrtLANE79jTsaMdN2c1JHTMe81YEOlbcNkvrgzB2zI63AmjXsCCBQifg2nhbYjfl6hXpClvJqufPdBJFTkdfJEqj"
      "SnI04r_igm6_lMJJkyi1QwdmqiyugVL_ZDRL2GS-EjxcTTCUcsQypNgC5jHJN8B4un4dtDyCPYphy4FAO9CrY-"
      "83tVc7zt_"
      "Pj5ZPMYDCjaFAgzmGywXCCv1boeZp7uIokrghuH7mDYuGm3B2ydSdvfpcICTbdFD4S3aSIYLWQPEvxmQ9CkuqO8kmOwU"
      "aipz2RmxjXQaC5LEV3Q3ehFZ8j4CsVlg4Yulzk2jb3J6gn2c-"
      "GpHazYuLlgAn8PKWzFxVUD7bqNuj3DNM9DhX6T2VuD3QUC8Nu3Lu9AyXlXJ8td2BW4MbAhb5TMbqIyniKtXsiOrRRPSl"
      "s7r8ClpnCZ79T0FvFeaTAOp-ioDWQ3p124fKu7wxBWejJ7lghDQV-Fd0EPhG7f0QClR2WoJLbcVkUQmyccP-"
      "8hJUuGtS9R0vCb6FGzYKtRaBzqgtSyrIdoR8KJiCNfe6KuiWKb7Pw_0tPJm09wg8QwrLEj2IuOmLHbZOP4Bhx_"
      "35HeKP73y83wEjshilZf8_IXmdCms-"
      "X31oSk5FiaSJcig0f6LahmnRtoI1tFH2WmnECgoPTpjaQXiEPk0dWQWn5x9MGubsSYciiKRKosaYZBtj_"
      "X40TsS7J8BGOkxxSEUU6NNWNTuQZoCQftcz7uGmmNV_svzI9Udf9baDUac_THPzjPQFt7-"
      "cwJZrU2ehRbNZMgU5I5j7aokcvpqPf4UJeFLTQKALJ8LD9bmnoSQ6ntIgUSwYCJOQYG1hawlBINbKQH-"
      "isKijh5yzfbP8XGLZi_rddgtL-NGTafqsHagwmHhEaBxlEKikCwMnhsXLKWdsjcTKXCpHy8c_I9Me-E2uAf_"
      "0sZbnMIvAx3stcVeQ7ZxFP6pKBoEQng8UGPNPwZAa6C1GLt4TUux0NwsIHjfHc1ovnpDQkgmMSzfHReftdBE7OemGfDL"
      "MUebWBeOK2EwS_wBUcW2AKTGsZkt-"
      "fnkNLOwwZ12DgI1iw2qWa4LGcdvPLVt5yZ2Q4WCEckM3K6GDSQLOV3CDFv2S73VHYN5ViL52ty8rtsAZZ5Kizofm5d-"
      "JzXTVlRoge2egkp4OwNfGU5Fjz5xStR2L0QtUPvSRZaY30yHr9v-OoNRDwyUEQgO7g9wtDYxJc_OZeR4bZ_"
      "h3Ffeq5JCSPA5h_rXyyRjXjHmFEu_wBnY5UY6iLE5G-6rFWsEKnX6vXLf9CGdo8ffCDBZTa5O5YoP_"
      "HlBxMuAmoEQTlSdofr3yZ4AhO_suv9gYasKQyV7NawTpf2EeqAqoypbDuss3FNGchflXKqmsuiC6o9ZGZAhS-vqpY_"
      "PJksiE_AGvEgLeNuMcoDabc4_"
      "I9lskj23hIo6Y0Cb4ZrfzcNiM8MN2ixou4uTZ4dlXu7Jd3edqm6Q7iwmaF1VQEGCFlMSiFYzWQu9kW-lU5Xqxmgo-"
      "KuWThxm5uexQPqTuuTCRbzdAWwkzu6Y0LpkoUVROuH6K8Cz90kdFSbHtZbP8yfIJzolu0AuzQPKnjtCilSIJ0eh0CTxX"
      "oK8mw99ucKy0hTN4mCiyb3mf_x0L1EWzW8IGpy5j7G-"
      "4p18lBhYzOLqz61BmkWgbamdC8GO9MxdFgM2IOB5MTEdH9alF5-0-k2Bw1cm_"
      "SfzzRiwsyveATxNTFsCFbWA8259k0UpJzCEnrQaJYWiBe68s_Dzs-x8ojQq4DkK8zaART5olf2jDzkabNrQa-"
      "EPXaqhIvcipjereJZek_fq7E1glZSbu1boiquQpkw-TjvgcuhsMhLuZb3FxzefqiZ0x30AhiFexU-"
      "Tu1j9Lybzhz2akOjLik-"
      "LniyHhun5fMSHMFkAwp9477ApY8Fr1v14G8WzaDmG8KdJmxZipyIIwQIAMruZdcWFBvvsqeN8D3pDuXSHaLMzFTal4vw"
      "IRsOSWaAKD5pzx3cgHbxvsRwZPjnwSspRqrfHUxwYc4hiFazjnvD8729JkbA_"
      "bWsrwuIcn2jcWA0nb5ZvHxyRk0b0VkD1WkS9mumKDxojrpsAYLdBudBDIXJyZbgPrMbiTDBx7vkSJq0cfyIblWujAL_"
      "2B3I1eqFJtRHW6wqorvzbjAwxx2NVxxTQqVrCznamkXYkZJ6mo2PA9EdPrx2l7AVQviCI3OJBJxVWNYth3Bp8UcLjDyr"
      "udVs6eQlrbbUIj4c7xJ5hE49YeHpTsmrZgtXYIE8_sPgVQT2Aqnf5OuJqCaqDwTQcDm-rIZwE8omFQ9XbWoVXfbep0-"
      "9qFOdPMBkK-Xry-M1tnTyn5j-h81YO1u7zDczFNJ0lNQDGGju7cmbbCQGcpOaaGKpxTJyie-vyztUQ45W1vGfETPW_"
      "kOwwMWbykHfV75-rsw4tW3lfQwhOuH-"
      "x0tL442Nz1kUMDmeekqJNblr6jPSN6ma8eGJ7kMl63wvtwmCXcHsoxaS45BWNRciLuR2CICs-_Bm_"
      "7MdODO9zKxIxAgqPazqHJKq5y0oPKzsbQRsREkUVU3CJnJmmfbPMHEzjrAFk4yjVmNKKtWzMNU0O-"
      "PWfsRPZSYnX8XtzTuVk5gT0it1UiE9fkBFD-wZuZD2QVBTEehCSV4Rkeo4gR8S4FNI43Y7FoJUKM7HxBhmtOML-pXuU-"
      "g_QjOw-JMrNyRK0RoaE9cFBkHDhoKOgjbMew1p2Wasic7SDVXULj_jt7s0OY393QWMFQHRXtaI_"
      "yPjQMulcDDOrJoleSHFtEP-Cd-6QzyA_J86JD_"
      "MVGcrLAfwQON15471Qfc2Yo6l7YI23vWAUk69fhqT7qxCZ3dmYOjE2vi3u2wvJ3GoBsuv9KAXO1LZ9RkJZk59lCkWxOa"
      "r1YiwRpiSZUrZRvKsBwi7WIHTRK1LGI4ryhg1UBlvvEBFNxOeVt4umVShAZZm91lF1UbMNebFMMnYgjrnIm7e2dfiF3U"
      "PUR_YEbOg6KSA5IhDwQoTUqEEEyMUE4Q8Bp-xjgvIsShvSAhSeFmtkGSQRB_-6-"
      "7sUpYgcJud6weyMtnB8KWJZDGfntuSzDgSHRbudXbbasrmtV3nDE0wvgcI9ywpeWa5nNKdcXkIg-"
      "hUJmfyigOUflyEmrBFbj16S_818EUDQGcz-sGcktc3gMH3hQqTfQzNxOAMn_J2K2i_dF-AzaRfasES_"
      "UE32I332wGeN6LqpT_TWF83geqwiLSWuzSFfnCbY9NL8_"
      "6b3zvs0cKRq6mA6urgcQIb1HnvXx55PDN2uKchEwKHP1HhvtK7Ssz0s4fcvzyHHqvKw16f-xLpMbWRXYBH9T-"
      "LqJoCPRJkNFGQzehGSylTRZinXKsuvY-"
      "QP5q5Tv8jPEanNBNBMViWE0fYBoGUamoudlWT2goaYtIEyLFyOYtRIFYafoPxoJox5_"
      "NpsEtcH4ZkW6IkARrUZFFQBQo-"
      "ZMRZGdE314XTLaZIfAO3PPUALSjkcWyCuBeEasVyEgzBgVAKfjFykd6LTD7Q9DBkwwWdfip8FR1m3f2Kqj4eXYiRJPKi"
      "4dbSwsxWQ0QTkhOn2e-lsAfhbn8fUBDiztt4HCulkVwlfVaaeYY9wCzHwXsG5PqHgCrqpzmRcMxP3uzI-"
      "LbYW1c7kdlA3FJMP-Pgd_Rpjs3bONkRqJTVxx12NdDoPTfss85l1IyzNfcE2fOQ73-0i4A1eSSM77rbSOtz-"
      "o6hx6Ew7YHdAcurUnl4ftUmVChZrqwrxCKnSrmdCsIxn6HFgrutRlZq3TVREXc3FDIjkaJG8catfd2v0-SsUOS4w_"
      "ELpkW3Lo3Rz8peWV2JgUGT73f2HIv2INE9mkTzvtS70UcZ-"
      "zzYoyFVhK3n2i8uwvWXIPv4oDBSAAj9CjcQyej2Z3UgoZ3CoCYKReMDZEwrzoKOgR2qhO38-"
      "piFIPRSrxclhgHdYr1UryvERJ3-"
      "Cqa0srLxVhTyTJV6m4gBiy69ZvE2Q1w75YKR7epZXwo26ycIqQUcoSSHJcK1As9N4PNbbKS8fNsReg2eO6cNSMYRPbpI"
      "OZGt9qByUWuuh8hiqWu8kjpd8DMlVI26jhWwSPcu-0joXn-xDHE3ttTfKkahMCSZ_"
      "5l0tOj3a7yDuJILgPU30WfRrX3uZr1nc9k2-EqhWlbfuN63C_8U6gO-"
      "eOsTHoSWp3XMZs9YV48WUVYVRpBJD2OkG3I2V4Hk1wPujMgOP82ilZ37BUZ8scv_elX71dp9TYO1ym8R-iQ_soM36-"
      "iq14eGj-X7tpMXBTE1Vqs9djiN6FphB9sr5mifHbKLSs37FMkF5tGUYhramPrHU8YktPba4L319rUL-"
      "Etb99pNhcXrYJ0K9jmydHXSEimLNyO7-"
      "nqLUKhj14Y0qIo4c5bnQNh6h1XDw7W4Qov1K7qeC1UzVTqbBkRgkE80mNcbJ5jRkss8_W0JxVj3Ww-"
      "CovHha6E1kPnBdlb9pf25hjD9LqNp_0PIrv4-"
      "yE0SpdvcnjhCQozx7QnyAdjJhDAFbqKt4PB4bf1tDY5EjXjTbwTuAgLhspcdkT8zrkUcKbcfLLDGLRDq8tO-"
      "kURodTI3vr0BI1JKqWKhvDnKSt2QnsKwYDi339hebUDhndGOfziLN_d7hgX8hjYqX8oAaJjT5fVJGrlsbVg-cvY_5-"
      "bvnhHO46Aer47If_e80HhIaY5RRz31eFIVylt9_DDzxzCPugqReucGJGdkKMGySdQH1nGPwlvI52hn_"
      "0x9jBkoxqykInIkO2hkc64_3d-Qt-4oRBqqyNfqjPwva8YBinhq6ebEve38a5dTzhYmgbzg90XaK7W-"
      "UflDiRC66oj5hCR9gv4VvKpZPZJbpczoeai21MKEfIV5IAFvX_"
      "WprndbpDsveiGrmbarqppADEGJPLY4O1J1fh8lgZxi6rcNShURdDkgHnI1Mx03F49OFrokR_j3TW7eLGVR1-"
      "BzdJqMhV06PdF-XPQA5DcMyd7EOnzId0OuXlHELKm2_qMGlTy6-B1yzYtdcowoIaFq-Tr-"
      "wIdWzNoR73PA3TTWGh7LJtehCLnreSKgxo_"
      "ddBGZcWBEM3pPdRJPk9Y0huGbYB0cGjN56vOAMUkcB1qZaGrwYHqGlZ9bF24WioFTPjt0a1btLu3eYwpXNgErgYfwk_"
      "a58wQ4pCQ84Qtiph_kk5aF4Yj37TOkP11t07NOchb5z1QaHmwynwL1bCvcnEysvHafpUmoUbBggslF_"
      "ythV3FSyPDZxmE2r-bUrcKDT7-1sedXDNQuiyY2Ytfru-"
      "W07eBz60qoRbvDLdVYOi6nmwvYUX8lGxVrOJWW00jenSPwt-os3_Y0Xu5rgI0QBSEePb1x_"
      "ORtkjqtWFp96v0reemoPL8u0lqQ1iYTNJZci1zQL_Wwg2hVmYfn8lxw7lrNVh2cFbLIwE3T_yI_"
      "FW04iBXdt4LxdFK6PCs61VLU182j3Z6cCzOSFeYLB19RcUWh3z046eTV6TIeWdcD6u5kNe2ocYy6UQlFXo0WLqm2APvU"
      "Rnt2oqyZYhdME5O0yEfLk0iVVqMEa1QntszWgivhjgw8foLIxh6wb36Z4AnXYUfi2kUJWRiUQw2pchT6TmutcjL21TOc"
      "AHSevon0L43C2mgqBJ9i8SdKLbkewprxcJxFgaMO9MFhXOZjx0tawpLByg8PfgjV-2GAPKTL-hpOOPuX2bIUkirWr-"
      "QIaA3L8nOOSs71xR0fnfCoAZoy2mC2B94cx0-0gpeqaa0wnCYaI_VAXhB_Y_"
      "F5SvWGnV2iHdvwhcvBiqCuIK5q3Wl7xdCUHYGY6bhB7XBBmoAVMTr-jXtl8H0LPY5LVn2sjcwtPpJcdmpflkPh77FS_"
      "ABU-uiOC8vb-cS5-JEc01OBgDmy5sxzmmxqveSbu1E1JvcD-"
      "O5ANQxTwRCRr3ZagW6e3S2m65mdruVVtZf4RvL43vt6DznylXESIS_LtNnP2bBT6DDS--5maVVuwIeT_"
      "FQNnyNQ2cCUWVQR6_fNalHXvWh1MrEvlYdmTgKy_KJV4N2F-hMYaKl_0jsT_"
      "O7hVPKxO7SuRi8qgKfh17ZQT5UN8Wy2cw6ILa2d9UYNDENl0s5fsKWunDzTveWzX1Z_"
      "EjULCcn2FPXLhz4BnJjsgq1vl2sxjsgTJlS_Q3mKL2vOESkn0LYU9YfSjLN8IWfpnRzlf3be_"
      "RundUtI9DmnUHzUOqopDHb9oHE-"
      "6o6BKcudPibB2NNvbtUpq6vzjJaVwbqpaQ5ts5Eqal9zsPpyHb6IxhnoNDs94wtmlFj1xZ5xx9IFleStc3nZ1CPiQ5Cv"
      "TGwLwZ8AJDdV_k_"
      "uPtvQNyP5Y2B92Y5OGfjRauzyrzSzLJ6LuCZNW5MRNtvSSKQq4LaLasmKxCLR3EWzHQ0tgVStyAi6W1DjWBIKzDh0jUk"
      "cYeIYLIx_ZlIbjtFKwurL8NY9gkOYPFtPhMn77LOxrWK0NMY9odWBIdtJuNIEHOVQWqcBRcieerTzwrX4pkj_slnSq9_"
      "BnpO9qY7VA9sotsI2Li0p3y9QBqeqYmspFFDdl_Z78f2UPwDr-uIH9J_"
      "1OEvIlKiTQtNhtHob33Zu4pM9rRsNDU0xgOx08EcPYjDx65EcIgjF2y5Xaes_jZ16ZIYq5SrrOORD3-"
      "xXn71lJWjAjKYKxAmwGQGKSglOzLKK1b63Uy7R2ctDTUyNf8TardXFMaOK97z8GljQBMytshGFDWkJOwhkeWPYbG3bNG"
      "8ShdVb_nGKhoqNYpxEI0fEYixVy68uPAftyI_"
      "Qt9Odvt1lVp47K6Wz7PHAXLYSRVgeBYI8cjuKPQFmfJmwVAtodRIeBxsg4mYUnTmlvxp0Pi4egELG4MNKJEOFD78hHOF"
      "kkYPplgysDE2IepIv1os7lR67h9WPH684D7Snc9rGWw_X-9VmQxcSjdf6Ud9k18V_"
      "98ZAmM35VRORWGAL0z7KmRu0YRzENp-6Ua-PrE1pPih_c19J-5wwTBjZxFswn3WlzuC8nWeRq-eQWQGRxBdPzCc_"
      "Ginif5374jQEQ3zSB8tQKRA7a8ZayUlTGbM8zzqFg6wKAylalMUm3PqPi8AtHbX6rOh7erMpzCDm69Cqrf9L7gBZyLzO"
      "dIUAnjjYXL5ta2kuvADShK7kEQqcBFBp4Co9hQz_8-EjV5IX7n7ejVizf69TWUMAOCmaSvlUV3GYz-"
      "5nKyvo66EUKHbF1JWhgDL4usG2vq82mIsBYns0HPz3anLsLmYz3N5Ek-cmwWcsXM88FBrWWRPiDdhHLpa4N-BP_"
      "916uK_SvUzIRjUTDxfldETOppH-"
      "bFlPrutjz9SVqvX0edLtKi7R6JJtStccQ20ocheWa311zPNLv2yZKqhNnSSMmtbNPXCJN2YMN5eORkf-"
      "93Hoh3sKZWFmFumrOwyiLIwcCZtFhLNEFYxQ_XexOSW_Q8Q1M3uSqJ0XfCfKWD5_Z60aUgwQVttF-n4P-"
      "71OxbneUsOwNT0yPMmOz4ppDpX0LLxg7EyQVap96FUTWRthx5DID2mdoQuv27lNHVacANfnHGvtJ_"
      "XPUX3EADJMLQVxVKrPmjDvyjMcLkpWghYtnqaNlLLCqTn8iIm43j7NGEMiRaEQx75AS5IVp8Yib97Grv_"
      "oV05YDi3PwBZDqEOr_rCQp-NWOLeG4M0YTHCrs8u_"
      "LUMCTYuP0jYUBvFQIo1tnpaybhLrrs7iBNU4iWNyDFDugDv1KBJbS22p4OmxteDBxkwRjO2zjePhyIryP7iKMiPcQ4Sv"
      "gpoBsPjnqUP8OuXREgS7gkHUVUiwiSVwZ1CU4EcbhNrHUAB_"
      "2MUBlgU57LGas3gCfCmYg0wkH6ZfSia4yHjylrKJkLhMyV029OS-FEVqisfm6knY7EKilGpKtlPQnd4EJdK3R2b-"
      "exk4eh9QkRduh3zOqnO3lQsNCSzCkLIvtLYTOlQjvQVy8VAQmSWvHkyZP06L_GIiF0D8WTYUl_"
      "hKmsaPFfoN0krKZvbFQITPVGcUDFOZ-pVaMb_yFNQEUMZrge3ghP-"
      "lEY3yqiGkIxdsG1Bg1qOdq41TSI3o3LzdCAm2Pc8Jg1iTAcnea-"
      "6k0oNmNa6kn57bllLybHkK1a28oriqs2EDokLiPNUjQmh_bpR6lUfKljZN0OfhYf_iQpuMDJRp2lXSs2CKgY_1T-"
      "cQ1jxwKghfW2wxOyZokWhS4bbe68r3eemBUTGO8OmfrQySGVXMEyvRbBcMwbREPvBXPn1LEbnj9qzSryazh-"
      "YNbWievHTrQ6oyDpMD7JtIOjtCrFm7CAx-4rwBD9RfNO1w52Lkst-DGL97GhX2umVFlasWVxxWVXZQkdHWLsxA4WXL-"
      "6Bs_RQ3_mQ4ORYgePuN2L94DGBFRwQsDlh-iK10kAmwyQH_anUS1g-QrCdu7VMTkdDRpdO_3t_"
      "JtTvsDk9sKoRh91fX32-vUSFzbPg32JTZ0Jw_jE6rQQVcweqQsDuLau78I-"
      "zH6diiRzPSgYq5V3cfkuNKwhLs3agQLkgF6gusK5cAKv7F2298UkDGw5bm1G_"
      "CU5fNfHN221r9a37AvAY0td5LivTn63gxlYVRsOqjOq3zLCj2CrMR-OD5M2uPtrPHEwZ_4rsPSLUBu_"
      "94X2RazoeflGvO1RCkb_Q_S8aMMawmS_wDt2lQ_a5fNuhIMNQ5Y_"
      "JnlBkVfXHAeZKh4xynWMMHz9AjMgvo0gHuspIt8djyPYzECTNPSEbiXTxomJVq9-"
      "rTuDE6BK7x6gy0YOqmgAcpu8fH6nnu-Vgqt04o5J8mrq-"
      "2P76RyF1VzvRPzaJvaBw5mWuHCqpPJnSjWO9i4JaBs0IXnRab-31t7kX77LNDqbRn62aXVg58fFKOjFVxBnG-"
      "53zGIUOdYW0cm6hA0gVaQVXfG2V-HvHjz1XOq0FKZGGvEUZcj48OhVZjQabapeA_"
      "MFc8ljZzQVHLYNY4rSr9AAHI2d2kZjSIp2bspntKZrQPYyXp4JwD-BIQZptwjkDqAlXl0s95ikKeScCXRkKY2xLGV6-"
      "F7V6WGMGzGCS2Z9hNn2BuW09YT47TrgupYAE7DVG5BcTmEpYm9YrXOSRAWKu7OJChnLPW9wB6V-"
      "l5mXiTJiuT7kFo2A79_wSz_GD1CJcDNmstDEaTmfnk87WjQ85sqlkHiOR7INFZwuiBzRHxa33bT-t8y4vM8_"
      "ugw3tkf4C_h_yMMSDcdapCDbIUL23sqUd4pj4_HUjG6g7nKGgAX1FaVtZtRBp_TC-"
      "mqjxUcK7koMHiH8A3QhPMfWm1NkMoOXFcvPmTJHeS7aXpvmdcobPHYkyE8PgRaO2o_011ACkB9MxruhOdjx-8Xs_"
      "LiUpDK8TjzxwR4U3vEcItlLnGI76zyTQcV5BQTxevRPnnKOZ3rLeDQPTUbRT_G_H8sumwz5r-"
      "PawchBpfAgZUVmGcIcBRji3JwJKcDAxhC_"
      "IWX9zAybOmJEt2SMDx0zlH9ezaVRRIfYmudO3mxGYKszvaEwcMmC9GXv7jg-HwPtihZRJsW-"
      "a3AxFfP7kAjLRBmPe8VmMnlKbjfVUvzgzG8Ohh9u_"
      "c9jdgSkczqTkZyegV1t3FkuA1ZuVmvnjZoehT56k8A7cpHuFS0Z7IZBjEeztOhZVat7PqHs9VQxjk7bFf8v4qZTT605S"
      "SD1thf17kOGV-wwL0076EpAncDm9NQVvfg4tR3O3qi0gFBc-Tnbuck7Idu-fleIve_jFxMhP4fwGe-kg_"
      "fzLEe5xbiAq02Z8KDs87oWXpou-"
      "5F9a3uxOOu22Ia8iJXlFyvZONuIiEO2muv6QD1ttYa0oFvlvmqkXU4L8i77BMqgzpj0eM2NeWxTnNSItB5jZgQsWy6I0"
      "p8RIthX9oxqHGRdLriehfwSMvjFSYnwcRuJA9U_XSiGld1if5X0WJJ4gsKTmqZUenBs-"
      "MMYvXi28FWd05F7tgkm2isN9EDvvnPhlNI_IMaKHJc_NB_"
      "HVWHuctT2ZuJvacxy1XNH9SrcP8RP8BFyJi54nYv6oQSNftOUUNMqxgPj4hT6oKWERoqf9EmdgUYNASSrq7CFScZvNj8"
      "c2tcAFCTwbi2TdKtKjBGAjJbd-Ilt7wTUvt9NCUxaXXdDam5QongfHoh_"
      "R6fQIp4Wq7F9BzKt4A0JE55ylWfmsI2MgCLsJQyKBbrL9Y17ts7dxFNbm6rRQWYIcTPMHweVsB6RmoDj0Cg_HwTOYNZ_"
      "zE_jxGEGWQvQVh0J89upQTBWOdiSgIeySQQMva2jHQlb3I19uwAX4kR-"
      "7q5zVmKUBCSqQHz2B7k77HGyC2aXZPbPHkESbP1kVgAqdGBnBU5aZ_YgCYvqXauJs94asBpKFt-sfOFgvx_"
      "FMMLDj5y8U69ZrqsSfrEctjTq9hHm1shTZoUwQ8qBVPJpzx4YHWX9IaWVNdEXAXeXxJIbp-"
      "hvJwQev3fngooP03vsBNpSF2zeyNb11MG7f5KLO4pe--_xN1zfgh0ltfARgFqmBnV32AiLlfdVjK_7DgkZeF_"
      "263T4dZpHQt0MwXWYHk0ixywbe54T_-EEtACplyCFmqQyt9vBg-7wvdBrxcOIKUDJDSJA4MzBr0TmRevqaMSz9gfK-"
      "ad-x5XEmijqv0ysRFPa5i-44VXB6hFlhI8rHV8gLoA1rj0-gaw4Xyc3B0Rvvv_F2NqgwNxnenRgsum-"
      "Rviupyzb9Gsw1T1bnxjwWPUP-tBzy-dSZF-US9Ze7k7Bwu9HfUvzpkwsaNUlr70m_LgjHkSkRZ_1-"
      "VZz70AvrJIqa4ed71EyGw7Zdi6OX2-PqIqP6U6V8hkxNtA_667VkEmBTsag5E0lH-"
      "ssTlhNsndvPUWvXv9frWNOnQJ8eORKYA42UKlbypugn7QzFU7yeRmLtcJyxvbCY85a7XfFwrb97R9yLRABTeWugQlKps"
      "BB6NXwC_Eh5A0wmXFXUMwIwPgsmwHqYT1xp6TRl1gFcKbEELBk8FqXHhks-ut_Zn73J31Qus3hYwJKZXFYJj_"
      "LKFWk4BX3GX3VuzTkwXpuNIx2Pzh8s_QiMZwtQmP9OC4QoM4ljX8ESloRBR5rBri7koCONI3mQdCoHnFfw-"
      "prnDiBMESksKvVKkxwuqaTVD6ptLRMAGtybHjP66m9jvDVAXhXAhbrryoYajaxgEKahcbPcXC55bMyY2-"
      "5CBGseJ1pP7KieNjTe87R_r4dU1Mlt_zTGVGd6cTAsbihogk5WHGkK9S2GXuot0Xd_QD9G-"
      "tLWOFyKydOv09OTtsKoKU_RNhL7WkGfJlylZx8e3RVPYoxq0eLV856JtRH4O8KhUj0L2-"
      "OVRuw1EuIVl9WQUWRcrg588Snm2_FfVGbNy44vABUUcrzcStdj3VjMC36YFTPjHoIFNC_"
      "g3hhU0Qr8wHTipNhflzODri9g2c_-6dn1dBx0R_"
      "hWWGLdkqt5ZhFFgSP8A1SNFtxSGxPovPZWVvGjzUk1wew19qtw3pXh7gdhZFCguJo1xRw8upWGlbtJfiZioPJP1UPeXY"
      "YX_vbve6kWdl57ijGWqa2pG_ZcY49qH-YVA88XdCJqVbDcj0q0btDUIq0ppja7KgYIkmdt54_"
      "PAnvLRV1pwb0N5ri4cPszv5x86pBOGI-Rzh9oX9JeqAXg4vOmLj__VzM7GGe8suA8uY_mj7qM1_"
      "O3sRSISmhUAr3TixaejRg8n-iqlS5NEJTC6AStUfkipG8cLNcVT09OwLjDVcXCS9Db2_2634sjVlmA60hnGYfj5mKKK_"
      "Bvcq6rUKXaWKzvpgtHD9vKNLZl_6E_DnOEr6ZdZzPKSfnsJWo5LeA8MD_EtDZ3GDfbM5TThCf4WS7ivLvEl_"
      "luku6V7UI2jLVfV0-gkCT1U-8JY302uzOCugFCv1tNzUCbXFDHD_ea8IiF4Vz12YWS1Mq0y1JcJMOi_FM0Qtm_"
      "bkji0PtM6eSryJWFRs8U65cmhjsuUsoBcFNpSze9neIzDRU8a1QALGKdl45ZRPpZ7jVMjbhhwWwy3KKV10ZWJS6ijabV"
      "9NW-HBa8WqMKPRaJnNL_"
      "hLrOtbHFSpo2HYKrbdBXNIFE8Uui8gvQMLsgQiMFbBFQgdQjdFdje0tbVgR9O1Gbs2oBxAsWp5hsr8yIohjhIoJndDxr"
      "-Iqw1x-6olxnbTsd-VzDwHasYbkStdWP-6HrG3D-Rg4WDfoVOhO8QhBLCn2B4kv-_"
      "AFQIlT1ozbtiFMbk23uPG2gwaGp7oZJWbM_5nbGb8O2_iZZv-RILMbwt4p02myZUs9mgeRUe8OJfrHE7aH6-"
      "MdZAc0VMXslQBp3iZAMKudadDsiQ9DGkTuMEf_bfgVBz2XGkJ2PfXnOf3dsFCjiUuB8pH3kz0CR37_mZQjFW-"
      "WGymUG5IYaYoj645C0uOXtBjC_XQUMhFSYFz-CT9n1XrorZMqoXpWWh8oAonT3D3OIi8wEcxB3OoIJmVc6_"
      "ptsHOHZZMFjGYHpxlpbyinSCgCcJIXEKdpCl680SjkUYn8E4KJRgWIPnlxmt7Rq1s5i6pxtCA_"
      "sLdgOa3wjvcauZgQFRqOe5enBrBTUZ5QTtjMUaX4L4WHp_stOqVhyX4J2yP01ne1eqnNUr-9hzebV98N1_"
      "4a4mVIOqegu0xS0haMDRzaOn_szAWkefTdosfDFimVKtpzaibZVSWIhU-"
      "x5LWXVDSYL1CF5y3VMvCJRLl7BYCvlCpm1okZnGYfPM-"
      "iM6Cekju7J96YLnIvfoeKTOiZbE8Gmmm5b0Q24KYyoh7LGdsRD0B7vi-"
      "lCbOwaQasVvobJw3LJgSlVUfmBB8JIMBgSfc6LhJ7yRDdTkEU5fRtT7zzrdPWMySnsdzcNJqaSYjnf4e_"
      "2MZecup8sZoB8zfDxUPPRfBpMs76S97Fyxh89AWh4A9hRfxXxH6C_o7DUXdi3N0I-"
      "Dr8RFMprmUmWs9EgN8HGinxDQly7zUfBYFqcOt--"
      "hJ0Yxc2iaiNTWExujMuRUGULSvMXRhPlvjmmk9nbVi9gEz7OPYSAtl0G5eC7g-0i0N3j-tqqoqR7gwre-H32J0vm_"
      "U5KTNZBxQi37suyGkMiNW-zT6Buox0pVUsNBMJogEulJRtA7p-H5EaglV5BtREQeyP-"
      "484PyJaWcJGNWRbnr4WI7PbuLN3BXwePv9NZY_6KQRXysv_"
      "OV49rzKeU0UrELbnqhmazDgeEFPNuOmotn0HBSmtGxflwbkm--"
      "yU7F8sxIj1jTRbMlFB9QZonKTriHXcYkWFI1h6ujrveWUBEm4g_rDdQZ_vs1MqAB4mKpTBjkkg_VCKVHQuT_"
      "NfVWxxal9SiQkfPsJbyzVi1Avf-DxM2C6-wO2jkv6QULPY1dBJM-"
      "6auHIYpi2EuzAX8wZq2JSc67YabDidVfpDq2fbfSas_mMDkejcdlhSkiaNpBuuhnn5YTSDiRif0HYkSKk2CXQx_"
      "RaJ3AtjpbECoqA_34dhpP19WeURazvb6lgxLczBBc_VvD7smoYF_S_"
      "AKljR4Wp6g5dtVU4bMImVYhvVnphSxP6hJi3vYRD3jvoq9BVJ6wFibNEHqcARVigxF2ZQmuIqV0ReQnH14VitUg5aD6x"
      "mhaKSz7l3Z59RyMcuOzYWNROdlEb6szhcIB9sWyXTEgTVWMmsN-"
      "oaQ0VsyGsaM2LJEkTdjO1CUgrtX2n2wl5GymrfCTk3zhC9kn9qXUFdoqGbLkUpmG4mAKH3y9LBGQ5D0WEMFEuJ6fW9b4"
      "E7KeQj7lhWZlGjoCqg9I6FciIhrPd92pUPxpLhKzzSnpDaBQH0Nk5yfsaE1WIFErrqzEXCioVwW4mO3ROoS_"
      "7yhlcDeXnlqXTRYG_pA4ybDcxIM1TdPCbqHd1e9eR8DsDKGrlzyhF2LdY-oGruf733Vf30O5xsGQ139yVUn2v7_"
      "cCh7TVPTdlExq0-hlA2gri2PENjpSHOBIX4W_Zhwj53ADGs7ZBh9eTZOus8a_"
      "FLRd9gPehNac9i8wFtBSOz9MM00wk1dbVxkgJpf5ERhIXivXa6VsiwCgnaT7GkVlme99XYA8gT35M1Lw_"
      "SNE4ww5MH0Ca_"
      "q3GkWKoS7sOXE0L2MQ75BEgTk8TejFBH3ZE8GxJ0hZ3qnzsN514N7aqRYHfyuYGnMMxgCoiIuZHUtgDCT3e5G6codxrR"
      "BbCE_RzmgFPaiBtE_-"
      "J0PbVmn15aBMagqlvyRIvsNylaDm3CNOLw6FXlDTOGDl3pouA5M1o9ukRekYuvr7wVcEbnwOBHfsiD_"
      "l1XpMHS72NWAq0KrJuIW8R2U6I8vr2DFKMm8mehz6cpX2xRTVxj_"
      "LeOQRlM2PCrdgtJphs6h3z8zObk4NWAUFnQfCepSo4BBUrYJbX0OisgdGkCznvwn3mkf1CSyNvEsdUtmRek2l9IOVXH1"
      "weRWrDqcvv3YNe6FzNa2GgZeXZ6zgNi9U_"
      "IaNKH29GeAec3Z2RU3cUIYTbj0zsRQsQysi8vh4fDtVHdwTMzc71xR3ra1zY-"
      "sq77N9YAzN8DDZaEnTP5TfI4vJLpkWrRgIBsuY_P9F_UydYmaMGk24_6YoC7Xyu38T4wIer80AKiC0-"
      "macUcMx5HgBxwixNrvh0uiji79bOBIlXnnnwZTAvXS1MLSiZldj4HabZWKjoM7KSe8WlxQquxmIBlzxpX5GKpbb0g5W_"
      "-jccfAi8XI4eJx2O4t2kxIcdlblDEYdL-wKm_"
      "cVOK9fgCFum2bEWBvyWm4QJWuR1ogdKRrpDsoIz0cquX9NPVZes6POMd0x2vL6Qii4nMpXS7bnwXKmXLYg7O2UhugXn5"
      "APD-ziplGYLXMeKvJi5bkLT4mOHQV_ydbT-mNsysS_N9uBgC_Dl1pE4lYfT1fdQuupHp5iCQAJ5nSUSt5lA5b5W_"
      "AfJB68xKC1NtcPkvoyNI-Kzq4H6tToojY0whhpiwNmk4LDXheINdS7LE5Mj0X1FOUZn1kmXvnXNSRp7Zra_"
      "xxH6UxbZSQA6MS_BwVSCsyS101_eETaBJ2XjHd_tWLl9eBbr6-"
      "4wqrlgILzJ3LgqOjaWUwYBrsxFd8H0uBfOFM3hdg4l2xfy4R60kFpXLE4cASxuR_"
      "eKDKAtILzhypFt3ZCM7kq1rvN0DU70iLjVgjhL11I2gF3_AKcNASfIL_3RH0UFUztJ0ahg5s5fYZtkkRDr0M3eZ5sSy-"
      "qW00gvOrqbWDdIELu6Es9kvd9Ujb3R5a1B6Nh2WaYhreP-Gevz0QC_NeoYSbz_TW69XwoUNP7pycge92r_"
      "R4OZj3meNCsLNLI7K0qhsQMiXf1VMC1PjZ3NKSeJwC40rR17ikyWrEHHe6fBhMg32KPTn_"
      "hNXXRJ97kekkwZ4NjefszPm3nUeV6Kz7z2fnCkgsNKqwcEHyoEc_RGcj8DxGg_D2kr_"
      "929lNegp1JLzL3MfUdmfxZRjUHhA587WlC0K7R_zqN8d29kI7KuCUI3Yjbq-"
      "IcoxbK9JFbwsqYCIjOIDHa4sFxbVMnnh_9uVVRQDG1DQ_zVTl2ROIl_DAPN_"
      "72d6WAcHAzV2EhkgtjMgbRvMbY9kEu59XNsxiZ1ApTPnKW69JHkTdezXnRuCi21f13Hd69FEHZxnTDCkYN_"
      "Hm6cV2vcNwcgLXuNZg2SnzFRZEi3E1f-FsKLBRfvV5tO2p3nqgB7IeZS6WzPHexhCMMtlTYcSOvx1P-"
      "oXhpt70SUGrN0Zf8_"
      "asYKwi7YKTwJTUQArAsb223he8YZUje5oTlMt0Ydl8lUUi4ioVtBdRHjydcCIKRsIPUN3VrrlIHGhhf--"
      "VN0hfG9mNKhk2Ie0EVx6oL7Xvtru-wwAIX07Xztrn7z4TX_pEsX9I45ZNQdQf20CfsjdIV-"
      "a8k4aCsxwUW3sJhAqybo8_6t-9WAGVyOFQWkRT_EV7FbWDTFBFxqGW-ROHlzB4cYYqOStrqr_bf8zDMpfbSq8Kv_"
      "YrYz6TyuHETBXH4mW_JC0kumr47ZfVcbpWSD9A3hb-qBHuB1cylqFUv7xhsHaxNXqLQOZFR0xBWR-Ch_-K7E-"
      "6mWBe7xLinwseGRLDkrti60ChOWcuVwn_Uub0dOitEacOo2XZ6x6c9QmNrN4X_"
      "tQ2CUcOy0LDAF8KQ75GmN9NKcVmjg4c6rPl6J3PadjOUTdtCeBv47aqEPeWYB8ZgggT6uz8zI0jLieInCXYqg5lPiKRk"
      "naDBVcUbvop7S3WhuNVnockdU3FJCYzc8qBzW7ZQcgPx30g-hEf50EK7Q0mJm-zy72t7l-"
      "6FfCTKgWr6YkaCwcxvjohdNnjHyKjx8ZW2XaGRSso0MaWH0xDA0i9xvwLVYDC4YJ0MrWg_"
      "V8hopdJJpBG0A4AHfGe3X0Tu4uUks18HhTRNy_m71stPq86H_XJ5eD8RqOVLhxbUEDvVf-"
      "UiFJzQcV3OMfdQkGyDWLmf33UBRD-kWrCu0WbIk0LTA4ENLpdypQ9xJRVtlGrfuyVBZ9XZGwKNzjT8lIGuEmSS7_"
      "REEIw4sDKFYojfzQ8T5ICqJWN-ZV-81rr723madoLLt_KUy5srP5EsXJ_"
      "iDXSajqJwE1U6g9jCSicXPAhvKRrdgZ2x8M9Ptbf3qhXTDyRp3hmELCeNCEK0c3nGxmkL3bE_D-sul9OcCtJWA_"
      "Fq8ZqDWZ5KZ-DdS_EdUNPCrDsd-"
      "3z0IARmxTCMNQMsaL5SEITWa3wzmmqPp3JAe86RwGIsgdI3JJ3cQPqWjy3WfPA3P4p7L99Ufg06CjrozckNPZBtFBSoX"
      "gAgvor4fN7PdWOlph1U9IZasYYeDS8Xu-JLET01sjvJCBMAAZUrU5I8NPt-"
      "InqpfuwzmuSBhxsxGLA9smjbpP80dcHXfcmbIwBYMGOOGmZHJS3.3XWe3Lv6LTKplJagfNaxe1-"
      "YXTnQygbniM24tVUgZEw");
  std::string const certificateName("certBackup");
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  /* auto params = CertificateCreateParameters();
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
  */
  BackupCertificateResult certificateBackup;
  certificateBackup.Certificate
      = std::vector<uint8_t>(rawcertificate.begin(), rawcertificate.end());

  auto response = client.RestoreCertificateBackup(certificateBackup);

  auto certificate = response.Value;

  EXPECT_EQ(certificate.Name(), certificateName);
  EXPECT_EQ(certificate.Policy.ValidityInMonths.Value(), 12);
  EXPECT_EQ(certificate.Policy.IssuerName.Value(), "Self");
}
