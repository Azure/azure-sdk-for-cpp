// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/datalake_protocol.hpp"

#include "generated/data_lake_client.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/exception.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <stdexcept>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {
  namespace {
    template <class Operation>
    auto InvokeStorageProtocol(Operation&& operation) -> decltype(operation())
    {
      try
      {
        return operation();
      }
      catch (StorageException const&)
      {
        throw;
      }
      catch (Core::RequestFailedException& error)
      {
        if (!error.RawResponse)
        {
          throw;
        }
        throw StorageException::CreateFromResponse(std::move(error.RawResponse));
      }
    }

    template <class T> T& Required(Nullable<T>& value, char const* name)
    {
      if (!value.HasValue())
      {
        throw std::out_of_range(std::string("Missing required response value: ") + name);
      }
      return value.Value();
    }

    template <class T> Nullable<T> NonEmpty(Nullable<T> const& value)
    {
      return value.HasValue() && !value.Value().empty() ? value : Nullable<T>{};
    }

    ETag NonEmpty(ETag const& value)
    {
      return value.HasValue() && !value.ToString().empty() ? value : ETag{};
    }

    template <class T> Nullable<T> EnumValue(Nullable<std::string> const& value)
    {
      return value.HasValue() && !value.Value().empty() ? Nullable<T>(T(value.Value()))
                                                        : Nullable<T>{};
    }

    template <class T, class U> Nullable<T> EnumValue(Nullable<U> const& value)
    {
      return value.HasValue() ? EnumValue<T>(Nullable<std::string>(value.Value().ToString()))
                              : Nullable<T>{};
    }

    Nullable<std::string> Encode(Nullable<std::vector<uint8_t>> const& value)
    {
      return value.HasValue() && !value.Value().empty()
          ? Nullable<std::string>(Core::Convert::Base64Encode(value.Value()))
          : Nullable<std::string>{};
    }

    Nullable<std::vector<uint8_t>> Decode(Nullable<std::string> const& value)
    {
      return value.HasValue()
          ? Nullable<std::vector<uint8_t>>(Core::Convert::Base64Decode(value.Value()))
          : Nullable<std::vector<uint8_t>>{};
    }

    template <class Target, class Source> void CopyConditions(Target& target, Source const& source)
    {
      target.XMsLeaseId = NonEmpty(source.LeaseId);
      target.MatchConditions.IfMatch = NonEmpty(source.IfMatch);
      target.MatchConditions.IfNoneMatch = NonEmpty(source.IfNoneMatch);
      target.IfModifiedSince = source.IfModifiedSince;
      target.IfUnmodifiedSince = source.IfUnmodifiedSince;
    }

    template <class Target, class Source> void CopyEncryption(Target& target, Source const& source)
    {
      target.XMsEncryptionKey = NonEmpty(source.EncryptionKey);
      target.XMsEncryptionKeySha256 = Encode(source.EncryptionKeySha256);
      target.XMsEncryptionAlgorithm
          = EnumValue<Models::EncryptionAlgorithmType>(source.EncryptionAlgorithm);
    }
  } // namespace

  Response<PublicModels::_detail::PathList> DataLakeProtocolClient::ListPaths(
      FileSystemClient::ListFileSystemPathsOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientListPathsFileSystemOptions generated;
    generated.XMsClientRequestId = NonEmpty(options.RequestId);
    generated.Timeout = options.Timeout;
    generated.Continuation = NonEmpty(options.ContinuationToken);
    generated.Directory = NonEmpty(options.Path);
    generated.MaxResults = options.MaxResults;
    generated.Upn = options.Upn;
    generated.BeginFrom = NonEmpty(options.BeginFrom);
    auto response = InvokeStorageProtocol([&] {
      return DataLakeClient(m_url, m_pipeline)
          .ListPathsFileSystem(options.Recursive, generated, context);
    });
    PublicModels::_detail::PathList result;
    result.ContinuationToken = std::move(response.Value.Continuation);
    if (response.Value.Paths.HasValue())
    {
      for (auto& path : response.Value.Paths.Value())
      {
        PublicModels::_detail::PathItem item;
        item.Name = std::move(Required(path.Name, "name"));
        item.IsDirectory = path.IsDirectory.HasValue() ? path.IsDirectory.Value() : false;
        item.LastModified = DateTime::Parse(
            Required(path.LastModified, "lastModified"), DateTime::DateFormat::Rfc1123);
        item.FileSize = Required(path.ContentLength, "contentLength");
        item.Owner = std::move(Required(path.Owner, "owner"));
        item.Group = std::move(Required(path.Group, "group"));
        item.Permissions = std::move(Required(path.Permissions, "permissions"));
        item.EncryptionScope = std::move(path.EncryptionScope);
        item.CreatedOn = std::move(path.CreationTime);
        item.ExpiresOn = std::move(path.ExpiryTime);
        item.EncryptionContext = std::move(path.EncryptionContext);
        if (path.ETag.HasValue())
        {
          item.ETag = path.ETag.ToString();
        }
        result.Paths.emplace_back(std::move(item));
      }
    }
    return Response<PublicModels::_detail::PathList>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::CreatePathResult> DataLakeProtocolClient::Create(
      PathClient::CreatePathOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientCreatePathOptions generated;
    generated.XMsClientRequestId = NonEmpty(options.RequestId);
    generated.Timeout = options.Timeout;
    generated.Resource = EnumValue<Models::PathResourceType>(options.Resource);
    generated.Continuation = NonEmpty(options.ContinuationToken);
    generated.Mode = EnumValue<Models::PathRenameMode>(options.Mode);
    generated.XMsCacheControl = NonEmpty(options.CacheControl);
    generated.XMsContentEncoding = NonEmpty(options.ContentEncoding);
    generated.XMsContentLanguage = NonEmpty(options.ContentLanguage);
    generated.XMsContentDisposition = NonEmpty(options.ContentDisposition);
    generated.XMsContentType = NonEmpty(options.ContentType);
    generated.XMsRenameSource = NonEmpty(options.RenameSource);
    generated.XMsSourceLeaseId = NonEmpty(options.SourceLeaseId);
    generated.XMsProperties = NonEmpty(options.Properties);
    generated.XMsPermissions = NonEmpty(options.Permissions);
    generated.XMsUmask = NonEmpty(options.Umask);
    CopyConditions(generated, options);
    if (NonEmpty(options.SourceIfMatch).HasValue())
    {
      generated.XMsSourceIfMatch = options.SourceIfMatch;
    }
    if (NonEmpty(options.SourceIfNoneMatch).HasValue())
    {
      generated.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    }
    generated.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generated.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    CopyEncryption(generated, options);
    generated.XMsOwner = NonEmpty(options.Owner);
    generated.XMsGroup = NonEmpty(options.Group);
    generated.XMsAcl = NonEmpty(options.Acl);
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    generated.XMsLeaseDuration = options.LeaseDuration;
    generated.XMsExpiryOption = EnumValue<Models::PathExpiryOptions>(options.ExpiryOptions);
    generated.XMsExpiryTime = NonEmpty(options.ExpiresOn);
    generated.XMsEncryptionContext = NonEmpty(options.EncryptionContext);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).CreatePath(generated, context); });
    PublicModels::CreatePathResult result;
    result.ETag = Required(response.Value.ETag, "ETag");
    result.LastModified = Required(response.Value.LastModified, "Last-Modified");
    result.FileSize = response.Value.ContentLength;
    result.IsServerEncrypted = response.Value.IsServerEncrypted.HasValue()
        ? response.Value.IsServerEncrypted.Value()
        : false;
    result.EncryptionKeySha256 = Decode(response.Value.EncryptionKeySha256);
    return Response<PublicModels::CreatePathResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::DeletePathResult> DataLakeProtocolClient::Delete(
      PathClient::DeletePathOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientDeletePathOptions generated;
    generated.XMsClientRequestId = NonEmpty(options.RequestId);
    generated.Timeout = options.Timeout;
    generated.Recursive = options.Recursive;
    generated.Continuation = NonEmpty(options.ContinuationToken);
    generated.Paginated = options.Paginated;
    CopyConditions(generated, options);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).DeletePath(generated, context); });
    return Response<PublicModels::DeletePathResult>(
        PublicModels::DeletePathResult{}, std::move(response.RawResponse));
  }

  Response<PublicModels::SetPathAccessControlListResult>
  DataLakeProtocolClient::SetAccessControlList(
      PathClient::SetPathAccessControlListOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientSetAccessControlPathOptions generated;
    CopyConditions(generated, options);
    generated.XMsOwner = NonEmpty(options.Owner);
    generated.XMsGroup = NonEmpty(options.Group);
    generated.XMsPermissions = NonEmpty(options.Permissions);
    generated.XMsAcl = NonEmpty(options.Acl);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).SetAccessControlPath(generated, context); });
    PublicModels::SetPathAccessControlListResult result;
    result.ETag = Required(response.Value.ETag, "ETag");
    result.LastModified = Required(response.Value.LastModified, "Last-Modified");
    return Response<PublicModels::SetPathAccessControlListResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::_detail::SetAccessControlListRecursiveResult>
  DataLakeProtocolClient::SetAccessControlListRecursive(
      PathClient::SetPathAccessControlListRecursiveOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientSetAccessControlRecursivePathOptions generated;
    generated.Continuation = NonEmpty(options.ContinuationToken);
    generated.ForceFlag = options.ForceFlag;
    generated.MaxRecords = options.MaxRecords;
    generated.XMsAcl = NonEmpty(options.Acl);
    auto response = InvokeStorageProtocol([&] {
      return DataLakeClient(m_url, m_pipeline)
          .SetAccessControlRecursivePath(
              Models::PathSetAccessControlRecursiveMode(options.Mode), generated, context);
    });
    PublicModels::_detail::SetAccessControlListRecursiveResult result;
    result.NumberOfSuccessfulDirectories
        = Required(response.Value.DirectoriesSuccessful, "directoriesSuccessful");
    result.NumberOfSuccessfulFiles = Required(response.Value.FilesSuccessful, "filesSuccessful");
    result.NumberOfFailures = Required(response.Value.FailureCount, "failureCount");
    result.ContinuationToken = std::move(response.Value.Continuation);
    if (response.Value.FailedEntries.HasValue())
    {
      for (auto& entry : response.Value.FailedEntries.Value())
      {
        PublicModels::AclFailedEntry item;
        item.Name = std::move(Required(entry.Name, "name"));
        item.Type = std::move(Required(entry.Type, "type"));
        item.ErrorMessage = std::move(Required(entry.ErrorMessage, "errorMessage"));
        result.FailedEntries.emplace_back(std::move(item));
      }
    }
    return Response<PublicModels::_detail::SetAccessControlListRecursiveResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::_detail::UndeletePathResult> DataLakeProtocolClient::Undelete(
      PathClient::UndeletePathOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientUndeletePathOptions generated;
    generated.XMsUndeleteSource = NonEmpty(options.UndeleteSource);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).UndeletePath(generated, context); });
    PublicModels::_detail::UndeletePathResult result;
    result.ResourceType = std::move(response.Value.ResourceType);
    return Response<PublicModels::_detail::UndeletePathResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::_detail::GetPathAccessControlListResult>
  DataLakeProtocolClient::GetAccessControlList(
      PathClient::GetPathAccessControlListOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientGetPropertiesPathOptions generated;
    generated.Action = Models::PathGetPropertiesAction::GetAccessControl;
    generated.Upn = options.Upn;
    CopyConditions(generated, options);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).GetPropertiesPath(generated, context); });
    PublicModels::_detail::GetPathAccessControlListResult result;
    result.Owner = std::move(Required(response.Value.Owner, "x-ms-owner"));
    result.Group = std::move(Required(response.Value.Group, "x-ms-group"));
    result.Permissions = std::move(Required(response.Value.Permissions, "x-ms-permissions"));
    result.Acl = std::move(Required(response.Value.Acl, "x-ms-acl"));
    return Response<PublicModels::_detail::GetPathAccessControlListResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::PathSystemProperties> DataLakeProtocolClient::GetSystemProperties(
      PathClient::GetPathSystemPropertiesOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientGetPropertiesPathOptions generated;
    generated.Action = Models::PathGetPropertiesAction::GetStatus;
    generated.Upn = options.Upn;
    CopyConditions(generated, options);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).GetPropertiesPath(generated, context); });
    PublicModels::PathSystemProperties result;
    result.FileSize = Required(response.Value.ContentLength, "Content-Length");
    result.ETag = Required(response.Value.ETag, "ETag");
    result.LastModified = Required(response.Value.LastModified, "Last-Modified");
    result.Owner = std::move(Required(response.Value.Owner, "x-ms-owner"));
    result.Group = std::move(Required(response.Value.Group, "x-ms-group"));
    result.Permissions = std::move(Required(response.Value.Permissions, "x-ms-permissions"));
    result.IsServerEncrypted = Required(response.Value.IsServerEncrypted, "x-ms-server-encrypted");
    result.EncryptionKeySha256 = Decode(response.Value.EncryptionKeySha256);
    result.EncryptionContext = std::move(response.Value.EncryptionContext);
    result.EncryptionScope = std::move(response.Value.EncryptionScope);
    result.CreatedOn = response.Value.CreationTime;
    result.ExpiresOn = response.Value.ExpiresOn;
    return Response<PublicModels::PathSystemProperties>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::FlushFileResult> DataLakeProtocolClient::Flush(
      FileClient::FlushFileOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientFlushDataPathOptions generated;
    generated.Position = options.Position;
    generated.RetainUncommittedData = options.RetainUncommittedData;
    generated.Close = options.Close;
    generated.ContentLength = 0;
    generated.XMsContentMd5 = NonEmpty(options.ContentMD5);
    generated.XMsLeaseAction = EnumValue<Models::LeaseAction>(options.LeaseAction);
    generated.XMsLeaseDuration = options.LeaseDuration;
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    generated.XMsCacheControl = NonEmpty(options.CacheControl);
    generated.XMsContentType = NonEmpty(options.ContentType);
    generated.XMsContentDisposition = NonEmpty(options.ContentDisposition);
    generated.XMsContentEncoding = NonEmpty(options.ContentEncoding);
    generated.XMsContentLanguage = NonEmpty(options.ContentLanguage);
    CopyConditions(generated, options);
    CopyEncryption(generated, options);
    auto response = InvokeStorageProtocol(
        [&] { return DataLakeClient(m_url, m_pipeline).FlushDataPath(generated, context); });
    PublicModels::FlushFileResult result;
    result.ETag = Required(response.Value.ETag, "ETag");
    result.LastModified = Required(response.Value.LastModified, "Last-Modified");
    result.FileSize = Required(response.Value.ContentLength, "Content-Length");
    result.IsServerEncrypted
        = Required(response.Value.IsServerEncrypted, "x-ms-request-server-encrypted");
    result.EncryptionKeySha256 = Decode(response.Value.EncryptionKeySha256);
    result.IsLeaseRenewed = response.Value.LeaseRenewed;
    return Response<PublicModels::FlushFileResult>(
        std::move(result), std::move(response.RawResponse));
  }

  Response<PublicModels::AppendFileResult> DataLakeProtocolClient::Append(
      Core::IO::BodyStream& content,
      FileClient::AppendFileOptions const& options,
      Core::Context const& context) const
  {
    DataLakeClientAppendDataPathOptions generated;
    generated.Position = options.Position;
    generated.ContentMd5 = NonEmpty(options.TransactionalContentHash);
    generated.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsLeaseAction = EnumValue<Models::LeaseAction>(options.LeaseAction);
    generated.XMsLeaseDuration = options.LeaseDuration;
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    CopyEncryption(generated, options);
    generated.Flush = options.Flush;
    generated.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generated.XMsStructuredContentLength = options.StructuredContentLength;
    auto response = InvokeStorageProtocol([&] {
      return DataLakeClient(m_url, m_pipeline).AppendDataPath(content, generated, context);
    });
    PublicModels::AppendFileResult result;
    if (response.Value.ContentMd5.HasValue())
    {
      result.TransactionalContentHash = ContentHash{};
      result.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      result.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(response.Value.ContentMd5.Value());
    }
    if (response.Value.ContentCrc64.HasValue())
    {
      result.TransactionalContentHash = ContentHash{};
      result.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      result.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(response.Value.ContentCrc64.Value());
    }
    result.IsServerEncrypted
        = Required(response.Value.IsServerEncrypted, "x-ms-request-server-encrypted");
    result.EncryptionKeySha256 = Decode(response.Value.EncryptionKeySha256);
    result.IsLeaseRenewed = response.Value.LeaseRenewed;
    return Response<PublicModels::AppendFileResult>(
        std::move(result), std::move(response.RawResponse));
  }

}}}}} // namespace Azure::Storage::Files::DataLake::_detail
