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
#include <azure/storage/common/storage_exception.hpp>

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
    auto protocolLayerOptions = _detail::FileClient::CreateFileOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
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
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
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
    protocolLayerOptions.FileContentLength = fileSize;
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
      protocolLayerOptions.FileContentMD5 = options.HttpHeaders.ContentHash.Value;
    }
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    auto result
        = _detail::FileClient::Create(*m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
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
    auto protocolLayerOptions = _detail::FileClient::DeleteFileOptions();
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    auto result
        = _detail::FileClient::Delete(*m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
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
    auto protocolLayerOptions = _detail::FileClient::DownloadFileOptions();
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
        protocolLayerOptions.RangeGetContentMD5 = true;
      }
    }
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;

    auto downloadResponse
        = _detail::FileClient::Download(*m_pipeline, m_shareFileUrl, protocolLayerOptions, context);

    {
      // In case network failure during reading the body
      auto eTag = downloadResponse.Value.Details.ETag;

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
    if (downloadResponse.RawResponse->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
    {
      downloadResponse.Value.FileSize = std::stoll(
          downloadResponse.RawResponse->GetHeaders().at(_internal::HttpHeaderContentLength));
      downloadResponse.Value.ContentRange.Offset = 0;
      downloadResponse.Value.ContentRange.Length = downloadResponse.Value.FileSize;
    }
    else if (
        downloadResponse.RawResponse->GetStatusCode()
        == Azure::Core::Http::HttpStatusCode::PartialContent)
    {
      const std::string& contentRange
          = downloadResponse.RawResponse->GetHeaders().at(_internal::HttpHeaderContentRange);
      auto bytes_pos = contentRange.find("bytes ");
      auto dash_pos = contentRange.find("-", bytes_pos + 6);
      auto slash_pos = contentRange.find("/", dash_pos + 1);
      const int64_t rangeStartOffset = std::stoll(
          std::string(contentRange.begin() + bytes_pos + 6, contentRange.begin() + dash_pos));
      const int64_t rangeEndOffset = std::stoll(
          std::string(contentRange.begin() + dash_pos + 1, contentRange.begin() + slash_pos));
      downloadResponse.Value.ContentRange
          = Azure::Core::Http::HttpRange{rangeStartOffset, rangeEndOffset - rangeStartOffset + 1};
      downloadResponse.Value.FileSize = std::stoll(contentRange.substr(slash_pos + 1));
    }
    return downloadResponse;
  }

  StartFileCopyOperation ShareFileClient::StartCopy(
      std::string copySource,
      const StartFileCopyOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::StartFileCopyOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    protocolLayerOptions.CopySource = std::move(copySource);
    if (options.SmbProperties.Attributes.GetValues().empty())
    {
      protocolLayerOptions.FileAttributes = FileCopySourceTime;
    }
    else
    {
      protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileCopySourceTime);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileCopySourceTime);
    }
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.PermissionCopyMode.HasValue())
    {
      protocolLayerOptions.FilePermissionCopyMode = options.PermissionCopyMode.Value();
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
      protocolLayerOptions.FilePermissionCopyMode = Models::PermissionCopyMode::Source;
    }
    protocolLayerOptions.IgnoreReadOnly = options.IgnoreReadOnly;
    protocolLayerOptions.SetArchiveAttribute = options.SetArchiveAttribute;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    auto response = _detail::FileClient::StartCopy(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);

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
    auto protocolLayerOptions = _detail::FileClient::AbortFileCopyOptions();
    protocolLayerOptions.CopyId = std::move(copyId);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::FileClient::AbortCopy(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::FileProperties> ShareFileClient::GetProperties(
      const GetFilePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::GetFilePropertiesOptions();
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::FileClient::GetProperties(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetFilePropertiesResult> ShareFileClient::SetProperties(
      const Models::FileHttpHeaders& httpHeaders,
      const Models::FileSmbProperties& smbProperties,
      const SetFilePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::SetFileHttpHeadersOptions();
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
    if (smbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = smbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    protocolLayerOptions.FileContentLength = options.Size;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
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

    return _detail::FileClient::SetHttpHeaders(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetFileMetadataResult> ShareFileClient::SetMetadata(
      Storage::Metadata metadata,
      const SetFileMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::SetFileMetadataOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(metadata.begin(), metadata.end());
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::FileClient::SetMetadata(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::UploadFileRangeResult> ShareFileClient::UploadRange(
      int64_t offset,
      Azure::Core::IO::BodyStream& content,
      const UploadFileRangeOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::UploadFileRangeOptions();
    protocolLayerOptions.FileRangeWrite = "update";
    protocolLayerOptions.Range = std::string("bytes=") + std::to_string(offset) + std::string("-")
        + std::to_string(offset + content.Length() - 1);
    if (options.TransactionalContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
      protocolLayerOptions.ContentMD5 = options.TransactionalContentHash.Value().Value;
    }
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.FileLastWrittenMode = options.FileLastWrittenMode;
    return _detail::FileClient::UploadRange(
        *m_pipeline, m_shareFileUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::ClearFileRangeResult> ShareFileClient::ClearRange(
      int64_t offset,
      int64_t length,
      const ClearFileRangeOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::UploadFileRangeOptions();
    protocolLayerOptions.FileRangeWrite = "clear";
    protocolLayerOptions.Range = std::string("bytes=") + std::to_string(offset) + std::string("-")
        + std::to_string(offset + length - 1);

    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.FileLastWrittenMode = options.FileLastWrittenMode;
    auto response = _detail::FileClient::UploadRange(
        *m_pipeline,
        m_shareFileUrl,
        *Azure::Core::IO::_internal::NullBodyStream::GetNullBodyStream(),
        protocolLayerOptions,
        context);
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
    auto protocolLayerOptions = _detail::FileClient::GetFileRangeListOptions();
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

    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::FileClient::GetRangeList(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::GetFileRangeListResult> ShareFileClient::GetRangeListDiff(
      std::string previousShareSnapshot,
      const GetFileRangeListOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::GetFileRangeListOptions();
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

    protocolLayerOptions.Prevsharesnapshot = std::move(previousShareSnapshot);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::FileClient::GetRangeList(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }

  ListFileHandlesPagedResponse ShareFileClient::ListHandles(
      const ListFileHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::ListFileHandlesOptions();
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto response = _detail::FileClient::ListHandles(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);

    ListFileHandlesPagedResponse pagedResponse;

    pagedResponse.FileHandles = std::move(response.Value.HandleList);
    pagedResponse.m_shareFileClient = std::make_shared<ShareFileClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    if (!response.Value.NextMarker.empty())
    {
      pagedResponse.NextPageToken = response.Value.NextMarker;
    }
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::ForceCloseFileHandleResult> ShareFileClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseFileHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::FileClient::ForceFileCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = _detail::FileClient::ForceCloseHandles(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
    return Azure::Response<Models::ForceCloseFileHandleResult>(
        Models::ForceCloseFileHandleResult(), std::move(result.RawResponse));
  }

  ForceCloseAllFileHandlesPagedResponse ShareFileClient::ForceCloseAllHandles(
      const ForceCloseAllFileHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::FileClient::ForceFileCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.Marker = options.ContinuationToken;
    auto response = _detail::FileClient::ForceCloseHandles(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);

    ForceCloseAllFileHandlesPagedResponse pagedResponse;

    pagedResponse.NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    pagedResponse.NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    pagedResponse.m_shareFileClient = std::make_shared<ShareFileClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
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

    _internal::FileWriter fileWriter(fileName);
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
    _detail::FileClient::CreateFileOptions protocolLayerOptions;
    protocolLayerOptions.FileContentLength = bufferSize;
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
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
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
      protocolLayerOptions.FileContentMD5 = options.HttpHeaders.ContentHash.Value;
    }
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    auto createResult
        = _detail::FileClient::Create(*m_pipeline, m_shareFileUrl, protocolLayerOptions, context);

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      (void)chunkId;
      (void)numChunks;
      // TODO: Investigate changing lambda parameters to be size_t, unless they need to be int64_t
      // for some reason.
      Azure::Core::IO::MemoryBodyStream contentStream(buffer + offset, static_cast<size_t>(length));
      UploadFileRangeOptions uploadRangeOptions;
      if (options.SmbProperties.LastWrittenOn.HasValue())
      {
        uploadRangeOptions.FileLastWrittenMode
            = Azure::Storage::Files::Shares::Models::FileLastWrittenMode::Preserve;
      }
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

    _detail::FileClient::CreateFileOptions protocolLayerOptions;
    protocolLayerOptions.FileContentLength = fileReader.GetFileSize();
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
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
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
      protocolLayerOptions.FileContentMD5 = options.HttpHeaders.ContentHash.Value;
    }
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    auto createResult
        = _detail::FileClient::Create(*m_pipeline, m_shareFileUrl, protocolLayerOptions, context);

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      (void)chunkId;
      (void)numChunks;
      Azure::Core::IO::_internal::RandomAccessFileBodyStream contentStream(
          fileReader.GetHandle(), offset, length);
      UploadFileRangeOptions uploadRangeOptions;
      if (options.SmbProperties.LastWrittenOn.HasValue())
      {
        uploadRangeOptions.FileLastWrittenMode
            = Azure::Storage::Files::Shares::Models::FileLastWrittenMode::Preserve;
      }
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

    auto protocolLayerOptions = _detail::FileClient::UploadFileRangeFromUriOptions();
    protocolLayerOptions.Range = std::string("bytes=") + std::to_string(destinationOffset)
        + std::string("-") + std::to_string(destinationOffset + rangeLength - 1);
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.FileLastWrittenMode = options.FileLastWrittenMode;
    if (options.TransactionalContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64,
          "This operation only supports CRC64 content hash.");
      protocolLayerOptions.SourceContentCrc64 = options.TransactionalContentHash.Value().Value;
    }
    if (options.SourceAccessCondition.IfMatchContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.SourceAccessCondition.IfMatchContentHash.Value().Algorithm
              == HashAlgorithm::Crc64,
          "This operation only supports CRC64 Source-If-Match condition.");
      protocolLayerOptions.SourceIfMatchCrc64
          = options.SourceAccessCondition.IfMatchContentHash.Value().Value;
    }
    if (options.SourceAccessCondition.IfNoneMatchContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.SourceAccessCondition.IfNoneMatchContentHash.Value().Algorithm
              == HashAlgorithm::Crc64,
          "This operation only supports CRC64 Source-If-None-Match condition.");
      protocolLayerOptions.SourceIfNoneMatchCrc64
          = options.SourceAccessCondition.IfNoneMatchContentHash.Value().Value;
    }
    protocolLayerOptions.SourceRange = std::string("bytes=") + std::to_string(sourceRange.Offset)
        + std::string("-") + std::to_string(sourceRange.Offset + sourceRange.Length.Value() - 1);

    return _detail::FileClient::UploadRangeFromUri(
        *m_pipeline, m_shareFileUrl, protocolLayerOptions, context);
  }
}}}} // namespace Azure::Storage::Files::Shares
