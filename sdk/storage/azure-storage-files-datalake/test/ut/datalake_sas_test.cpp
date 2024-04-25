// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "datalake_file_system_client_test.hpp"

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs/blob_sas_builder.hpp>
#include <azure/storage/files/datalake/datalake_sas_builder.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  class DataLakeSasTest : public DataLakeFileSystemClientTest {
  public:
    template <class T> T GetSasAuthenticatedClient(const T& pathClient, const std::string& sasToken)
    {
      T pathClient1(
          AppendQueryParameters(
              Azure::Core::Url(Files::DataLake::_detail::GetDfsUrlFromUrl(pathClient.GetUrl())),
              sasToken),
          InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
      return pathClient1;
    }
    void VerifyDataLakeSasRead(
        const Files::DataLake::DataLakePathClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.GetProperties());
    }

    void VerifyDataLakeSasNonRead(
        const Files::DataLake::DataLakePathClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_THROW(pathClient1.GetProperties(), StorageException);
    }

    void VerifyDataLakeSasWrite(
        const Files::DataLake::DataLakeFileClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1));
    }

    void VerifyDataLakeSasDelete(
        const Files::DataLake::DataLakeFileClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.Delete());
      pathClient.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1);
    }

    void VerifyDataLakeSasDelete(
        const Files::DataLake::DataLakeDirectoryClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.DeleteRecursive());
      pathClient.Create();
    }

    void VerifyDataLakeSasCreate(
        const Files::DataLake::DataLakeFileClient& pathClient,
        const std::string& sasToken)
    {
      pathClient.DeleteIfExists();
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.Create());
    }

    void VerifyDataLakeSasCreate(
        const Files::DataLake::DataLakeDirectoryClient& pathClient,
        const std::string& sasToken)
    {
      pathClient.DeleteRecursiveIfExists();
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.Create());
    }

    void VerifyDataLakeSasList(
        const Files::DataLake::DataLakeDirectoryClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.ListPaths(true));
    }

    void VerifyDataLakeSasMove(
        const Files::DataLake::DataLakeDirectoryClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      std::string fileName = RandomString();
      std::string newFilename = RandomString();
      auto fileClient = pathClient.GetFileClient(fileName);
      auto newFileClient = pathClient.GetFileClient(newFilename);
      fileClient.Create();
      EXPECT_NO_THROW(pathClient1.RenameFile(fileName, newFilename));
    }

    void VerifyDataLakeSasExecute(
        const Files::DataLake::DataLakeFileClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      EXPECT_NO_THROW(pathClient1.GetAccessControlList());
    }

    void VerifyDataLakeSasManageAccessControl(
        const Files::DataLake::DataLakePathClient& pathClient,
        const std::string& sasToken)
    {
      auto pathClient1 = GetSasAuthenticatedClient(pathClient, sasToken);
      auto acls = pathClient.GetAccessControlList().Value.Acls;
      EXPECT_NO_THROW(pathClient1.SetAccessControlList(acls));
    }
  };

  TEST_F(DataLakeSasTest, AccountSasPermissions)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string directoryName = RandomString();
    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeDirectoryClient = dataLakeFileSystemClient.GetDirectoryClient(directoryName);
    dataLakeDirectoryClient.Create();
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    auto allPermissions = Sas::AccountSasPermissions::Read | Sas::AccountSasPermissions::Write
        | Sas::AccountSasPermissions::Delete | Sas::AccountSasPermissions::List
        | Sas::AccountSasPermissions::Add | Sas::AccountSasPermissions::Create;

    for (auto permissions : {
             allPermissions,
             Sas::AccountSasPermissions::Read,
             Sas::AccountSasPermissions::Write,
             Sas::AccountSasPermissions::Delete,
             Sas::AccountSasPermissions::List,
             Sas::AccountSasPermissions::Add,
             Sas::AccountSasPermissions::Create,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::AccountSasPermissions::Read) == Sas::AccountSasPermissions::Read)
      {
        VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        VerifyDataLakeSasWrite(dataLakeFileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        VerifyDataLakeSasDelete(dataLakeFileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        VerifyDataLakeSasList(dataLakeDirectoryClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Add) == Sas::AccountSasPermissions::Add)
      {
      /*
       * Add test for append block when DataLake supports append blobs.
       */      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        VerifyDataLakeSasCreate(dataLakeFileClient, sasToken);
      }
    }
  }

  TEST_F(DataLakeSasTest, ServiceFileSystemSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::DataLake::Models::UserDelegationKey userDelegationKey;
    {
      auto dataLakeServiceClient = Files::DataLake::DataLakeServiceClient(
          Files::DataLake::_detail::GetDfsUrlFromUrl(m_dataLakeServiceClient->GetUrl()),
          std::make_shared<Azure::Identity::ClientSecretCredential>(
              AadTenantId(), AadClientId(), AadClientSecret(), GetTokenCredentialOptions()),
          InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
      userDelegationKey = dataLakeServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    std::string directoryName = RandomString();
    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeDirectoryClient = dataLakeFileSystemClient.GetDirectoryClient(directoryName);
    dataLakeDirectoryClient.Create();
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSystemSasBuilder;
    fileSystemSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSystemSasBuilder.StartsOn = sasStartsOn;
    fileSystemSasBuilder.ExpiresOn = sasExpiresOn;
    fileSystemSasBuilder.FileSystemName = m_fileSystemName;
    fileSystemSasBuilder.Resource = Sas::DataLakeSasResource::FileSystem;

    for (auto permissions : {
             Sas::DataLakeSasPermissions::All,
             Sas::DataLakeSasPermissions::Read,
             Sas::DataLakeSasPermissions::Write,
             Sas::DataLakeSasPermissions::Delete,
             Sas::DataLakeSasPermissions::List,
             Sas::DataLakeSasPermissions::Add,
             Sas::DataLakeSasPermissions::Create,
             Sas::DataLakeSasPermissions::Move,
             Sas::DataLakeSasPermissions::Execute,
             Sas::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      fileSystemSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSystemSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = fileSystemSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      if ((permissions & Sas::DataLakeSasPermissions::Read) == Sas::DataLakeSasPermissions::Read)
      {
        VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
        VerifyDataLakeSasRead(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Write) == Sas::DataLakeSasPermissions::Write)
      {
        VerifyDataLakeSasWrite(dataLakeFileClient, sasToken);
        VerifyDataLakeSasWrite(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Delete)
          == Sas::DataLakeSasPermissions::Delete)
      {
        VerifyDataLakeSasDelete(dataLakeFileClient, sasToken);
        VerifyDataLakeSasDelete(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::List) == Sas::DataLakeSasPermissions::List)
      {
        VerifyDataLakeSasList(dataLakeDirectoryClient, sasToken);
        VerifyDataLakeSasList(dataLakeDirectoryClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Add) == Sas::DataLakeSasPermissions::Add)
      {
      /*
       * Add test for append block when DataLake supports append blobs.
       */      }
      if ((permissions & Sas::DataLakeSasPermissions::Create)
          == Sas::DataLakeSasPermissions::Create)
      {
        VerifyDataLakeSasCreate(dataLakeFileClient, sasToken);
        VerifyDataLakeSasCreate(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Move) == Sas::DataLakeSasPermissions::Move)
      {
        VerifyDataLakeSasMove(dataLakeDirectoryClient, sasToken);
        VerifyDataLakeSasMove(dataLakeDirectoryClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::ManageAccessControl)
          == Sas::DataLakeSasPermissions::ManageAccessControl)
      {
        VerifyDataLakeSasManageAccessControl(dataLakeDirectoryClient, sasToken);
        VerifyDataLakeSasManageAccessControl(dataLakeDirectoryClient, sasToken2);
      }
    }
  }

  TEST_F(DataLakeSasTest, ServiceFileSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::DataLake::Models::UserDelegationKey userDelegationKey;
    {
      auto dataLakeServiceClient = Files::DataLake::DataLakeServiceClient(
          Files::DataLake::_detail::GetDfsUrlFromUrl(m_dataLakeServiceClient->GetUrl()),
          std::make_shared<Azure::Identity::ClientSecretCredential>(
              AadTenantId(), AadClientId(), AadClientSecret(), GetTokenCredentialOptions()),
          InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
      userDelegationKey = dataLakeServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;

    for (auto permissions : {
             Sas::DataLakeSasPermissions::All,
             Sas::DataLakeSasPermissions::Read,
             Sas::DataLakeSasPermissions::Write,
             Sas::DataLakeSasPermissions::Delete,
             Sas::DataLakeSasPermissions::Add,
             Sas::DataLakeSasPermissions::Create,
             Sas::DataLakeSasPermissions::Execute,
             Sas::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      if ((permissions & Sas::DataLakeSasPermissions::Read) == Sas::DataLakeSasPermissions::Read)
      {
        VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
        VerifyDataLakeSasRead(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Write) == Sas::DataLakeSasPermissions::Write)
      {
        VerifyDataLakeSasWrite(dataLakeFileClient, sasToken);
        VerifyDataLakeSasWrite(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Delete)
          == Sas::DataLakeSasPermissions::Delete)
      {
        VerifyDataLakeSasDelete(dataLakeFileClient, sasToken);
        VerifyDataLakeSasDelete(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Add) == Sas::DataLakeSasPermissions::Add)
      {
      /*
       * Add test for append block when DataLake supports append blobs.
       */      }
      if ((permissions & Sas::DataLakeSasPermissions::Create)
          == Sas::DataLakeSasPermissions::Create)
      {
        VerifyDataLakeSasCreate(dataLakeFileClient, sasToken);
        VerifyDataLakeSasCreate(dataLakeFileClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::ManageAccessControl)
          == Sas::DataLakeSasPermissions::ManageAccessControl)
      {
        VerifyDataLakeSasManageAccessControl(dataLakeFileClient, sasToken);
        VerifyDataLakeSasManageAccessControl(dataLakeFileClient, sasToken2);
      }
    }
  }

  TEST_F(DataLakeSasTest, ServiceDirectorySasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::DataLake::Models::UserDelegationKey userDelegationKey;
    {
      auto dataLakeServiceClient = Files::DataLake::DataLakeServiceClient(
          Files::DataLake::_detail::GetDfsUrlFromUrl(m_dataLakeServiceClient->GetUrl()),
          std::make_shared<Azure::Identity::ClientSecretCredential>(
              AadTenantId(), AadClientId(), AadClientSecret(), GetTokenCredentialOptions()),
          InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
      userDelegationKey = dataLakeServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    std::string directoryName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeDirectoryClient = dataLakeFileSystemClient.GetDirectoryClient(directoryName);
    dataLakeDirectoryClient.Create();

    Sas::DataLakeSasBuilder directorySasBuilder;
    directorySasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    directorySasBuilder.StartsOn = sasStartsOn;
    directorySasBuilder.ExpiresOn = sasExpiresOn;
    directorySasBuilder.FileSystemName = m_fileSystemName;
    directorySasBuilder.Path = directoryName;
    directorySasBuilder.IsDirectory = true;
    directorySasBuilder.DirectoryDepth = 1;
    directorySasBuilder.Resource = Sas::DataLakeSasResource::Directory;

    for (auto permissions : {
             Sas::DataLakeSasPermissions::All,
             Sas::DataLakeSasPermissions::Read,
             Sas::DataLakeSasPermissions::Delete,
             Sas::DataLakeSasPermissions::List,
             Sas::DataLakeSasPermissions::Add,
             Sas::DataLakeSasPermissions::Create,
             Sas::DataLakeSasPermissions::Execute,
             Sas::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      directorySasBuilder.SetPermissions(permissions);
      auto sasToken2 = directorySasBuilder.GenerateSasToken(userDelegationKey, accountName);
      if ((permissions & Sas::DataLakeSasPermissions::Read) == Sas::DataLakeSasPermissions::Read)
      {
        VerifyDataLakeSasRead(dataLakeDirectoryClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Delete)
          == Sas::DataLakeSasPermissions::Delete)
      {
        VerifyDataLakeSasDelete(dataLakeDirectoryClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::List) == Sas::DataLakeSasPermissions::List)
      {
        VerifyDataLakeSasList(dataLakeDirectoryClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Create)
          == Sas::DataLakeSasPermissions::Create)
      {
        VerifyDataLakeSasCreate(dataLakeDirectoryClient, sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::ManageAccessControl)
          == Sas::DataLakeSasPermissions::ManageAccessControl)
      {
        VerifyDataLakeSasManageAccessControl(dataLakeDirectoryClient, sasToken2);
      }
    }
  }

  TEST_F(DataLakeSasTest, AccountSasExpired)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiredOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasNonRead(dataLakeFileClient, sasToken);

    accountSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, ServiceSasExpired)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiredOn;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasNonRead(dataLakeFileClient, sasToken);

    fileSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, AccountSasWithoutStarttime)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, ServiceSasWithoutStartTime)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, AccountSasWithIP)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);

    accountSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasNonRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, ServiceSasWithIP)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);

    fileSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyDataLakeSasNonRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, FileSasWithPreauthorizedAgentObjectId_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::DataLake::Models::UserDelegationKey userDelegationKey;
    {
      auto dataLakeServiceClient = Files::DataLake::DataLakeServiceClient(
          Files::DataLake::_detail::GetDfsUrlFromUrl(m_dataLakeServiceClient->GetUrl()),
          std::make_shared<Azure::Identity::ClientSecretCredential>(
              AadTenantId(), AadClientId(), AadClientSecret(), GetTokenCredentialOptions()),
          InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
      userDelegationKey = dataLakeServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);
    fileSasBuilder.PreauthorizedAgentObjectId = RandomUUID();
    fileSasBuilder.CorrelationId = RandomUUID();
    auto sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);
    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, FileSasWithIdentifier)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Files::DataLake::SetFileSystemAccessPolicyOptions options;
    options.AccessType = Files::DataLake::Models::PublicAccessType::None;
    Files::DataLake::Models::SignedIdentifier identifier;
    identifier.Id = RandomString(64);
    identifier.StartsOn = sasStartsOn;
    identifier.ExpiresOn = sasExpiresOn;
    identifier.Permissions = "r";
    options.SignedIdentifiers.emplace_back(identifier);
    dataLakeFileSystemClient.SetAccessPolicy(options);

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;
    fileSasBuilder.SetPermissions(static_cast<Sas::DataLakeFileSystemSasPermissions>(0));
    fileSasBuilder.Identifier = identifier.Id;

    TestSleep(std::chrono::seconds(30));

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

    VerifyDataLakeSasRead(dataLakeFileClient, sasToken);
  }

  TEST_F(DataLakeSasTest, FileSasResponseHeadersOverride)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);
    fileSasBuilder.ContentType = "application/x-binary";
    fileSasBuilder.ContentLanguage = "en-US";
    fileSasBuilder.ContentDisposition = "attachment";
    fileSasBuilder.CacheControl = "no-cache";
    fileSasBuilder.ContentEncoding = "identify";
    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

    auto fileClient1 = GetSasAuthenticatedClient(dataLakeFileClient, sasToken);
    auto properties = fileClient1.GetProperties();
    EXPECT_EQ(properties.Value.HttpHeaders.ContentType, fileSasBuilder.ContentType);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentLanguage, fileSasBuilder.ContentLanguage);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentDisposition, fileSasBuilder.ContentDisposition);
    EXPECT_EQ(properties.Value.HttpHeaders.CacheControl, fileSasBuilder.CacheControl);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentEncoding, fileSasBuilder.ContentEncoding);
  }

  TEST_F(DataLakeSasTest, AccountSasEncryptionScope)
  {
    const std::string encryptionScope = GetTestEncryptionScope();

    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(
        Sas::AccountSasPermissions::Read | Sas::AccountSasPermissions::Create);
    accountSasBuilder.EncryptionScope = encryptionScope;

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    auto fileSystemClient = GetSasAuthenticatedClient(dataLakeFileSystemClient, sasToken);
    auto fileClient1 = fileSystemClient.GetFileClient(RandomString());
    fileClient1.Create();
    auto properties = fileClient1.GetProperties().Value;

    ASSERT_TRUE(properties.EncryptionScope.HasValue());
    EXPECT_EQ(properties.EncryptionScope.Value(), encryptionScope);
  }

  TEST_F(DataLakeSasTest, ServiceSasEncryptionScope)
  {
    const std::string encryptionScope = GetTestEncryptionScope();

    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    Sas::DataLakeSasBuilder fileSystemSasBuilder;
    fileSystemSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSystemSasBuilder.StartsOn = sasStartsOn;
    fileSystemSasBuilder.ExpiresOn = sasExpiresOn;
    fileSystemSasBuilder.FileSystemName = m_fileSystemName;
    fileSystemSasBuilder.Resource = Sas::DataLakeSasResource::FileSystem;
    fileSystemSasBuilder.SetPermissions(Sas::DataLakeFileSystemSasPermissions::All);
    fileSystemSasBuilder.EncryptionScope = encryptionScope;

    auto sasToken = fileSystemSasBuilder.GenerateSasToken(*keyCredential);
    auto fileSystemClient = GetSasAuthenticatedClient(dataLakeFileSystemClient, sasToken);
    auto fileClient1 = fileSystemClient.GetFileClient(RandomString());
    fileClient1.Create();
    auto properties = fileClient1.GetProperties().Value;

    ASSERT_TRUE(properties.EncryptionScope.HasValue());
    EXPECT_EQ(properties.EncryptionScope.Value(), encryptionScope);
  }
}}} // namespace Azure::Storage::Test
