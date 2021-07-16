// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_file_client.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/io/null_body_stream.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/concurrent_transfer.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/file_io.hpp>
#include <azure/storage/common/internal/reliable_stream.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/shares/share_constants.hpp"

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareFileClient ShareFileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& fileName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto fileUrl = std::move(parsedConnectionString.FileServiceUrl);
    fileUrl.AppendPath(_internal::UrlEncodePath(shareName));
    fileUrl.AppendPath(_internal::UrlEncodePath(fileName));

    if (parsedConnectionString.KeyCredential)
    {
      return ShareFileClient(
          fileUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareFileClient(fileUrl.GetAbsoluteUrl(), options);
    }
  }

  ShareFileClient::ShareFileClient(
      const std::string& shareFileUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareFileUrl(shareFileUrl)
  {
    ShareClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::FileServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareFileClient::ShareFileClient(
      const std::string& shareFileUrl,
      const ShareClientOptions& options)
      : m_shareFileUrl(shareFileUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::FileServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareFileClient ShareFileClient::WithShareSnapshot(const std::string& shareSnapshot) const
  {
    ShareFileClient newClient(*this);
    if (shareSnapshot.empty())
    {
      newClient.m_shareFileUrl.RemoveQueryParameter(_detail::ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareFileUrl.AppendQueryParameter(
          _detail::ShareSnapshotQueryParameter, _internal::UrlEncodeQueryParameter(shareSnapshot));
    }
    return newClient;
  }

  Azure::Response<Models::CreateFileResult> ShareFileClient::Create(
      int64_t fileSize,
      const CreateFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.ToString();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
    }
    if (options.Permission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.Permission;
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
    }
    protocolLayerOptions.XMsContentLength = fileSize;
    if (!options.HttpHeaders.ContentType.empty())
    {
      protocolLayerOptions.FileContentType = options.HttpHeaders.ContentType;
    }
    if (!options.HttpHeaders.ContentEncoding.empty())
    {
      protocolLayerOptions.FileContentEncoding = options.HttpHeaders.ContentEncoding;
    }
    if (!options.HttpHeaders.ContentLanguage.empty())
    {
      protocolLayerOptions.FileContentLanguage = options.HttpHeaders.ContentLanguage;
    }
    if (!options.HttpHeaders.CacheControl.empty())
    {
      protocolLayerOptions.FileCacheControl = options.HttpHeaders.CacheControl;
    }
    if (!options.HttpHeaders.ContentDisposition.empty())
    {
      protocolLayerOptions.FileContentDisposition = options.HttpHeaders.ContentDisposition;
    }
    if (!options.HttpHeaders.ContentHash.Value.empty())
    {
      AZURE_ASSERT_MSG(
          options.HttpHeaders.ContentHash.Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
      protocolLayerOptions.ContentMd5 = options.HttpHeaders.ContentHash;
    }
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto result = _detail::ShareRestClient::File::Create(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateFileResult ret;
    ret.Created = true;
    ret.ETag = std::move(result.Value.ETag);
    ret.SmbProperties = std::move(result.Value.SmbProperties);
    ret.IsServerEncrypted = result.Value.IsServerEncrypted;
    ret.LastModified = std::move(result.Value.LastModified);

    return Azure::Response<Models::CreateFileResult>(std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteFileResult> ShareFileClient::Delete(
      const DeleteFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::DeleteOptions();
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto result = _detail::ShareRestClient::File::Delete(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteFileResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteFileResult>(std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteFileResult> ShareFileClient::DeleteIfExists(
      const DeleteFileOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ShareNotFound || e.ErrorCode == _detail::ParentNotFound
          || e.ErrorCode == _detail::ResourceNotFound)
      {
        Models::DeleteFileResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeleteFileResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DownloadFileResult> ShareFileClient::Download(
      const DownloadFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::DownloadOptions();
    if (options.Range.HasValue())
    {
      if (options.Range.Value().Length.HasValue())
      {
        protocolLayerOptions.Range = std::string("bytes=")
            + std::to_string(options.Range.Value().Offset) + std::string("-")
            + std::to_string(options.Range.Value().Offset + options.Range.Value().Length.Value()
                             - 1);
      }
      else
      {
        protocolLayerOptions.Range = std::string("bytes=")
            + std::to_string(options.Range.Value().Offset) + std::string("-");
      }
    }
    if (options.RangeHashAlgorithm.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.RangeHashAlgorithm.Value() == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
      if (options.RangeHashAlgorithm.Value() == HashAlgorithm::Md5)
      {
        protocolLayerOptions.GetRangeContentMd5 = true;
      }
    }
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;

    auto downloadResponse = _detail::ShareRestClient::File::Download(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    {
      // In case network failure during reading the body
      auto eTag = downloadResponse.Value.ETag;

      auto retryFunction
          = [this, options, eTag](int64_t retryOffset, const Azure::Core::Context& context)
          -> std::unique_ptr<Azure::Core::IO::BodyStream> {
        DownloadFileOptions newOptions = options;
        newOptions.Range = Core::Http::HttpRange();
        newOptions.Range.Value().Offset
            = (options.Range.HasValue() ? options.Range.Value().Offset : 0) + retryOffset;
        if (options.Range.HasValue() && options.Range.Value().Length.HasValue())
        {
          newOptions.Range.Value().Length = options.Range.Value().Length.Value() - retryOffset;
        }

        auto newResponse = Download(newOptions, context);
        if (eTag != newResponse.Value.Details.ETag)
        {
          throw Azure::Core::RequestFailedException("File was modified in the middle of download.");
        }
        return std::move(newResponse.Value.BodyStream);
      };

      _internal::ReliableStreamOptions reliableStreamOptions;
      reliableStreamOptions.MaxRetryRequests = _internal::ReliableStreamRetryCount;
      downloadResponse.Value.BodyStream = std::make_unique<_internal::ReliableStream>(
          std::move(downloadResponse.Value.BodyStream), reliableStreamOptions, retryFunction);
    }
    Models::DownloadFileResult ret;
    ret.BodyStream = std::move(downloadResponse.Value.BodyStream);
    ret.ContentRange = std::move(downloadResponse.Value.ContentRange);
    ret.FileSize = downloadResponse.Value.FileSize;
    ret.TransactionalContentHash = std::move(downloadResponse.Value.TransactionalContentHash);
    ret.HttpHeaders = std::move(downloadResponse.Value.HttpHeaders);
    ret.Details.LastModified = std::move(downloadResponse.Value.LastModified);
    ret.Details.Metadata = std::move(downloadResponse.Value.Metadata);
    ret.Details.ETag = std::move(downloadResponse.Value.ETag);
    ret.Details.CopyCompletedOn = std::move(downloadResponse.Value.CopyCompletedOn);
    ret.Details.CopyStatusDescription = std::move(downloadResponse.Value.CopyStatusDescription);
    ret.Details.CopyId = std::move(downloadResponse.Value.CopyId);
    ret.Details.CopyProgress = std::move(downloadResponse.Value.CopyProgress);
    ret.Details.CopySource = std::move(downloadResponse.Value.CopySource);
    ret.Details.CopyStatus = std::move(downloadResponse.Value.CopyStatus);
    ret.Details.IsServerEncrypted = downloadResponse.Value.IsServerEncrypted;
    ret.Details.SmbProperties = std::move(downloadResponse.Value.SmbProperties);
    ret.Details.LeaseDuration = std::move(downloadResponse.Value.LeaseDuration);
    ret.Details.LeaseState = std::move(downloadResponse.Value.LeaseState);
    ret.Details.LeaseStatus = std::move(downloadResponse.Value.LeaseStatus);
    return Azure::Response<Models::DownloadFileResult>(
        std::move(ret), std::move(downloadResponse.RawResponse));
  }

  StartFileCopyOperation ShareFileClient::StartCopy(
      std::string copySource,
      const StartFileCopyOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::StartCopyOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.CopySource = std::move(copySource);
    protocolLayerOptions.FileCopyFileAttributes = options.SmbProperties.Attributes.ToString();
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCopyFileCreationTime
          = options.SmbProperties.CreatedOn.Value().ToString(
              Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCopyFileCreationTime = std::string(FileCopySourceTime);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileCopyFileLastWriteTime
          = options.SmbProperties.LastWrittenOn.Value().ToString(
              Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCopyFileLastWriteTime = std::string(FileCopySourceTime);
    }
    if (options.PermissionCopyMode.HasValue())
    {
      protocolLayerOptions.XMsFilePermissionCopyMode = options.PermissionCopyMode.Value();
      if (options.PermissionCopyMode.Value() == Models::PermissionCopyMode::Override)
      {
        if (options.Permission.HasValue())
        {
          protocolLayerOptions.FilePermission = options.Permission;
        }
        else if (options.SmbProperties.PermissionKey.HasValue())
        {
          protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
        }
        else
        {
          AZURE_ASSERT_MSG(false, "Either FilePermission or FilePermissionKey must be set.");
        }
      }
    }
    else
    {
      protocolLayerOptions.XMsFilePermissionCopyMode = Models::PermissionCopyMode::Source;
    }
    protocolLayerOptions.FileCopyIgnoreReadOnly = options.IgnoreReadOnly;
    protocolLayerOptions.FileCopySetArchiveAttribute = options.SetArchiveAttribute;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto response = _detail::ShareRestClient::File::StartCopy(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    StartFileCopyOperation res;
    res.m_rawResponse = std::move(response.RawResponse);
    res.m_fileClient = std::make_shared<ShareFileClient>(*this);
    return res;
  }

  Azure::Response<Models::AbortFileCopyResult> ShareFileClient::AbortCopy(
      std::string copyId,
      const AbortFileCopyOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::AbortCopyOptions();
    protocolLayerOptions.CopyId = std::move(copyId);
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::ShareRestClient::File::AbortCopy(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::FileProperties> ShareFileClient::GetProperties(
      const GetFilePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::GetPropertiesOptions();
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::ShareRestClient::File::GetProperties(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetFilePropertiesResult> ShareFileClient::SetProperties(
      const Models::FileHttpHeaders& httpHeaders,
      const Models::FileSmbProperties& smbProperties,
      const SetFilePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::SetHttpHeadersOptions();
    protocolLayerOptions.FileAttributes = smbProperties.Attributes.ToString();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = FilePreserveSmbProperties;
    }
    if (smbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = FilePreserveSmbProperties;
    }
    if (smbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = smbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = FilePreserveSmbProperties;
    }
    protocolLayerOptions.XMsContentLength = options.Size;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    if (options.Permission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.Permission;
    }
    else if (smbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = smbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = FilePreserveSmbProperties;
    }

    if (!httpHeaders.ContentType.empty())
    {
      protocolLayerOptions.FileContentType = httpHeaders.ContentType;
    }
    if (!httpHeaders.ContentEncoding.empty())
    {
      protocolLayerOptions.FileContentEncoding = httpHeaders.ContentEncoding;
    }
    if (!httpHeaders.ContentLanguage.empty())
    {
      protocolLayerOptions.FileContentLanguage = httpHeaders.ContentLanguage;
    }
    if (!httpHeaders.CacheControl.empty())
    {
      protocolLayerOptions.FileCacheControl = httpHeaders.CacheControl;
    }
    if (!httpHeaders.ContentDisposition.empty())
    {
      protocolLayerOptions.FileContentDisposition = httpHeaders.ContentDisposition;
    }

    return _detail::ShareRestClient::File::SetHttpHeaders(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetFileMetadataResult> ShareFileClient::SetMetadata(
      Storage::Metadata metadata,
      const SetFileMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::ShareRestClient::File::SetMetadata(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::UploadFileRangeResult> ShareFileClient::UploadRange(
      int64_t offset,
      Azure::Core::IO::BodyStream& content,
      const UploadFileRangeOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::UploadRangeOptions();
    protocolLayerOptions.XMsWrite = _detail::FileRangeWrite::Update;
    protocolLayerOptions.ContentLength = content.Length();
    protocolLayerOptions.XMsRange = std::string("bytes=") + std::to_string(offset)
        + std::string("-") + std::to_string(offset + content.Length() - 1);
    if (options.TransactionalContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
    }
    protocolLayerOptions.ContentMd5 = options.TransactionalContentHash;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::ShareRestClient::File::UploadRange(
        m_shareFileUrl, content, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::ClearFileRangeResult> ShareFileClient::ClearRange(
      int64_t offset,
      int64_t length,
      const ClearFileRangeOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::UploadRangeOptions();
    protocolLayerOptions.XMsWrite = _detail::FileRangeWrite::Clear;
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.XMsRange = std::string("bytes=") + std::to_string(offset)
        + std::string("-") + std::to_string(offset + length - 1);

    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto response = _detail::ShareRestClient::File::UploadRange(
        m_shareFileUrl,
        *Azure::Core::IO::_internal::NullBodyStream::GetNullBodyStream(),
        *m_pipeline,
        context,
        protocolLayerOptions);
    Models::ClearFileRangeResult ret;
    ret.ETag = std::move(response.Value.ETag);
    ret.IsServerEncrypted = response.Value.IsServerEncrypted;
    ret.LastModified = std::move(response.Value.LastModified);
    return Azure::Response<Models::ClearFileRangeResult>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::GetFileRangeListResult> ShareFileClient::GetRangeList(
      const GetFileRangeListOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::GetRangeListOptions();
    if (options.Range.HasValue())
    {
      if (options.Range.Value().Length.HasValue())
      {
        protocolLayerOptions.XMsRange = std::string("bytes=")
            + std::to_string(options.Range.Value().Offset) + std::string("-")
            + std::to_string(options.Range.Value().Offset + options.Range.Value().Length.Value()
                             - 1);
      }
      else
      {
        protocolLayerOptions.XMsRange = std::string("bytes=")
            + std::to_string(options.Range.Value().Offset) + std::string("-");
      }
    }

    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::ShareRestClient::File::GetRangeList(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::GetFileRangeListResult> ShareFileClient::GetRangeListDiff(
      std::string previousShareSnapshot,
      const GetFileRangeListOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::GetRangeListOptions();
    if (options.Range.HasValue())
    {
      if (options.Range.Value().Length.HasValue())
      {
        protocolLayerOptions.XMsRange = std::string("bytes=")
            + std::to_string(options.Range.Value().Offset) + std::string("-")
            + std::to_string(options.Range.Value().Offset + options.Range.Value().Length.Value()
                             - 1);
      }
      else
      {
        protocolLayerOptions.XMsRange = std::string("bytes=")
            + std::to_string(options.Range.Value().Offset) + std::string("-");
      }
    }

    protocolLayerOptions.PrevShareSnapshot = std::move(previousShareSnapshot);
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::ShareRestClient::File::GetRangeList(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  ListFileHandlesPagedResponse ShareFileClient::ListHandles(
      const ListFileHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::ListHandlesOptions();
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto response = _detail::ShareRestClient::File::ListHandles(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    ListFileHandlesPagedResponse pagedResponse;

    pagedResponse.FileHandles = std::move(response.Value.HandleList);
    pagedResponse.m_shareFileClient = std::make_shared<ShareFileClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::ForceCloseFileHandleResult> ShareFileClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseFileHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = _detail::ShareRestClient::File::ForceCloseHandles(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    return Azure::Response<Models::ForceCloseFileHandleResult>(
        Models::ForceCloseFileHandleResult(), std::move(result.RawResponse));
  }

  ForceCloseAllFileHandlesPagedResponse ShareFileClient::ForceCloseAllHandles(
      const ForceCloseAllFileHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    auto response = _detail::ShareRestClient::File::ForceCloseHandles(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    ForceCloseAllFileHandlesPagedResponse pagedResponse;

    pagedResponse.NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    pagedResponse.NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    pagedResponse.m_shareFileClient = std::make_shared<ShareFileClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::DownloadFileToResult> ShareFileClient::DownloadTo(
      uint8_t* buffer,
      size_t bufferSize,
      const DownloadFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    // Just start downloading using an initial chunk. If it's a small file, we'll get the whole
    // thing in one shot. If it's a large file, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    int64_t firstChunkOffset = options.Range.HasValue() ? options.Range.Value().Offset : 0;
    int64_t firstChunkLength = options.TransferOptions.InitialChunkSize;

    if (options.Range.HasValue() && options.Range.Value().Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Range.Value().Length.Value());
    }

    DownloadFileOptions firstChunkOptions;
    firstChunkOptions.Range = options.Range;
    if (firstChunkOptions.Range.HasValue())
    {
      firstChunkOptions.Range.Value().Length = firstChunkLength;
    }

    auto firstChunk = Download(firstChunkOptions, context);
    const Azure::ETag etag = firstChunk.Value.Details.ETag;

    int64_t fileSize;
    int64_t fileRangeSize;
    if (firstChunkOptions.Range.HasValue())
    {
      fileSize = firstChunk.Value.FileSize;
      fileRangeSize = fileSize - firstChunkOffset;
      if (options.Range.Value().Length.HasValue())
      {
        fileRangeSize = std::min(fileRangeSize, options.Range.Value().Length.Value());
      }
    }
    else
    {
      fileSize = firstChunk.Value.BodyStream->Length();
      fileRangeSize = fileSize;
    }
    firstChunkLength = std::min(firstChunkLength, fileRangeSize);

    if (static_cast<uint64_t>(fileRangeSize) > std::numeric_limits<size_t>::max()
        || static_cast<size_t>(fileRangeSize) > bufferSize)
    {
      throw Azure::Core::RequestFailedException(
          "Buffer is not big enough, file range size is " + std::to_string(fileRangeSize) + ".");
    }

    int64_t bytesRead = firstChunk.Value.BodyStream->ReadToCount(
        buffer, static_cast<size_t>(firstChunkLength), context);
    if (bytesRead != firstChunkLength)
    {
      throw Azure::Core::RequestFailedException("Error when reading body stream.");
    }
    firstChunk.Value.BodyStream.reset();

    auto returnTypeConverter = [](Azure::Response<Models::DownloadFileResult>& response) {
      Models::DownloadFileToResult ret;
      ret.FileSize = response.Value.FileSize;
      ret.HttpHeaders = std::move(response.Value.HttpHeaders);
      ret.Details = std::move(response.Value.Details);
      return Azure::Response<Models::DownloadFileToResult>(
          std::move(ret), std::move(response.RawResponse));
    };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadFileOptions chunkOptions;
            chunkOptions.Range = Core::Http::HttpRange();
            chunkOptions.Range.Value().Offset = offset;
            chunkOptions.Range.Value().Length = length;
            auto chunk = Download(chunkOptions, context);
            int64_t bytesRead = chunk.Value.BodyStream->ReadToCount(
                buffer + (offset - firstChunkOffset),
                static_cast<size_t>(chunkOptions.Range.Value().Length.Value()),
                context);
            if (bytesRead != chunkOptions.Range.Value().Length.Value())
            {
              throw Azure::Core::RequestFailedException("Error when reading body stream.");
            }
            if (chunk.Value.Details.ETag != etag)
            {
              throw Azure::Core::RequestFailedException(
                  "File was modified in the middle of download.");
            }

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = fileRangeSize - firstChunkLength;

    _internal::ConcurrentTransfer(
        remainingOffset,
        remainingSize,
        options.TransferOptions.ChunkSize,
        options.TransferOptions.Concurrency,
        downloadChunkFunc);
    ret.Value.ContentRange.Offset = firstChunkOffset;
    ret.Value.ContentRange.Length = fileRangeSize;
    return ret;
  }

  Azure::Response<Models::DownloadFileToResult> ShareFileClient::DownloadTo(
      const std::string& fileName,
      const DownloadFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    // Just start downloading using an initial chunk. If it's a small file, we'll get the whole
    // thing in one shot. If it's a large file, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    int64_t firstChunkOffset = options.Range.HasValue() ? options.Range.Value().Offset : 0;
    int64_t firstChunkLength = options.TransferOptions.InitialChunkSize;
    if (options.Range.HasValue() && options.Range.Value().Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Range.Value().Length.Value());
    }

    DownloadFileOptions firstChunkOptions;
    firstChunkOptions.Range = options.Range;
    if (firstChunkOptions.Range.HasValue())
    {
      firstChunkOptions.Range.Value().Length = firstChunkLength;
    }

    _internal::FileWriter fileWriter(fileName);

    auto firstChunk = Download(firstChunkOptions, context);
    const Azure::ETag etag = firstChunk.Value.Details.ETag;

    int64_t fileSize;
    int64_t fileRangeSize;
    if (firstChunkOptions.Range.HasValue())
    {
      fileSize = firstChunk.Value.FileSize;
      fileRangeSize = fileSize - firstChunkOffset;
      if (options.Range.Value().Length.HasValue())
      {
        fileRangeSize = std::min(fileRangeSize, options.Range.Value().Length.Value());
      }
    }
    else
    {
      fileSize = firstChunk.Value.BodyStream->Length();
      fileRangeSize = fileSize;
    }
    firstChunkLength = std::min(firstChunkLength, fileRangeSize);

    auto bodyStreamToFile = [](Azure::Core::IO::BodyStream& stream,
                               _internal::FileWriter& fileWriter,
                               int64_t offset,
                               int64_t length,
                               const Azure::Core::Context& context) {
      constexpr size_t bufferSize = 4 * 1024 * 1024;
      std::vector<uint8_t> buffer(bufferSize);
      while (length > 0)
      {
        size_t readSize = static_cast<size_t>(std::min<int64_t>(bufferSize, length));
        size_t bytesRead = stream.ReadToCount(buffer.data(), readSize, context);
        if (bytesRead != readSize)
        {
          throw Azure::Core::RequestFailedException("Error when reading body stream.");
        }
        fileWriter.Write(buffer.data(), bytesRead, offset);
        length -= bytesRead;
        offset += bytesRead;
      }
    };

    bodyStreamToFile(*(firstChunk.Value.BodyStream), fileWriter, 0, firstChunkLength, context);
    firstChunk.Value.BodyStream.reset();

    auto returnTypeConverter = [](Azure::Response<Models::DownloadFileResult>& response) {
      Models::DownloadFileToResult ret;
      ret.FileSize = response.Value.FileSize;
      ret.HttpHeaders = std::move(response.Value.HttpHeaders);
      ret.Details = std::move(response.Value.Details);
      return Azure::Response<Models::DownloadFileToResult>(
          std::move(ret), std::move(response.RawResponse));
    };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadFileOptions chunkOptions;
            chunkOptions.Range = Core::Http::HttpRange();
            chunkOptions.Range.Value().Offset = offset;
            chunkOptions.Range.Value().Length = length;
            auto chunk = Download(chunkOptions, context);
            if (chunk.Value.Details.ETag != etag)
            {
              throw Azure::Core::RequestFailedException(
                  "File was modified in the middle of download.");
            }
            bodyStreamToFile(
                *(chunk.Value.BodyStream),
                fileWriter,
                offset - firstChunkOffset,
                chunkOptions.Range.Value().Length.Value(),
                context);

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = fileRangeSize - firstChunkLength;

    _internal::ConcurrentTransfer(
        remainingOffset,
        remainingSize,
        options.TransferOptions.ChunkSize,
        options.TransferOptions.Concurrency,
        downloadChunkFunc);
    ret.Value.ContentRange.Offset = firstChunkOffset;
    ret.Value.ContentRange.Length = fileRangeSize;
    return ret;
  }

  Azure::Response<Models::UploadFileFromResult> ShareFileClient::UploadFrom(
      const uint8_t* buffer,
      size_t bufferSize,
      const UploadFileFromOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::ShareRestClient::File::CreateOptions protocolLayerOptions;
    protocolLayerOptions.XMsContentLength = bufferSize;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.ToString();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission;
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
    }

    if (!options.HttpHeaders.ContentType.empty())
    {
      protocolLayerOptions.FileContentType = options.HttpHeaders.ContentType;
    }
    if (!options.HttpHeaders.ContentEncoding.empty())
    {
      protocolLayerOptions.FileContentEncoding = options.HttpHeaders.ContentEncoding;
    }
    if (!options.HttpHeaders.ContentLanguage.empty())
    {
      protocolLayerOptions.FileContentLanguage = options.HttpHeaders.ContentLanguage;
    }
    if (!options.HttpHeaders.CacheControl.empty())
    {
      protocolLayerOptions.FileCacheControl = options.HttpHeaders.CacheControl;
    }
    if (!options.HttpHeaders.ContentDisposition.empty())
    {
      protocolLayerOptions.FileContentDisposition = options.HttpHeaders.ContentDisposition;
    }
    if (!options.HttpHeaders.ContentHash.Value.empty())
    {
      AZURE_ASSERT_MSG(
          options.HttpHeaders.ContentHash.Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
      protocolLayerOptions.ContentMd5 = options.HttpHeaders.ContentHash;
    }
    protocolLayerOptions.Metadata = options.Metadata;
    auto createResult = _detail::ShareRestClient::File::Create(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      (void)chunkId;
      (void)numChunks;
      // TODO: Investigate changing lambda parameters to be size_t, unless they need to be int64_t
      // for some reason.
      Azure::Core::IO::MemoryBodyStream contentStream(buffer + offset, static_cast<size_t>(length));
      UploadFileRangeOptions uploadRangeOptions;
      UploadRange(offset, contentStream, uploadRangeOptions, context);
    };

    int64_t chunkSize = options.TransferOptions.ChunkSize;
    if (bufferSize < static_cast<size_t>(options.TransferOptions.SingleUploadThreshold))
    {
      chunkSize = bufferSize;
    }

    if (bufferSize > 0)
    {
      _internal::ConcurrentTransfer(
          0, bufferSize, chunkSize, options.TransferOptions.Concurrency, uploadPageFunc);
    }

    Models::UploadFileFromResult result;
    result.IsServerEncrypted = createResult.Value.IsServerEncrypted;
    return Azure::Response<Models::UploadFileFromResult>(
        std::move(result), std::move(createResult.RawResponse));
  }

  Azure::Response<Models::UploadFileFromResult> ShareFileClient::UploadFrom(
      const std::string& fileName,
      const UploadFileFromOptions& options,
      const Azure::Core::Context& context) const
  {
    _internal::FileReader fileReader(fileName);

    _detail::ShareRestClient::File::CreateOptions protocolLayerOptions;
    protocolLayerOptions.XMsContentLength = fileReader.GetFileSize();
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.ToString();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission;
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
    }

    if (!options.HttpHeaders.ContentType.empty())
    {
      protocolLayerOptions.FileContentType = options.HttpHeaders.ContentType;
    }
    if (!options.HttpHeaders.ContentEncoding.empty())
    {
      protocolLayerOptions.FileContentEncoding = options.HttpHeaders.ContentEncoding;
    }
    if (!options.HttpHeaders.ContentLanguage.empty())
    {
      protocolLayerOptions.FileContentLanguage = options.HttpHeaders.ContentLanguage;
    }
    if (!options.HttpHeaders.CacheControl.empty())
    {
      protocolLayerOptions.FileCacheControl = options.HttpHeaders.CacheControl;
    }
    if (!options.HttpHeaders.ContentDisposition.empty())
    {
      protocolLayerOptions.FileContentDisposition = options.HttpHeaders.ContentDisposition;
    }
    if (!options.HttpHeaders.ContentHash.Value.empty())
    {
      AZURE_ASSERT_MSG(
          options.HttpHeaders.ContentHash.Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
      protocolLayerOptions.ContentMd5 = options.HttpHeaders.ContentHash;
    }
    protocolLayerOptions.Metadata = options.Metadata;
    auto createResult = _detail::ShareRestClient::File::Create(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      (void)chunkId;
      (void)numChunks;
      Azure::Core::IO::_internal::RandomAccessFileBodyStream contentStream(
          fileReader.GetHandle(), offset, length);
      UploadFileRangeOptions uploadRangeOptions;
      UploadRange(offset, contentStream, uploadRangeOptions, context);
    };

    const int64_t fileSize = fileReader.GetFileSize();
    int64_t chunkSize = options.TransferOptions.ChunkSize;
    if (fileSize < options.TransferOptions.SingleUploadThreshold)
    {
      chunkSize = fileSize;
    }

    if (fileSize > 0)
    {
      _internal::ConcurrentTransfer(
          0, fileSize, chunkSize, options.TransferOptions.Concurrency, uploadPageFunc);
    }

    Models::UploadFileFromResult result;
    result.IsServerEncrypted = createResult.Value.IsServerEncrypted;
    return Azure::Response<Models::UploadFileFromResult>(
        std::move(result), std::move(createResult.RawResponse));
  }

  Azure::Response<Models::UploadFileRangeFromUriResult> ShareFileClient::UploadRangeFromUri(
      int64_t destinationOffset,
      const std::string& sourceUri,
      const Azure::Core::Http::HttpRange& sourceRange,
      const UploadFileRangeFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    AZURE_ASSERT_MSG(sourceRange.Length.HasValue(), "Source length cannot be null.");
    int64_t rangeLength = sourceRange.Length.Value();

    auto protocolLayerOptions = _detail::ShareRestClient::File::UploadRangeFromUrlOptions();
    protocolLayerOptions.TargetRange = std::string("bytes=") + std::to_string(destinationOffset)
        + std::string("-") + std::to_string(destinationOffset + rangeLength - 1);
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    if (options.TransactionalContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64,
          "This operation only supports CRC64 content hash.");
    }
    protocolLayerOptions.SourceContentCrc64 = options.TransactionalContentHash;
    if (options.SourceAccessCondition.IfMatchContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.SourceAccessCondition.IfMatchContentHash.Value().Algorithm
              == HashAlgorithm::Crc64,
          "This operation only supports CRC64 Source-If-Match condition.");
    }
    protocolLayerOptions.SourceIfMatchCrc64 = options.SourceAccessCondition.IfMatchContentHash;
    if (options.SourceAccessCondition.IfNoneMatchContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.SourceAccessCondition.IfNoneMatchContentHash.Value().Algorithm
              == HashAlgorithm::Crc64,
          "This operation only supports CRC64 Source-If-None-Match condition.");
    }
    protocolLayerOptions.SourceIfNoneMatchCrc64
        = options.SourceAccessCondition.IfNoneMatchContentHash;
    protocolLayerOptions.SourceRange = std::string("bytes=") + std::to_string(sourceRange.Offset)
        + std::string("-") + std::to_string(sourceRange.Offset + sourceRange.Length.Value() - 1);
    protocolLayerOptions.XMsWrite = _detail::FileRangeWriteFromUrl::Update;

    return _detail::ShareRestClient::File::UploadRangeFromUrl(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }
}}}} // namespace Azure::Storage::Files::Shares
