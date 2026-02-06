// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "datalake_file_system_client_test.hpp"

#include <azure/core/internal/json/json.hpp>
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

  TEST_F(DataLakeSasTest, AccountSasPermissions_LIVEONLY_)
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

    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn).Value;

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

    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn).Value;

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

    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn).Value;

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

  TEST_F(DataLakeSasTest, AccountSasExpired_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, ServiceSasExpired_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, AccountSasWithoutStarttime_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, ServiceSasWithoutStartTime_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, AccountSasWithIP_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, ServiceSasWithIP_LIVEONLY_)
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

    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn).Value;

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

  TEST_F(DataLakeSasTest, FileSasWithIdentifier_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, FileSasResponseHeadersOverride_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, AccountSasEncryptionScope_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, ServiceSasEncryptionScope_LIVEONLY_)
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

  TEST_F(DataLakeSasTest, AccountSasAuthorizationErrorDetail_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::Service;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;

    std::string directoryName = RandomString();
    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeDirectoryClient = dataLakeFileSystemClient.GetDirectoryClient(directoryName);
    dataLakeDirectoryClient.Create();
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    auto unauthorizedFileClient = GetSasAuthenticatedClient(dataLakeFileClient, sasToken);
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
    std::string blobUrl = "https://testAccountName.blob.core.windows.net/container/blob";
    auto keyCredential = std::make_shared<StorageSharedKeyCredential>(accountName, accountKey);
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    // Datalake Sas
    {
      Sas::DataLakeSasBuilder datalakeSasBuilder;
      datalakeSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      datalakeSasBuilder.StartsOn = sasStartsOn;
      datalakeSasBuilder.ExpiresOn = sasExpiresOn;
      datalakeSasBuilder.FileSystemName = "filesystem";
      datalakeSasBuilder.Path = "path";
      datalakeSasBuilder.Resource = Sas::DataLakeSasResource::File;
      datalakeSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::Read);
      auto sasToken = datalakeSasBuilder.GenerateSasToken(*keyCredential);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(blobUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign = datalakeSasBuilder.GenerateSasStringToSign(*keyCredential);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
    }

    // Datalake User Delegation Sas
    {
      Blobs::Models::UserDelegationKey userDelegationKey;
      userDelegationKey.SignedObjectId = "testSignedObjectId";
      userDelegationKey.SignedTenantId = "testSignedTenantId";
      userDelegationKey.SignedStartsOn = sasStartsOn;
      userDelegationKey.SignedExpiresOn = sasExpiresOn;
      userDelegationKey.SignedService = "b";
      userDelegationKey.SignedVersion = "2020-08-04";
      userDelegationKey.Value = accountKey;

      Sas::DataLakeSasBuilder datalakeSasBuilder;
      datalakeSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      datalakeSasBuilder.StartsOn = sasStartsOn;
      datalakeSasBuilder.ExpiresOn = sasExpiresOn;
      datalakeSasBuilder.FileSystemName = "container";
      datalakeSasBuilder.Path = "blob";
      datalakeSasBuilder.Resource = Sas::DataLakeSasResource::File;
      datalakeSasBuilder.DelegatedUserObjectId = "TestDelegatedUserObjectId";
      datalakeSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::Read);
      auto sasToken = datalakeSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(blobUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign
          = datalakeSasBuilder.GenerateSasStringToSign(userDelegationKey, accountName);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
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

  TEST_F(DataLakeSasTest, PrincipalBoundDelegationSas_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    auto tokenCredential = GetTestCredential();
    auto delegatedUserObjectId = getObjectIdFromTokenCredential(tokenCredential);

    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn).Value;

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
    fileSasBuilder.DelegatedUserObjectId = delegatedUserObjectId;

    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);
    auto sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);

    Files::DataLake::DataLakeFileClient fileClient1(
        AppendQueryParameters(Azure::Core::Url(dataLakeFileClient.GetUrl()), sasToken),
        GetTestCredential(),
        InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
    EXPECT_NO_THROW(fileClient1.GetProperties());

    fileSasBuilder.DelegatedUserObjectId = "invalidObjectId";
    sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);
    Files::DataLake::DataLakeFileClient fileClient2(
        AppendQueryParameters(Azure::Core::Url(dataLakeFileClient.GetUrl()), sasToken),
        GetTestCredential(),
        InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
    EXPECT_THROW(fileClient2.GetProperties(), StorageException);
  }

  TEST_F(DataLakeSasTest, PrincipalBoundDelegationSas_CrossTenant_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Azure::Identity::ClientSecretCredentialOptions credentialOptions;
    credentialOptions.AdditionallyAllowedTenants = {"*"};
    auto endUserCredential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        GetEnv("AZURE_TENANT_ID_CROSS_TENANT"),
        GetEnv("AZURE_CLIENT_ID_CROSS_TENANT"),
        GetEnv("AZURE_CLIENT_SECRET_CROSS_TENANT"));
    auto delegatedUserObjectId = getObjectIdFromTokenCredential(endUserCredential);

    Files::DataLake::GetUserDelegationKeyOptions options;
    options.DelegatedUserTid = "4ab3a968-f1ae-47a6-b82c-f654612122a9";
    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn, options).Value;

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
    fileSasBuilder.DelegatedUserObjectId = delegatedUserObjectId;

    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);
    auto sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);

    Files::DataLake::DataLakeFileClient fileClient1(
        AppendQueryParameters(Azure::Core::Url(dataLakeFileClient.GetUrl()), sasToken),
        endUserCredential,
        InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
    EXPECT_NO_THROW(fileClient1.GetProperties());

    options.DelegatedUserTid = "00000000-0000-0000-0000-000000000000";
    userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn, options).Value;

    sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);
    Files::DataLake::DataLakeFileClient fileClient2(
        AppendQueryParameters(Azure::Core::Url(dataLakeFileClient.GetUrl()), sasToken),
        endUserCredential,
        InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
    EXPECT_THROW(fileClient2.GetProperties(), StorageException);
  }

  TEST_F(DataLakeSasTest, DynamicSas_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential = _internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Files::DataLake::Models::UserDelegationKey userDelegationKey
        = GetDataLakeServiceClientOAuth().GetUserDelegationKey(sasExpiresOn).Value;

    std::string fileName = RandomString();

    auto dataLakeFileSystemClient = *m_fileSystemClient;
    auto dataLakeFileClient = dataLakeFileSystemClient.GetFileClient(fileName);
    dataLakeFileClient.Create();
    auto buffer = RandomBuffer(1024);
    auto stream = Azure::Core::IO::MemoryBodyStream(buffer);
    Files::DataLake::AppendFileOptions appendOptions;
    appendOptions.Flush = true;
    dataLakeFileClient.Append(stream, 0, appendOptions);

    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = fileName;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;

    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);

    // cSpell:disable
    std::map<std::string, std::string> requestHeaders;
    requestHeaders["x-ms-range"] = "bytes=0-1023";
    requestHeaders["x-ms-upn"] = "true";

    std::map<std::string, std::string> requestQueryParameters;
    requestQueryParameters["spr"] = "https,http";
    requestQueryParameters["sks"] = "b";

    fileSasBuilder.RequestHeaders = requestHeaders;
    fileSasBuilder.RequestQueryParameters = requestQueryParameters;
    auto sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);

    Files::DataLake::DownloadFileOptions downloadOptions;
    Core::Http::HttpRange range;
    range.Offset = 0;
    range.Length = 1024;
    downloadOptions.Range = range;
    downloadOptions.IncludeUserPrincipalName = true;

    Files::DataLake::DataLakeFileClient fileClient1(
        AppendQueryParameters(Azure::Core::Url(dataLakeFileClient.GetUrl()), sasToken),
        InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
    EXPECT_NO_THROW(fileClient1.Download(downloadOptions));

    requestHeaders["foo$"] = "bar!";
    requestHeaders["company"] = "msft";
    requestHeaders["city"] = "redmond,atlanta,reston";

    requestQueryParameters["hello$"] = "world!";
    requestQueryParameters["abra"] = "cadabra";
    requestQueryParameters["firstName"] = "john,Tim";
    // cSpell:enable

    fileSasBuilder.RequestHeaders = requestHeaders;
    fileSasBuilder.RequestQueryParameters = requestQueryParameters;

    sasToken = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);
    Files::DataLake::DataLakeFileClient fileClient2(
        AppendQueryParameters(Azure::Core::Url(dataLakeFileClient.GetUrl()), sasToken),
        InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>());
    EXPECT_THROW(fileClient2.Download(downloadOptions), StorageException);
  }
}}} // namespace Azure::Storage::Test
