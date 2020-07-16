// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/file_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/token_credential_policy.hpp"
#include "datalake/datalake_utilities.hpp"
#include "http/curl/curl.hpp"

#include <limits>
#include <utility> //std::pair

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace {
    std::pair<int64_t, int64_t> GetOffsetLength(const std::string& rangeString)
    {
      int64_t offset = std::numeric_limits<int64_t>::max();
      int64_t length = std::numeric_limits<int64_t>::max();
      const std::string c_bytesPrefix = "bytes ";
      if (rangeString.length() > c_bytesPrefix.length())
      {
        auto subRangeString = rangeString.substr(c_bytesPrefix.length());
        std::string::const_iterator cur = subRangeString.begin();
        offset = std::stoll(Details::GetSubstringTillDelimiter('-', subRangeString, cur));
        if (cur != subRangeString.end())
        {
          length = std::stoll(Details::GetSubstringTillDelimiter('/', subRangeString, cur)) - offset
              + 1;
        }
        else
        {
          throw std::runtime_error("The format of the range string is not correct: " + rangeString);
        }
      }
      return std::make_pair(offset, length);
    }
  } // namespace

  FileClient FileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& filePath,
      const FileClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileUri = std::move(parsedConnectionString.DataLakeServiceUri);
    fileUri.AppendPath(fileSystemName, true);
    fileUri.AppendPath(filePath, true);

    if (parsedConnectionString.KeyCredential)
    {
      return FileClient(fileUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return FileClient(fileUri.ToString(), options);
    }
  }

  FileClient::FileClient(
      const std::string& fileUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const FileClientOptions& options)
      : PathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileClient::FileClient(
      const std::string& fileUri,
      std::shared_ptr<TokenCredential> credential,
      const FileClientOptions& options)
      : PathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<TokenCredentialPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileClient::FileClient(const std::string& fileUri, const FileClientOptions& options)
      : PathClient(fileUri, options), m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  PathAppendDataResponse FileClient::AppendData(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const PathAppendDataOptions& options) const
  {
    DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = content->Length();
    protocolLayerOptions.TransactionalContentMD5 = options.ContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return DataLakeRestClient::Path::AppendData(
        m_dfsUri.ToString(), *content, *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathFlushDataResponse FileClient::FlushData(
      int64_t endingOffset,
      const PathFlushDataOptions& options) const
  {
    DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = endingOffset;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.CacheControl = options.HttpHeaders.CacheControl;
    protocolLayerOptions.ContentType = options.HttpHeaders.ContentType;
    protocolLayerOptions.ContentDisposition = options.HttpHeaders.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.HttpHeaders.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.HttpHeaders.ContentLanguage;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return DataLakeRestClient::Path::FlushData(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileRenameResponse FileClient::Rename(
      const std::string& destinationPath,
      const FileRenameOptions& options)
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_dfsUri.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = Details::GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUri = m_dfsUri;
    destinationDfsUri.SetPath(destinationFileSystem.GetValue() + '/' + destinationPath);

    DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    protocolLayerOptions.Mode = options.Mode;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RenameSource = "/" + m_dfsUri.GetPath();
    auto result = DataLakeRestClient::Path::Create(
        destinationDfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    m_dfsUri = std::move(destinationDfsUri);
    m_blobClient = Blobs::BlobClient(
        UriBuilder(Details::GetBlobUriFromUri(m_dfsUri.ToString())), m_pipeline);
    m_blockBlobClient = Blobs::BlockBlobClient(m_blobClient);
    auto ret = FileRenameResponse();
    ret.Date = std::move(result.Date);
    ret.ETag = std::move(result.ETag);
    ret.LastModified = std::move(result.LastModified);
    ret.RequestId = std::move(result.RequestId);
    ret.Version = std::move(result.Version);
    return ret;
  }

  FileDeleteResponse FileClient::Delete(const FileDeleteOptions& options) const
  {
    DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = DataLakeRestClient::Path::Delete(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    auto ret = FileDeleteResponse();
    ret.Date = std::move(result.Date);
    ret.RequestId = std::move(result.RequestId);
    ret.Version = std::move(result.Version);
    return ret;
  }

  ReadPathResponse FileClient::Read(const FileReadOptions& options) const
  {
    DataLakeRestClient::Path::ReadOptions protocolLayerOptions;
    if (options.Offset.HasValue())
    {
      auto rangeString = std::string("bytes=" + std::to_string(options.Offset.GetValue()) + "-");
      if (options.Length.HasValue())
      {
        rangeString += std::to_string(options.Offset.GetValue() + options.Length.GetValue() - 1);
      }
      protocolLayerOptions.Range = std::move(rangeString);
    }

    protocolLayerOptions.XMsRangeGetContentMd5 = options.RangeGetContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = DataLakeRestClient::Path::Read(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    Azure::Core::Nullable<int64_t> RangeOffset;
    Azure::Core::Nullable<int64_t> RangeLength;
    if (result.ContentRange.HasValue())
    {
      auto range = GetOffsetLength(result.ContentRange.GetValue());
      RangeOffset = range.first;
      RangeLength = range.second;
    }
    auto returnVal = ReadPathResponse{};
    returnVal.Body = std::move(result.BodyStream);
    returnVal.AcceptRanges = std::move(result.AcceptRanges);
    returnVal.HttpHeaders = std::move(result.HttpHeaders);
    returnVal.ContentLength = result.ContentLength;
    returnVal.RangeOffset = RangeOffset;
    returnVal.RangeLength = RangeLength;
    returnVal.TransactionalMD5 = std::move(result.TransactionalMD5);
    returnVal.Date = std::move(result.Date);
    returnVal.ETag = std::move(result.ETag);
    returnVal.LastModified = std::move(result.LastModified);
    returnVal.RequestId = std::move(result.RequestId);
    returnVal.Version = std::move(result.Version);
    returnVal.ResourceType = std::move(result.ResourceType);
    returnVal.LeaseDuration = std::move(result.LeaseDuration);
    returnVal.LeaseState = result.LeaseState;
    returnVal.LeaseStatus = result.LeaseStatus;
    returnVal.ContentMD5 = std::move(result.ContentMD5);
    auto rawProperties
        = result.Properties.HasValue() ? result.Properties.GetValue() : std::string();
    returnVal.Metadata = Details::DeserializeMetadata(rawProperties);
    return returnVal;
  }
}}}} // namespace Azure::Storage::Files::DataLake
