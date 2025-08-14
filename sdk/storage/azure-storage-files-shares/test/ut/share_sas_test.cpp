// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_client_test.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/files/shares/share_sas_builder.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  class ShareSasTest : public FileShareClientTest {
  public:
    template <class T>
    T GetSasAuthenticatedClient(const T& shareClient, const std::string& sasToken)
    {
      auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
      options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;
      T fileClient1(
          AppendQueryParameters(Azure::Core::Url(shareClient.GetUrl()), sasToken), options);
      return fileClient1;
    }
    void VerifyShareSasRead(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.GetProperties());
    }

    void VerifyShareSasNonRead(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_THROW(fileClient1.GetProperties(), StorageException);
    }

    void VerifyShareSasWrite(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1));
    }

    void VerifyShareSasDelete(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.Delete());
      fileClient.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1);
    }

    void VerifyShareSasCreate(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      fileClient.DeleteIfExists();
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.Create(1));
    }

    void VerifyShareSasList(
        const Files::Shares::ShareDirectoryClient& directoryClient,
        const std::string& sasToken)
    {
      auto directoryClient1 = GetSasAuthenticatedClient(directoryClient, sasToken);
      EXPECT_NO_THROW(directoryClient1.ListFilesAndDirectories());
    }
  };

  TEST_F(ShareSasTest, AccountSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string directoryName = RandomString();
    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
    directoryClient.Create();
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    auto allPermissions = Sas::AccountSasPermissions::Read | Sas::AccountSasPermissions::Write
        | Sas::AccountSasPermissions::Delete | Sas::AccountSasPermissions::List
        | Sas::AccountSasPermissions::Add | Sas::AccountSasPermissions::Create;

    for (auto permissions : {
             allPermissions,
             Sas::AccountSasPermissions::Read,
             Sas::AccountSasPermissions::Write,
             Sas::AccountSasPermissions::Delete,
             Sas::AccountSasPermissions::List,
             Sas::AccountSasPermissions::Create,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::AccountSasPermissions::Read) == Sas::AccountSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        VerifyShareSasList(directoryClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        VerifyShareSasCreate(fileClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, ShareServiceSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();
    std::string directoryName = RandomString();

    Sas::ShareSasBuilder shareSasBuilder;
    shareSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    shareSasBuilder.StartsOn = sasStartsOn;
    shareSasBuilder.ExpiresOn = sasExpiresOn;
    shareSasBuilder.ShareName = m_shareName;
    shareSasBuilder.Resource = Sas::ShareSasResource::Share;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto shareClient = *m_shareClient;
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
    directoryClient.Create();
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    for (auto permissions :
         {Sas::ShareSasPermissions::All,
          Sas::ShareSasPermissions::Read,
          Sas::ShareSasPermissions::Write,
          Sas::ShareSasPermissions::Delete,
          Sas::ShareSasPermissions::List,
          Sas::ShareSasPermissions::Create})
    {
      shareSasBuilder.SetPermissions(permissions);
      auto sasToken = shareSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::ShareSasPermissions::Read) == Sas::ShareSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Write) == Sas::ShareSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Delete) == Sas::ShareSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::List) == Sas::ShareSasPermissions::List)
      {
        VerifyShareSasList(directoryClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Create) == Sas::ShareSasPermissions::Create)
      {
        VerifyShareSasCreate(fileClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, FileServiceSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    for (auto permissions :
         {Sas::ShareFileSasPermissions::All,
          Sas::ShareFileSasPermissions::Read,
          Sas::ShareFileSasPermissions::Write,
          Sas::ShareFileSasPermissions::Delete,
          Sas::ShareFileSasPermissions::Create})
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::ShareFileSasPermissions::Read) == Sas::ShareFileSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Write)
          == Sas::ShareFileSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Delete)
          == Sas::ShareFileSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Create)
          == Sas::ShareFileSasPermissions::Create)
      {
        VerifyShareSasCreate(fileClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, AccountSasExpired_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiredOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);

    accountSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, ServiceSasExpired_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiredOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);

    fileSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, AccountSasWithoutStarttime_LIVEONLY_)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, ServiceSasWithoutStarttime_LIVEONLY_)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, AccountSasWithIP_LIVEONLY_)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);

    accountSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, ServiceSasWithIP_LIVEONLY_)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);

    fileSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, SasWithIdentifier_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Files::Shares::Models::SignedIdentifier identifier;
    identifier.Id = RandomString(64);
    identifier.Policy.StartsOn = sasStartsOn;
    identifier.Policy.ExpiresOn = sasExpiresOn;
    identifier.Policy.Permission = "r";
    shareClient.SetAccessPolicy({identifier});

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(static_cast<Sas::ShareSasPermissions>(0));
    fileSasBuilder.Identifier = identifier.Id;

    TestSleep(std::chrono::seconds(30));

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, FileSasResponseHeadersOverride_LIVEONLY_)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::All);
    fileSasBuilder.ContentType = "application/x-binary";
    fileSasBuilder.ContentLanguage = "en-US";
    fileSasBuilder.ContentDisposition = "attachment";
    fileSasBuilder.CacheControl = "no-cache";
    fileSasBuilder.ContentEncoding = "identify";
    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

    auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
    auto properties = fileClient1.GetProperties();
    EXPECT_EQ(properties.Value.HttpHeaders.ContentType, fileSasBuilder.ContentType);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentLanguage, fileSasBuilder.ContentLanguage);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentDisposition, fileSasBuilder.ContentDisposition);
    EXPECT_EQ(properties.Value.HttpHeaders.CacheControl, fileSasBuilder.CacheControl);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentEncoding, fileSasBuilder.ContentEncoding);
  }

  TEST_F(ShareSasTest, AccountSasAuthorizationErrorDetail_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::Service;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    auto unauthorizedFileClient = GetSasAuthenticatedClient(fileClient, sasToken);
    try
    {
      unauthorizedFileClient.Download();
    }
    catch (StorageException& e)
    {
      EXPECT_EQ("AuthorizationResourceTypeMismatch", e.ErrorCode);
      EXPECT_TRUE(e.AdditionalInformation.count("ExtendedErrorDetail") != 0);
    }
  }

  TEST(SasStringToSignTest, GenerateStringToSign)
  {
    std::string accountName = "testAccountName";
    std::string accountKey = "dGVzdEFjY291bnRLZXk=";
    std::string shareUrl = "https://testAccountName.file.core.windows.net/container/blob";
    auto keyCredential = std::make_shared<StorageSharedKeyCredential>(accountName, accountKey);
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    // Share Sas
    {
      Sas::ShareSasBuilder shareSasBuilder;
      shareSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      shareSasBuilder.StartsOn = sasStartsOn;
      shareSasBuilder.ExpiresOn = sasExpiresOn;
      shareSasBuilder.ShareName = "share";
      shareSasBuilder.FilePath = "file";
      shareSasBuilder.Resource = Sas::ShareSasResource::File;
      shareSasBuilder.SetPermissions(Sas::ShareSasPermissions::Read);
      auto sasToken = shareSasBuilder.GenerateSasToken(*keyCredential);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(shareUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign = shareSasBuilder.GenerateSasStringToSign(*keyCredential);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
    }
  }

  TEST(SasStringToSignTest, UserDelegationSasGenerateStringToSign)
  {
    std::string accountName = "testAccountName";
    std::string accountKey = "dGVzdEFjY291bnRLZXk=";
    std::string fileUrl = "https://testAccountName.file.core.windows.net/share/file";
    auto keyCredential = std::make_shared<StorageSharedKeyCredential>(accountName, accountKey);
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    // File User Delegation Sas
    {
      Files::Shares::Models::UserDelegationKey userDelegationKey;
      userDelegationKey.SignedObjectId = "testSignedObjectId";
      userDelegationKey.SignedTenantId = "testSignedTenantId";
      userDelegationKey.SignedStartsOn = sasStartsOn;
      userDelegationKey.SignedExpiresOn = sasExpiresOn;
      userDelegationKey.SignedService = "f";
      userDelegationKey.SignedVersion = "2020-08-04";
      userDelegationKey.Value = accountKey;

      Sas::ShareSasBuilder shareSasBuilder;
      shareSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      shareSasBuilder.StartsOn = sasStartsOn;
      shareSasBuilder.ExpiresOn = sasExpiresOn;
      shareSasBuilder.ShareName = "share";
      shareSasBuilder.FilePath = "file";
      shareSasBuilder.Resource = Sas::ShareSasResource::File;
      shareSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);
      auto sasToken = shareSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(fileUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign = shareSasBuilder.GenerateSasStringToSign(userDelegationKey, accountName);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
    }
  }

  TEST_F(ShareSasTest, ShareUserDelegationSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();
    std::string directoryName = RandomString();

    Sas::ShareSasBuilder shareSasBuilder;
    shareSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    shareSasBuilder.StartsOn = sasStartsOn;
    shareSasBuilder.ExpiresOn = sasExpiresOn;
    shareSasBuilder.ShareName = m_shareName;
    shareSasBuilder.Resource = Sas::ShareSasResource::Share;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::Shares::Models::UserDelegationKey userDelegationKey;
    {
      auto shareServiceClient = Files::Shares::ShareServiceClient(
          m_shareServiceClient->GetUrl(),
          GetTestCredential(),
          InitStorageClientOptions<Files::Shares::ShareClientOptions>());
      userDelegationKey = shareServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    auto shareClient = *m_shareClient;
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
    directoryClient.Create();
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    for (auto permissions :
         {Sas::ShareSasPermissions::All,
          Sas::ShareSasPermissions::Read,
          Sas::ShareSasPermissions::Write,
          Sas::ShareSasPermissions::Delete,
          Sas::ShareSasPermissions::List})
    {
      shareSasBuilder.SetPermissions(permissions);
      auto sasToken = shareSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::ShareSasPermissions::Read) == Sas::ShareSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Write) == Sas::ShareSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
        VerifyShareSasCreate(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Delete) == Sas::ShareSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::List) == Sas::ShareSasPermissions::List)
      {
        VerifyShareSasList(directoryClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, FileUserDelegationSasPermissions_DISABLED)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::Shares::Models::UserDelegationKey userDelegationKey;
    {
      auto shareServiceClient = Files::Shares::ShareServiceClient(
          m_shareServiceClient->GetUrl(),
          GetTestCredential(),
          InitStorageClientOptions<Files::Shares::ShareClientOptions>());
      userDelegationKey = shareServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    for (auto permissions :
         {Sas::ShareFileSasPermissions::All,
          Sas::ShareFileSasPermissions::Read,
          Sas::ShareFileSasPermissions::Write,
          Sas::ShareFileSasPermissions::Delete})
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::ShareFileSasPermissions::Read) == Sas::ShareFileSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Write)
          == Sas::ShareFileSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
        VerifyShareSasCreate(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Delete)
          == Sas::ShareFileSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
    }
  }

  std::string getObjectIdFromTokenCredential(
      const std::shared_ptr<const Azure::Core::Credentials::TokenCredential>& tokenCredential)
  {
    Azure::Core::Credentials::TokenRequestContext requestContext;
    requestContext.Scopes = {Storage::_internal::StorageScope};
    auto accessToken = tokenCredential->GetToken(requestContext, Azure::Core::Context());

    std::istringstream iss(accessToken.Token);
    std::string header, payload, signature;
    getline(iss, header, '.');
    getline(iss, payload, '.');
    getline(iss, signature, '.');

    size_t padding = payload.length() % 4;
    if (padding > 0)
    {
      payload.append(4 - padding, '=');
    }

    auto decodedPayload = Azure::Core::Convert::Base64Decode(payload);
    auto json = Core::Json::_internal::json::parse(decodedPayload.begin(), decodedPayload.end());
    if (json.contains("oid"))
    {
      return json["oid"].get<std::string>();
    }
    return {};
  }

  TEST_F(ShareSasTest, PrincipalBoundDelegationSas_DISABLED)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto tokenCredential = GetTestCredential();
    auto delegatedUserObjectId = getObjectIdFromTokenCredential(tokenCredential);

    Files::Shares::Models::UserDelegationKey userDelegationKey;
    {
      auto shareServiceClient = Files::Shares::ShareServiceClient(
          m_shareServiceClient->GetUrl(),
          GetTestCredential(),
          InitStorageClientOptions<Files::Shares::ShareClientOptions>());
      userDelegationKey = shareServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.DelegatedUserObjectId = delegatedUserObjectId;

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::All);
    auto sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);

    Files::Shares::ShareFileClient fileClient1(
        AppendQueryParameters(Azure::Core::Url(fileClient.GetUrl()), sasToken),
        GetTestCredential(),
        InitStorageClientOptions<Files::Shares::ShareClientOptions>());
    EXPECT_NO_THROW(fileClient1.GetProperties());

    fileSasBuilder.DelegatedUserObjectId = "invalidObjectId";
    sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);
    Files::Shares::ShareFileClient fileClient2(
        AppendQueryParameters(Azure::Core::Url(fileClient.GetUrl()), sasToken),
        GetTestCredential(),
        InitStorageClientOptions<Files::Shares::ShareClientOptions>());
    EXPECT_THROW(fileClient2.GetProperties(), StorageException);
  }
}}} // namespace Azure::Storage::Test
