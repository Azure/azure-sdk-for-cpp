// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_file_client.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/storage/common/concurrent_transfer.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/reliable_stream.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareFileClient ShareFileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& fileName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileUrl = std::move(parsedConnectionString.FileServiceUrl);
    fileUrl.AppendPath(Storage::Details::UrlEncodePath(shareName));
    fileUrl.AppendPath(Storage::Details::UrlEncodePath(fileName));

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
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Storage::Details::SharedKeyPolicy>(credential),
            options));
  }

  ShareFileClient::ShareFileClient(
      const std::string& shareFileUrl,
      const ShareClientOptions& options)
      : m_shareFileUrl(shareFileUrl)
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            nullptr,
            options));
  }

  ShareFileClient ShareFileClient::WithShareSnapshot(const std::string& shareSnapshot) const
  {
    ShareFileClient newClient(*this);
    if (shareSnapshot.empty())
    {
      newClient.m_shareFileUrl.RemoveQueryParameter(Details::ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareFileUrl.AppendQueryParameter(
          Details::ShareSnapshotQueryParameter,
          Storage::Details::UrlEncodeQueryParameter(shareSnapshot));
    }
    return newClient;
  }

  Azure::Core::Response<Models::CreateShareFileResult> ShareFileClient::Create(
      int64_t fileSize,
      const CreateShareFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.Get();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.Get();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime
          = options.SmbProperties.CreatedOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
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
      if (options.HttpHeaders.ContentHash.Algorithm != HashAlgorithm::Md5)
      {
        std::abort();
      }
      protocolLayerOptions.ContentMd5 = options.HttpHeaders.ContentHash;
    }
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto result = Details::ShareRestClient::File::Create(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateShareFileResult ret;
    ret.Created = true;
    ret.ETag = std::move(result->ETag);
    ret.SmbProperties = std::move(result->SmbProperties);
    ret.IsServerEncrypted = result->IsServerEncrypted;
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);

    return Azure::Core::Response<Models::CreateShareFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteShareFileResult> ShareFileClient::Delete(
      const DeleteShareFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::DeleteOptions();
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto result = Details::ShareRestClient::File::Delete(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteShareFileResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteShareFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteShareFileResult> ShareFileClient::DeleteIfExists(
      const DeleteShareFileOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareNotFound || e.ErrorCode == Details::ParentNotFound
          || e.ErrorCode == Details::ResourceNotFound)
      {
        Models::DeleteShareFileResult ret;
        ret.Deleted = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Core::Response<Models::DeleteShareFileResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DownloadShareFileResult> ShareFileClient::Download(
      const DownloadShareFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::DownloadOptions();
    if (options.Range.HasValue())
    {
      if (options.Range.GetValue().Length.HasValue())
      {
        protocolLayerOptions.Range = std::string("bytes=")
            + std::to_string(options.Range.GetValue().Offset) + std::string("-")
            + std::to_string(options.Range.GetValue().Offset
                             + options.Range.GetValue().Length.GetValue() - 1);
      }
      else
      {
        protocolLayerOptions.Range = std::string("bytes=")
            + std::to_string(options.Range.GetValue().Offset) + std::string("-");
      }
    }
    if (options.RangeHashAlgorithm.HasValue())
    {
      if (options.RangeHashAlgorithm.GetValue() == HashAlgorithm::Md5)
      {
        protocolLayerOptions.GetRangeContentMd5 = true;
      }
      else
      {
        std::abort();
      }
    }
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;

    auto downloadResponse = Details::ShareRestClient::File::Download(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    {
      // In case network failure during reading the body
      auto eTag = downloadResponse->ETag;

      auto retryFunction
          = [this, options, eTag](
                const Azure::Core::Context& context,
                const HttpGetterInfo& retryInfo) -> std::unique_ptr<Azure::Core::Http::BodyStream> {
        DownloadShareFileOptions newOptions = options;
        newOptions.Range = Core::Http::Range();
        newOptions.Range.GetValue().Offset
            = (options.Range.HasValue() ? options.Range.GetValue().Offset : 0) + retryInfo.Offset;
        if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
        {
          newOptions.Range.GetValue().Length
              = options.Range.GetValue().Length.GetValue() - retryInfo.Offset;
        }

        auto newResponse = Download(newOptions, context);
        if (eTag != newResponse->Details.ETag)
        {
          throw Azure::Core::RequestFailedException(
              "File was changed during the download process.");
        }
        return std::move(newResponse->BodyStream);
      };

      ReliableStreamOptions reliableStreamOptions;
      reliableStreamOptions.MaxRetryRequests = Storage::Details::ReliableStreamRetryCount;
      downloadResponse->BodyStream = std::make_unique<ReliableStream>(
          std::move(downloadResponse->BodyStream), reliableStreamOptions, retryFunction);
    }
    Models::DownloadShareFileResult ret;
    ret.BodyStream = std::move(downloadResponse->BodyStream);
    ret.ContentRange = std::move(downloadResponse->ContentRange);
    ret.FileSize = downloadResponse->FileSize;
    ret.TransactionalContentHash = std::move(downloadResponse->TransactionalContentHash);
    ret.HttpHeaders = std::move(downloadResponse->HttpHeaders);
    ret.Details.LastModified = std::move(downloadResponse->LastModified);
    ret.Details.Metadata = std::move(downloadResponse->Metadata);
    ret.Details.ETag = std::move(downloadResponse->ETag);
    ret.Details.CopyCompletedOn = std::move(downloadResponse->CopyCompletedOn);
    ret.Details.CopyStatusDescription = std::move(downloadResponse->CopyStatusDescription);
    ret.Details.CopyId = std::move(downloadResponse->CopyId);
    ret.Details.CopyProgress = std::move(downloadResponse->CopyProgress);
    ret.Details.CopySource = std::move(downloadResponse->CopySource);
    ret.Details.CopyStatus = std::move(downloadResponse->CopyStatus);
    ret.Details.IsServerEncrypted = downloadResponse->IsServerEncrypted;
    ret.Details.SmbProperties = std::move(downloadResponse->SmbProperties);
    ret.Details.LeaseDuration = std::move(downloadResponse->LeaseDuration);
    ret.Details.LeaseState = std::move(downloadResponse->LeaseState);
    ret.Details.LeaseStatus = std::move(downloadResponse->LeaseStatus);
    return Azure::Core::Response<Models::DownloadShareFileResult>(
        std::move(ret), downloadResponse.ExtractRawResponse());
  }

  Azure::Core::Response<Models::StartCopyShareFileResult> ShareFileClient::StartCopy(
      std::string copySource,
      const StartCopyShareFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::StartCopyOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.CopySource = std::move(copySource);
    protocolLayerOptions.FileCopyFileAttributes = options.SmbProperties.Attributes.Get();
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCopyFileCreationTime
          = options.SmbProperties.CreatedOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCopyFileCreationTime = std::string(FileCopySourceTime);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileCopyFileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCopyFileLastWriteTime = std::string(FileCopySourceTime);
    }
    if (options.PermissionCopyMode.HasValue())
    {
      protocolLayerOptions.XMsFilePermissionCopyMode = options.PermissionCopyMode.GetValue();
      if (options.PermissionCopyMode.GetValue() == Models::PermissionCopyModeType::Override)
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
          // FilePermission or FilePermissionKey must be set if FilePermissionCopyMode is set to
          // PermissionCopyModeType::Override.
          std::abort();
        }
      }
    }
    else
    {
      protocolLayerOptions.XMsFilePermissionCopyMode = Models::PermissionCopyModeType::Source;
    }
    protocolLayerOptions.FileCopyIgnoreReadOnly = options.IgnoreReadOnly;
    protocolLayerOptions.FileCopySetArchiveAttribute = options.SetArchiveAttribute;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::ShareRestClient::File::StartCopy(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::AbortCopyShareFileResult> ShareFileClient::AbortCopy(
      std::string copyId,
      const AbortCopyShareFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::AbortCopyOptions();
    protocolLayerOptions.CopyId = std::move(copyId);
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::ShareRestClient::File::AbortCopy(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetShareFilePropertiesResult> ShareFileClient::GetProperties(
      const GetShareFilePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::GetPropertiesOptions();
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::ShareRestClient::File::GetProperties(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareFilePropertiesResult> ShareFileClient::SetProperties(
      const Models::FileHttpHeaders& httpHeaders,
      const Models::FileSmbProperties& smbProperties,
      const SetShareFilePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::SetHttpHeadersOptions();
    protocolLayerOptions.FileAttributes = smbProperties.Attributes.Get();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.Get();
    }
    if (smbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreatedOn.GetValue().GetRfc3339String(
          Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (smbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = smbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
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
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
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

    return Details::ShareRestClient::File::SetHttpHeaders(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareFileMetadataResult> ShareFileClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareFileMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::ShareRestClient::File::SetMetadata(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::UploadShareFileRangeResult> ShareFileClient::UploadRange(
      int64_t offset,
      Azure::Core::Http::BodyStream* content,
      const UploadShareFileRangeOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::UploadRangeOptions();
    protocolLayerOptions.XMsWrite = Models::FileRangeWriteType::Update;
    protocolLayerOptions.ContentLength = content->Length();
    protocolLayerOptions.XMsRange = std::string("bytes=") + std::to_string(offset)
        + std::string("-") + std::to_string(offset + content->Length() - 1);
    if (options.TransactionalContentHash.HasValue()
        && options.TransactionalContentHash.GetValue().Algorithm != HashAlgorithm::Md5)
    {
      std::abort();
    }
    protocolLayerOptions.ContentMd5 = options.TransactionalContentHash;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::ShareRestClient::File::UploadRange(
        m_shareFileUrl, *content, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ClearShareFileRangeResult> ShareFileClient::ClearRange(
      int64_t offset,
      int64_t length,
      const ClearShareFileRangeOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::UploadRangeOptions();
    protocolLayerOptions.XMsWrite = Models::FileRangeWriteType::Clear;
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.XMsRange = std::string("bytes=") + std::to_string(offset)
        + std::string("-") + std::to_string(offset + length - 1);

    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    auto response = Details::ShareRestClient::File::UploadRange(
        m_shareFileUrl,
        *Azure::Core::Http::NullBodyStream::GetNullBodyStream(),
        *m_pipeline,
        context,
        protocolLayerOptions);
    Models::ClearShareFileRangeResult ret;
    ret.ETag = std::move(response->ETag);
    ret.IsServerEncrypted = response->IsServerEncrypted;
    ret.LastModified = std::move(response->LastModified);
    ret.RequestId = std::move(response->RequestId);
    return Azure::Core::Response<Models::ClearShareFileRangeResult>(
        std::move(ret), response.ExtractRawResponse());
  }

  Azure::Core::Response<Models::GetShareFileRangeListResult> ShareFileClient::GetRangeList(
      const GetShareFileRangeListOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::GetRangeListOptions();
    if (options.Range.HasValue())
    {
      if (options.Range.GetValue().Length.HasValue())
      {
        protocolLayerOptions.XMsRange = std::string("bytes=")
            + std::to_string(options.Range.GetValue().Offset) + std::string("-")
            + std::to_string(options.Range.GetValue().Offset
                             + options.Range.GetValue().Length.GetValue() - 1);
      }
      else
      {
        protocolLayerOptions.XMsRange = std::string("bytes=")
            + std::to_string(options.Range.GetValue().Offset) + std::string("-");
      }
    }

    protocolLayerOptions.PrevShareSnapshot = options.PreviousShareSnapshot;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::ShareRestClient::File::GetRangeList(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListShareFileHandlesSinglePageResult>
  ShareFileClient::ListHandlesSinglePage(
      const ListShareFileHandlesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto result = Details::ShareRestClient::File::ListHandles(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ListShareFileHandlesSinglePageResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.Handles = std::move(result->HandleList);

    return Azure::Core::Response<Models::ListShareFileHandlesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseShareFileHandleResult> ShareFileClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseShareFileHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = Details::ShareRestClient::File::ForceCloseHandles(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
    return Azure::Core::Response<Models::ForceCloseShareFileHandleResult>(
        Models::ForceCloseShareFileHandleResult(), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseAllShareFileHandlesSinglePageResult>
  ShareFileClient::ForceCloseAllHandlesSinglePage(
      const ForceCloseAllShareFileHandlesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    return Details::ShareRestClient::File::ForceCloseHandles(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DownloadShareFileToResult> ShareFileClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadShareFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    // Just start downloading using an initial chunk. If it's a small file, we'll get the whole
    // thing in one shot. If it's a large file, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    int64_t firstChunkOffset = options.Range.HasValue() ? options.Range.GetValue().Offset : 0;
    int64_t firstChunkLength = options.TransferOptions.InitialChunkSize;

    if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Range.GetValue().Length.GetValue());
    }

    DownloadShareFileOptions firstChunkOptions;
    firstChunkOptions.Range = options.Range;
    if (firstChunkOptions.Range.HasValue())
    {
      firstChunkOptions.Range.GetValue().Length = firstChunkLength;
    }

    auto firstChunk = Download(firstChunkOptions, context);

    int64_t fileSize;
    int64_t fileRangeSize;
    if (firstChunkOptions.Range.HasValue())
    {
      fileSize = firstChunk->FileSize;
      fileRangeSize = fileSize - firstChunkOffset;
      if (options.Range.GetValue().Length.HasValue())
      {
        fileRangeSize = std::min(fileRangeSize, options.Range.GetValue().Length.GetValue());
      }
    }
    else
    {
      fileSize = firstChunk->BodyStream->Length();
      fileRangeSize = fileSize;
    }
    firstChunkLength = std::min(firstChunkLength, fileRangeSize);

    if (static_cast<std::size_t>(fileRangeSize) > bufferSize)
    {
      throw Azure::Core::RequestFailedException(
          "buffer is not big enough, file range size is " + std::to_string(fileRangeSize));
    }

    int64_t bytesRead = Azure::Core::Http::BodyStream::ReadToCount(
        context, *(firstChunk->BodyStream), buffer, firstChunkLength);
    if (bytesRead != firstChunkLength)
    {
      throw Azure::Core::RequestFailedException("error when reading body stream");
    }
    firstChunk->BodyStream.reset();

    auto returnTypeConverter
        = [](Azure::Core::Response<Models::DownloadShareFileResult>& response) {
            Models::DownloadShareFileToResult ret;
            ret.FileSize = response->FileSize;
            ret.HttpHeaders = std::move(response->HttpHeaders);
            ret.Details = std::move(response->Details);
            return Azure::Core::Response<Models::DownloadShareFileToResult>(
                std::move(ret), response.ExtractRawResponse());
          };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadShareFileOptions chunkOptions;
            chunkOptions.Range = Core::Http::Range();
            chunkOptions.Range.GetValue().Offset = offset;
            chunkOptions.Range.GetValue().Length = length;
            auto chunk = Download(chunkOptions, context);
            int64_t bytesRead = Azure::Core::Http::BodyStream::ReadToCount(
                context,
                *(chunk->BodyStream),
                buffer + (offset - firstChunkOffset),
                chunkOptions.Range.GetValue().Length.GetValue());
            if (bytesRead != chunkOptions.Range.GetValue().Length.GetValue())
            {
              throw Azure::Core::RequestFailedException("error when reading body stream");
            }

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = fileRangeSize - firstChunkLength;

    Storage::Details::ConcurrentTransfer(
        remainingOffset,
        remainingSize,
        options.TransferOptions.ChunkSize,
        options.TransferOptions.Concurrency,
        downloadChunkFunc);
    ret->ContentRange.Offset = firstChunkOffset;
    ret->ContentRange.Length = fileRangeSize;
    return ret;
  }

  Azure::Core::Response<Models::DownloadShareFileToResult> ShareFileClient::DownloadTo(
      const std::string& fileName,
      const DownloadShareFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    // Just start downloading using an initial chunk. If it's a small file, we'll get the whole
    // thing in one shot. If it's a large file, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    int64_t firstChunkOffset = options.Range.HasValue() ? options.Range.GetValue().Offset : 0;
    int64_t firstChunkLength = options.TransferOptions.InitialChunkSize;
    if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Range.GetValue().Length.GetValue());
    }

    DownloadShareFileOptions firstChunkOptions;
    firstChunkOptions.Range = options.Range;
    if (firstChunkOptions.Range.HasValue())
    {
      firstChunkOptions.Range.GetValue().Length = firstChunkLength;
    }

    Storage::Details::FileWriter fileWriter(fileName);

    auto firstChunk = Download(firstChunkOptions, context);

    int64_t fileSize;
    int64_t fileRangeSize;
    if (firstChunkOptions.Range.HasValue())
    {
      fileSize = firstChunk->FileSize;
      fileRangeSize = fileSize - firstChunkOffset;
      if (options.Range.GetValue().Length.HasValue())
      {
        fileRangeSize = std::min(fileRangeSize, options.Range.GetValue().Length.GetValue());
      }
    }
    else
    {
      fileSize = firstChunk->BodyStream->Length();
      fileRangeSize = fileSize;
    }
    firstChunkLength = std::min(firstChunkLength, fileRangeSize);

    auto bodyStreamToFile = [](Azure::Core::Http::BodyStream& stream,
                               Storage::Details::FileWriter& fileWriter,
                               int64_t offset,
                               int64_t length,
                               const Azure::Core::Context& context) {
      constexpr std::size_t bufferSize = 4 * 1024 * 1024;
      std::vector<uint8_t> buffer(bufferSize);
      while (length > 0)
      {
        int64_t readSize = std::min(static_cast<int64_t>(bufferSize), length);
        int64_t bytesRead
            = Azure::Core::Http::BodyStream::ReadToCount(context, stream, buffer.data(), readSize);
        if (bytesRead != readSize)
        {
          throw Azure::Core::RequestFailedException("error when reading body stream");
        }
        fileWriter.Write(buffer.data(), bytesRead, offset);
        length -= bytesRead;
        offset += bytesRead;
      }
    };

    bodyStreamToFile(*(firstChunk->BodyStream), fileWriter, 0, firstChunkLength, context);
    firstChunk->BodyStream.reset();

    auto returnTypeConverter
        = [](Azure::Core::Response<Models::DownloadShareFileResult>& response) {
            Models::DownloadShareFileToResult ret;
            ret.FileSize = response->FileSize;
            ret.HttpHeaders = std::move(response->HttpHeaders);
            ret.Details = std::move(response->Details);
            return Azure::Core::Response<Models::DownloadShareFileToResult>(
                std::move(ret), response.ExtractRawResponse());
          };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadShareFileOptions chunkOptions;
            chunkOptions.Range = Core::Http::Range();
            chunkOptions.Range.GetValue().Offset = offset;
            chunkOptions.Range.GetValue().Length = length;
            auto chunk = Download(chunkOptions, context);
            bodyStreamToFile(
                *(chunk->BodyStream),
                fileWriter,
                offset - firstChunkOffset,
                chunkOptions.Range.GetValue().Length.GetValue(),
                context);

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = fileRangeSize - firstChunkLength;

    Storage::Details::ConcurrentTransfer(
        remainingOffset,
        remainingSize,
        options.TransferOptions.ChunkSize,
        options.TransferOptions.Concurrency,
        downloadChunkFunc);
    ret->ContentRange.Offset = firstChunkOffset;
    ret->ContentRange.Length = fileRangeSize;
    return ret;
  }

  Azure::Core::Response<Models::UploadShareFileFromResult> ShareFileClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadShareFileFromOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::ShareRestClient::File::CreateOptions protocolLayerOptions;
    protocolLayerOptions.XMsContentLength = bufferSize;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.Get();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.Get();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime
          = options.SmbProperties.CreatedOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
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
      if (options.HttpHeaders.ContentHash.Algorithm != HashAlgorithm::Md5)
      {
        std::abort();
      }
      protocolLayerOptions.ContentMd5 = options.HttpHeaders.ContentHash;
    }
    protocolLayerOptions.Metadata = options.Metadata;
    auto createResult = Details::ShareRestClient::File::Create(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      (void)chunkId;
      (void)numChunks;
      Azure::Core::Http::MemoryBodyStream contentStream(buffer + offset, length);
      UploadShareFileRangeOptions uploadRangeOptions;
      UploadRange(offset, &contentStream, uploadRangeOptions, context);
    };

    int64_t chunkSize = options.TransferOptions.ChunkSize;
    if (bufferSize < static_cast<std::size_t>(options.TransferOptions.SingleUploadThreshold))
    {
      chunkSize = bufferSize;
    }

    if (bufferSize > 0)
    {
      Storage::Details::ConcurrentTransfer(
          0, bufferSize, chunkSize, options.TransferOptions.Concurrency, uploadPageFunc);
    }

    Models::UploadShareFileFromResult result;
    result.IsServerEncrypted = createResult->IsServerEncrypted;
    return Azure::Core::Response<Models::UploadShareFileFromResult>(
        std::move(result), createResult.ExtractRawResponse());
  }

  Azure::Core::Response<Models::UploadShareFileFromResult> ShareFileClient::UploadFrom(
      const std::string& fileName,
      const UploadShareFileFromOptions& options,
      const Azure::Core::Context& context) const
  {
    Storage::Details::FileReader fileReader(fileName);

    Details::ShareRestClient::File::CreateOptions protocolLayerOptions;
    protocolLayerOptions.XMsContentLength = fileReader.GetFileSize();
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.Get();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::None.Get();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime
          = options.SmbProperties.CreatedOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
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
      if (options.HttpHeaders.ContentHash.Algorithm != HashAlgorithm::Md5)
      {
        std::abort();
      }
      protocolLayerOptions.ContentMd5 = options.HttpHeaders.ContentHash;
    }
    protocolLayerOptions.Metadata = options.Metadata;
    auto createResult = Details::ShareRestClient::File::Create(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      (void)chunkId;
      (void)numChunks;
      Azure::Core::Http::FileBodyStream contentStream(fileReader.GetHandle(), offset, length);
      UploadShareFileRangeOptions uploadRangeOptions;
      UploadRange(offset, &contentStream, uploadRangeOptions, context);
    };

    const int64_t fileSize = fileReader.GetFileSize();
    int64_t chunkSize = options.TransferOptions.ChunkSize;
    if (fileSize < options.TransferOptions.SingleUploadThreshold)
    {
      chunkSize = fileSize;
    }

    if (fileSize > 0)
    {
      Storage::Details::ConcurrentTransfer(
          0, fileSize, chunkSize, options.TransferOptions.Concurrency, uploadPageFunc);
    }

    Models::UploadShareFileFromResult result;
    result.IsServerEncrypted = createResult->IsServerEncrypted;
    return Azure::Core::Response<Models::UploadShareFileFromResult>(
        std::move(result), createResult.ExtractRawResponse());
  }

  Azure::Core::Response<Models::UploadFileRangeFromUriResult> ShareFileClient::UploadRangeFromUri(
      int64_t destinationOffset,
      const std::string& sourceUri,
      const Azure::Core::Http::Range& sourceRange,
      const UploadFileRangeFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    if (!sourceRange.Length.HasValue())
    {
      // sourceRange must have length to perform this operation.
      std::abort();
    }
    int64_t rangeLength = sourceRange.Length.GetValue();

    auto protocolLayerOptions = Details::ShareRestClient::File::UploadRangeFromUrlOptions();
    protocolLayerOptions.TargetRange = std::string("bytes=") + std::to_string(destinationOffset)
        + std::string("-") + std::to_string(destinationOffset + rangeLength - 1);
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    if (options.TransactionalContentHash.HasValue()
        && options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Md5)
    {
      // SourceContentHash now only supports Crc64 hash algorithm.
      std::abort();
    }
    protocolLayerOptions.SourceContentCrc64 = options.TransactionalContentHash;
    if (options.SourceAccessCondition.IfMatchContentHash.HasValue()
        && options.SourceAccessCondition.IfMatchContentHash.GetValue().Algorithm
            == HashAlgorithm::Md5)
    {
      // IfMatchContentHash now only supports Crc64 hash algorithm.
      std::abort();
    }
    protocolLayerOptions.SourceIfMatchCrc64 = options.SourceAccessCondition.IfMatchContentHash;
    if (options.SourceAccessCondition.IfNoneMatchContentHash.HasValue()
        && options.SourceAccessCondition.IfNoneMatchContentHash.GetValue().Algorithm
            == HashAlgorithm::Md5)
    {
      // IfNoneMatchContentHash now only supports Crc64 hash algorithm.
      std::abort();
    }
    protocolLayerOptions.SourceIfNoneMatchCrc64
        = options.SourceAccessCondition.IfNoneMatchContentHash;
    protocolLayerOptions.SourceRange = std::string("bytes=") + std::to_string(sourceRange.Offset)
        + std::string("-") + std::to_string(sourceRange.Offset + sourceRange.Length.GetValue() - 1);
    protocolLayerOptions.XMsWrite = Models::FileRangeWriteFromUrlType::Update;

    return Details::ShareRestClient::File::UploadRangeFromUrl(
        m_shareFileUrl, *m_pipeline, context, protocolLayerOptions);
  }
}}}} // namespace Azure::Storage::Files::Shares
