// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "block_blob_client_test.hpp"

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs/blob_sas_builder.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  class BlobSasTest : public BlockBlobClientTest {
  public:
    template <class T> T GetSasAuthenticatedClient(const T& blobClient, const std::string& sasToken)
    {
      T blobClient1(
          AppendQueryParameters(Azure::Core::Url(blobClient.GetUrl()), sasToken),
          InitStorageClientOptions<Blobs::BlobClientOptions>());
      return blobClient1;
    }

    void VerifyBlobSasRead(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
      EXPECT_NO_THROW(blobClient1.GetProperties());
    }

    void VerifyBlobSasNonRead(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
      EXPECT_THROW(blobClient1.GetProperties(), StorageException);
    }

    void VerifyBlobSasWrite(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      auto blobClient1 = GetSasAuthenticatedClient(blobClient.AsBlockBlobClient(), sasToken);
      EXPECT_NO_THROW(blobClient1.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1));
    }

    void VerifyBlobSasDelete(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      auto blobClient1 = GetSasAuthenticatedClient(blobClient.AsBlockBlobClient(), sasToken);
      Blobs::DeleteBlobOptions options;
      options.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
      EXPECT_NO_THROW(blobClient1.Delete(options));
      blobClient.AsBlockBlobClient().UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1);
    }

    void VerifyBlobSasAdd(const Blobs::AppendBlobClient& blobClient, const std::string& sasToken)
    {
      blobClient.CreateIfNotExists();
      auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
      const std::string content = "Hello world";
      auto blockContent = Azure::Core::IO::MemoryBodyStream(
          reinterpret_cast<const uint8_t*>(content.data()), content.size());
      EXPECT_NO_THROW(blobClient1.AppendBlock(blockContent));
    }

    void VerifyBlobSasList(
        const Blobs::BlobContainerClient& blobContainerClient,
        const std::string& sasToken)
    {
      auto blobContainerClient1 = GetSasAuthenticatedClient(blobContainerClient, sasToken);
      EXPECT_NO_THROW(blobContainerClient1.ListBlobs());
    }

    void VerifyBlobSasCreate(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
      EXPECT_NO_THROW(blobClient1.CreateSnapshot());
    }

    void VerifyBlobSasTags(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      std::map<std::string, std::string> tags = {{"tag_key1", "tag_value1"}};
      blobClient.SetTags(tags);
      auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
      EXPECT_NO_THROW(blobClient1.GetTags());
    }

    void VerifyBlobSasFilter(
        const Blobs::BlobContainerClient& blobContainerClient,
        const std::string& sasToken)
    {
      auto blobContainerClient1 = GetSasAuthenticatedClient(blobContainerClient, sasToken);
      EXPECT_NO_THROW(blobContainerClient1.FindBlobsByTags("\"tag_key1\" = 'tag_value1'"));
    }

    void VerifyBlobSasImmutability(const Blobs::BlobClient& blobClient, const std::string& sasToken)
    {
      (void)blobClient;
      (void)sasToken;
      // Disabled because there's no way to enable immutability on a container with dataplane API
    }
  };

  TEST_F(BlobSasTest, AccountSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobContainerClient = *m_blobContainerClient;
    auto blobClient = *m_blockBlobClient;

    for (auto permissions : {
             Sas::AccountSasPermissions::All,
             Sas::AccountSasPermissions::Read,
             Sas::AccountSasPermissions::Write,
             Sas::AccountSasPermissions::Delete,
             Sas::AccountSasPermissions::DeleteVersion,
             Sas::AccountSasPermissions::List,
             Sas::AccountSasPermissions::Add,
             Sas::AccountSasPermissions::Create,
             Sas::AccountSasPermissions::Tags,
             Sas::AccountSasPermissions::Filter,
             Sas::AccountSasPermissions::SetImmutabilityPolicy,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::AccountSasPermissions::Read) == Sas::AccountSasPermissions::Read)
      {
        VerifyBlobSasRead(blobClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        VerifyBlobSasWrite(blobClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        VerifyBlobSasDelete(blobClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        VerifyBlobSasList(blobContainerClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Add) == Sas::AccountSasPermissions::Add)
      {
        VerifyBlobSasAdd(blobContainerClient.GetAppendBlobClient(RandomString()), sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        VerifyBlobSasCreate(blobClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Tags) == Sas::AccountSasPermissions::Tags)
      {
        VerifyBlobSasTags(blobClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Filter) == Sas::AccountSasPermissions::Filter)
      {
        VerifyBlobSasFilter(blobContainerClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::SetImmutabilityPolicy)
          == Sas::AccountSasPermissions::SetImmutabilityPolicy)
      {
        VerifyBlobSasImmutability(blobClient, sasToken);
      }
    }
  }

  TEST_F(BlobSasTest, ServiceContainerSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Blobs::Models::UserDelegationKey userDelegationKey;
    {
      auto blobServiceClient = Blobs::BlobServiceClient(
          m_blobServiceClient->GetUrl(),
          GetTestCredential(),
          InitStorageClientOptions<Blobs::BlobClientOptions>());
      userDelegationKey = blobServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    auto blobContainerClient = *m_blobContainerClient;
    auto blobClient = *m_blockBlobClient;

    Sas::BlobSasBuilder containerSasBuilder;
    containerSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    containerSasBuilder.StartsOn = sasStartsOn;
    containerSasBuilder.ExpiresOn = sasExpiresOn;
    containerSasBuilder.BlobContainerName = m_containerName;
    containerSasBuilder.Resource = Sas::BlobSasResource::BlobContainer;

    for (auto permissions : {
             Sas::BlobContainerSasPermissions::All,
             Sas::BlobContainerSasPermissions::Read,
             Sas::BlobContainerSasPermissions::Write,
             Sas::BlobContainerSasPermissions::Delete,
             Sas::BlobContainerSasPermissions::List,
             Sas::BlobContainerSasPermissions::Add,
             Sas::BlobContainerSasPermissions::Create,
             Sas::BlobContainerSasPermissions::Tags,
             Sas::BlobContainerSasPermissions::SetImmutabilityPolicy,
         })
    {
      containerSasBuilder.SetPermissions(permissions);
      auto sasToken = containerSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = containerSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::BlobContainerSasPermissions::Read)
          == Sas::BlobContainerSasPermissions::Read)
      {
        VerifyBlobSasRead(blobClient, sasToken);
        VerifyBlobSasRead(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Write)
          == Sas::BlobContainerSasPermissions::Write)
      {
        VerifyBlobSasWrite(blobClient, sasToken);
        VerifyBlobSasWrite(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Delete)
          == Sas::BlobContainerSasPermissions::Delete)
      {
        VerifyBlobSasDelete(blobClient, sasToken);
        VerifyBlobSasDelete(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::List)
          == Sas::BlobContainerSasPermissions::List)
      {
        VerifyBlobSasList(blobContainerClient, sasToken);
        VerifyBlobSasList(blobContainerClient, sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Create)
          == Sas::BlobContainerSasPermissions::Create)
      {
        VerifyBlobSasCreate(blobClient, sasToken);
        VerifyBlobSasCreate(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Tags)
          == Sas::BlobContainerSasPermissions::Tags)
      {
        VerifyBlobSasTags(blobClient, sasToken);
        VerifyBlobSasTags(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::SetImmutabilityPolicy)
          == Sas::BlobContainerSasPermissions::SetImmutabilityPolicy)
      {
        VerifyBlobSasImmutability(blobClient, sasToken);
        VerifyBlobSasImmutability(blobClient, sasToken2);
      }
    }

    const auto appendBlobName = RandomString();
    auto appendBlobClient = blobContainerClient.GetAppendBlobClient(appendBlobName);
    containerSasBuilder.BlobName = appendBlobName;

    for (auto permissions : {
             Sas::BlobContainerSasPermissions::All,
             Sas::BlobContainerSasPermissions::Add,
         })
    {
      containerSasBuilder.SetPermissions(permissions);
      auto sasToken = containerSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = containerSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      if ((permissions & Sas::BlobContainerSasPermissions::Add)
          == Sas::BlobContainerSasPermissions::Add)
      {
        VerifyBlobSasAdd(appendBlobClient, sasToken);
        VerifyBlobSasAdd(appendBlobClient, sasToken2);
      }
    }
  }

  TEST_F(BlobSasTest, ServiceBlobSasPermissions_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    Blobs::Models::UserDelegationKey userDelegationKey;
    {
      auto blobServiceClient = Blobs::BlobServiceClient(
          m_blobServiceClient->GetUrl(),
          GetTestCredential(),
          InitStorageClientOptions<Blobs::BlobClientOptions>());
      userDelegationKey = blobServiceClient.GetUserDelegationKey(sasExpiresOn).Value;
    }

    auto blobContainerClient = *m_blobContainerClient;
    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.StartsOn = sasStartsOn;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;

    for (auto permissions : {
             Sas::BlobSasPermissions::All,
             Sas::BlobSasPermissions::Read,
             Sas::BlobSasPermissions::Write,
             Sas::BlobSasPermissions::Delete,
             Sas::BlobSasPermissions::Add,
             Sas::BlobSasPermissions::Create,
             Sas::BlobSasPermissions::Tags,
             Sas::BlobSasPermissions::DeleteVersion,
             Sas::BlobSasPermissions::SetImmutabilityPolicy,
         })
    {
      blobSasBuilder.SetPermissions(permissions);
      auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = blobSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::BlobSasPermissions::Read) == Sas::BlobSasPermissions::Read)
      {
        VerifyBlobSasRead(blobClient, sasToken);
        VerifyBlobSasRead(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Write) == Sas::BlobSasPermissions::Write)
      {
        VerifyBlobSasWrite(blobClient, sasToken);
        VerifyBlobSasWrite(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Delete) == Sas::BlobSasPermissions::Delete)
      {
        VerifyBlobSasDelete(blobClient, sasToken);
        VerifyBlobSasDelete(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Create) == Sas::BlobSasPermissions::Create)
      {
        VerifyBlobSasCreate(blobClient, sasToken);
        VerifyBlobSasCreate(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Tags) == Sas::BlobSasPermissions::Tags)
      {
        VerifyBlobSasTags(blobClient, sasToken);
        VerifyBlobSasTags(blobClient, sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::SetImmutabilityPolicy)
          == Sas::BlobSasPermissions::SetImmutabilityPolicy)
      {
        VerifyBlobSasImmutability(blobClient, sasToken);
        VerifyBlobSasImmutability(blobClient, sasToken2);
      }
    }

    const auto appendBlobName = RandomString();
    auto appendBlobClient = blobContainerClient.GetAppendBlobClient(appendBlobName);
    blobSasBuilder.BlobName = appendBlobName;

    for (auto permissions : {
             Sas::BlobSasPermissions::All,
             Sas::BlobSasPermissions::Add,
         })
    {
      blobSasBuilder.SetPermissions(permissions);
      auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = blobSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      if ((permissions & Sas::BlobSasPermissions::Add) == Sas::BlobSasPermissions::Add)
      {
        VerifyBlobSasAdd(appendBlobClient, sasToken);
        VerifyBlobSasAdd(appendBlobClient, sasToken2);
      }
    }
  }

  TEST_F(BlobSasTest, AccountSasExpired_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiredOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasNonRead(blobClient, sasToken);

    accountSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, ServiceSasExpired_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.StartsOn = sasStartsOn;
    blobSasBuilder.ExpiresOn = sasExpiredOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
    blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::All);

    auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasNonRead(blobClient, sasToken);

    blobSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, AccountSasWithoutStarttime_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, ServiceSasWithoutStartTime_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
    blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::All);

    auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, AccountSasWithIP_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);

    accountSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasNonRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, ServiceSasWithIP_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.StartsOn = sasStartsOn;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
    blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::All);

    auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);

    blobSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasNonRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, AccountSasService_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    accountSasBuilder.Services = Sas::AccountSasServices::Files;

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasNonRead(blobClient, sasToken);

    accountSasBuilder.Services = Sas::AccountSasServices::All;
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, AccountSasResourceTypes_LIVEONLY_)
  {
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::Service;

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasNonRead(blobClient, sasToken);

    auto blobServiceClient1 = GetSasAuthenticatedClient(*m_blobServiceClient, sasToken);
    EXPECT_NO_THROW(blobServiceClient1.ListBlobContainers());
  }

  TEST_F(BlobSasTest, BlobSasWithIdentifier_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobContainerClient = *m_blobContainerClient;
    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Blobs::SetBlobContainerAccessPolicyOptions options;
    options.AccessType = Blobs::Models::PublicAccessType::None;
    Blobs::Models::SignedIdentifier identifier;
    identifier.Id = RandomString(64);
    identifier.StartsOn = sasStartsOn;
    identifier.ExpiresOn = sasExpiresOn;
    identifier.Permissions = "r";
    options.SignedIdentifiers.emplace_back(identifier);
    blobContainerClient.SetAccessPolicy(options);

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
    blobSasBuilder.SetPermissions(static_cast<Sas::BlobSasPermissions>(0));
    blobSasBuilder.Identifier = identifier.Id;

    TestSleep(std::chrono::seconds(30));

    auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    VerifyBlobSasRead(blobClient, sasToken);
  }

  TEST_F(BlobSasTest, BlobSasResponseHeadersOverride_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.StartsOn = sasStartsOn;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
    blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::All);
    blobSasBuilder.ContentType = "application/x-binary";
    blobSasBuilder.ContentLanguage = "en-US";
    blobSasBuilder.ContentDisposition = "attachment";
    blobSasBuilder.CacheControl = "no-cache";
    blobSasBuilder.ContentEncoding = "identify";

    auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
    auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
    auto properties = blobClient1.GetProperties().Value;
    EXPECT_EQ(properties.HttpHeaders.ContentType, blobSasBuilder.ContentType);
    EXPECT_EQ(properties.HttpHeaders.ContentLanguage, blobSasBuilder.ContentLanguage);
    EXPECT_EQ(properties.HttpHeaders.ContentDisposition, blobSasBuilder.ContentDisposition);
    EXPECT_EQ(properties.HttpHeaders.CacheControl, blobSasBuilder.CacheControl);
    EXPECT_EQ(properties.HttpHeaders.ContentEncoding, blobSasBuilder.ContentEncoding);
  }

  TEST_F(BlobSasTest, AccountSasEncryptionScope_LIVEONLY_)
  {
    const auto encryptionScope = GetTestEncryptionScope();

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobContainerClient = *m_blobContainerClient;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    accountSasBuilder.EncryptionScope = encryptionScope;

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    auto blobContainerClient1 = GetSasAuthenticatedClient(blobContainerClient, sasToken);
    auto appendBlobClient1 = blobContainerClient1.GetAppendBlobClient(RandomString());

    appendBlobClient1.Create();
    auto properties = appendBlobClient1.GetProperties().Value;
    ASSERT_TRUE(properties.EncryptionScope.HasValue());
    EXPECT_EQ(properties.EncryptionScope.Value(), encryptionScope);
  }

  TEST_F(BlobSasTest, ServiceSasEncryptionScope_LIVEONLY_)
  {
    const auto encryptionScope = GetTestEncryptionScope();

    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobContainerClient = *m_blobContainerClient;

    Sas::BlobSasBuilder containerSasBuilder;
    containerSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    containerSasBuilder.StartsOn = sasStartsOn;
    containerSasBuilder.ExpiresOn = sasExpiresOn;
    containerSasBuilder.BlobContainerName = m_containerName;
    containerSasBuilder.Resource = Sas::BlobSasResource::BlobContainer;
    containerSasBuilder.SetPermissions(Sas::BlobSasPermissions::All);
    containerSasBuilder.EncryptionScope = encryptionScope;

    auto sasToken = containerSasBuilder.GenerateSasToken(*keyCredential);
    auto blobContainerClient1 = GetSasAuthenticatedClient(blobContainerClient, sasToken);
    auto appendBlobClient1 = blobContainerClient1.GetAppendBlobClient(RandomString());

    appendBlobClient1.Create();
    auto properties = appendBlobClient1.GetProperties().Value;
    ASSERT_TRUE(properties.EncryptionScope.HasValue());
    EXPECT_EQ(properties.EncryptionScope.Value(), encryptionScope);
  }

  TEST_F(BlobSasTest, ServiceSasPermissionDeleteVersion_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;
    auto response = blobClient.GetProperties();
    const std::string versionId = response.Value.VersionId.Value();
    blobClient.SetMetadata({}); // need to modify the blob so that the version id above does not
                                // point to the root blob.

    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.StartsOn = sasStartsOn;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::BlobVersion;
    blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::DeleteVersion);
    blobSasBuilder.BlobVersionId = versionId;
    auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);

    auto blobClient1 = GetSasAuthenticatedClient(blobClient, sasToken);
    blobClient1 = blobClient1.WithVersionId(versionId);
    EXPECT_NO_THROW(blobClient1.Delete());
  }

  TEST_F(BlobSasTest, AccountSasAuthorizationErrorDetail_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;

    auto blobContainerClient = *m_blobContainerClient;
    auto blobClient = *m_blockBlobClient;
    const std::string blobName = m_blobName;

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::Service;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    auto unauthorizedBlobClient = GetSasAuthenticatedClient(blobClient, sasToken);
    try
    {
      unauthorizedBlobClient.Download();
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

    // Account Sas
    {
      Sas::AccountSasBuilder accountSasBuilder;
      accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      accountSasBuilder.StartsOn = sasStartsOn;
      accountSasBuilder.ExpiresOn = sasExpiresOn;
      accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
      accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
      accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::Read);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(blobUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign = accountSasBuilder.GenerateStringToSign(*keyCredential);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
    }

    // Blob Sas
    {
      Sas::BlobSasBuilder blobSasBuilder;
      blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      blobSasBuilder.StartsOn = sasStartsOn;
      blobSasBuilder.ExpiresOn = sasExpiresOn;
      blobSasBuilder.BlobContainerName = "container";
      blobSasBuilder.BlobName = "blob";
      blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
      blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::Read);
      auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(blobUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign = blobSasBuilder.GenerateStringToSign(*keyCredential);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
    }

    // Blob User Delegation Sas
    {
      Blobs::Models::UserDelegationKey userDelegationKey;
      userDelegationKey.SignedObjectId = "testSignedObjectId";
      userDelegationKey.SignedTenantId = "testSignedTenantId";
      userDelegationKey.SignedStartsOn = sasStartsOn;
      userDelegationKey.SignedExpiresOn = sasExpiresOn;
      userDelegationKey.SignedService = "b";
      userDelegationKey.SignedVersion = "2020-08-04";
      userDelegationKey.Value = accountKey;

      Sas::BlobSasBuilder blobSasBuilder;
      blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      blobSasBuilder.StartsOn = sasStartsOn;
      blobSasBuilder.ExpiresOn = sasExpiresOn;
      blobSasBuilder.BlobContainerName = "container";
      blobSasBuilder.BlobName = "blob";
      blobSasBuilder.Resource = Sas::BlobSasResource::Blob;
      blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::Read);
      auto sasToken = blobSasBuilder.GenerateSasToken(userDelegationKey, accountName);
      auto signature = Azure::Core::Url::Decode(
          Azure::Core::Url(blobUrl + sasToken).GetQueryParameters().find("sig")->second);
      auto stringToSign = blobSasBuilder.GenerateStringToSign(userDelegationKey, accountName);
      auto signatureFromStringToSign = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
          std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
          Azure::Core::Convert::Base64Decode(accountKey)));
      EXPECT_EQ(signature, signatureFromStringToSign);
    }
  }
}}} // namespace Azure::Storage::Test
