
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/datalake/dll_import_export.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace Models {
    struct PathHttpHeaders
    {
      std::string CacheControl;
      std::string ContentDisposition;
      std::string ContentEncoding;
      std::string ContentLanguage;
      std::string ContentType;
      Storage::ContentHash ContentHash;
    };

    // The value must be "filesystem" for all filesystem operations.
    class FileSystemResourceType {
    public:
      FileSystemResourceType() = default;
      explicit FileSystemResourceType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const FileSystemResourceType& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const FileSystemResourceType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static FileSystemResourceType Filesystem;

    private:
      std::string m_value;
    }; // extensible enum FileSystemResourceType

    // Mode "set" sets POSIX access control rights on files and directories, "modify" modifies one
    // or more POSIX access control rights  that pre-exist on files and directories, "remove"
    // removes one or more POSIX access control rights  that were present earlier on files and
    // directories
    class PathSetAccessControlRecursiveMode {
    public:
      PathSetAccessControlRecursiveMode() = default;
      explicit PathSetAccessControlRecursiveMode(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathSetAccessControlRecursiveMode& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const PathSetAccessControlRecursiveMode& other) const
      {
        return !(*this == other);
      }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathSetAccessControlRecursiveMode Set;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathSetAccessControlRecursiveMode Modify;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathSetAccessControlRecursiveMode Remove;

    private:
      std::string m_value;
    }; // extensible enum PathSetAccessControlRecursiveMode

    // Required. Indicates mode of the expiry time
    class PathExpiryOptions {
    public:
      PathExpiryOptions() = default;
      explicit PathExpiryOptions(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathExpiryOptions& other) const { return m_value == other.m_value; }
      bool operator!=(const PathExpiryOptions& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathExpiryOptions NeverExpire;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathExpiryOptions RelativeToCreation;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathExpiryOptions RelativeToNow;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathExpiryOptions Absolute;

    private:
      std::string m_value;
    }; // extensible enum PathExpiryOptions

    struct AclFailedEntry
    {
      std::string Name;
      std::string Type;
      std::string ErrorMessage;
    };

    struct PathItem
    {
      std::string Name;
      bool IsDirectory = bool();
      Core::DateTime LastModified;
      std::string ETag;
      int64_t FileSize = int64_t();
      std::string Owner;
      std::string Group;
      std::string Permissions;
    };

    class PublicAccessType {
    public:
      PublicAccessType() = default;
      explicit PublicAccessType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PublicAccessType& other) const { return m_value == other.m_value; }
      bool operator!=(const PublicAccessType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PublicAccessType FileSystem;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PublicAccessType Path;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PublicAccessType None;

    private:
      std::string m_value;
    }; // extensible enum PublicAccessType

    // Required only for Create File and Create Directory. The value must be "file" or "directory".
    class PathResourceType {
    public:
      PathResourceType() = default;
      explicit PathResourceType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathResourceType& other) const { return m_value == other.m_value; }
      bool operator!=(const PathResourceType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathResourceType Directory;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathResourceType File;

    private:
      std::string m_value;
    }; // extensible enum PathResourceType

    // Optional. Valid only when namespace is enabled. This parameter determines the behavior of the
    // rename operation. The value must be "legacy" or "posix", and the default value will be
    // "posix".
    class PathRenameMode {
    public:
      PathRenameMode() = default;
      explicit PathRenameMode(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathRenameMode& other) const { return m_value == other.m_value; }
      bool operator!=(const PathRenameMode& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathRenameMode Legacy;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathRenameMode Posix;

    private:
      std::string m_value;
    }; // extensible enum PathRenameMode

    // Optional. If the value is "getStatus" only the system defined properties for the path are
    // returned. If the value is "getAccessControl" the access control list is returned in the
    // response headers (Hierarchical Namespace must be enabled for the account), otherwise the
    // properties are returned.
    class PathGetPropertiesAction {
    public:
      PathGetPropertiesAction() = default;
      explicit PathGetPropertiesAction(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathGetPropertiesAction& other) const
      {
        return m_value == other.m_value;
      }
      bool operator!=(const PathGetPropertiesAction& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathGetPropertiesAction GetAccessControl;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static PathGetPropertiesAction GetStatus;

    private:
      std::string m_value;
    }; // extensible enum PathGetPropertiesAction

    // When a resource is leased, specifies whether the lease is of infinite or fixed duration.
    class LeaseDurationType {
    public:
      LeaseDurationType() = default;
      explicit LeaseDurationType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseDurationType& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseDurationType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseDurationType Infinite;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseDurationType Fixed;

    private:
      std::string m_value;
    }; // extensible enum LeaseDurationType

    // Lease state of the resource.
    class LeaseStateType {
    public:
      LeaseStateType() = default;
      explicit LeaseStateType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseStateType& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseStateType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStateType Available;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStateType Leased;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStateType Expired;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStateType Breaking;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStateType Broken;

    private:
      std::string m_value;
    }; // extensible enum LeaseStateType

    // The lease status of the resource.
    class LeaseStatusType {
    public:
      LeaseStatusType() = default;
      explicit LeaseStatusType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseStatusType& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseStatusType& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStatusType Locked;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStatusType Unlocked;

    private:
      std::string m_value;
    }; // extensible enum LeaseStatusType

  } // namespace Models
  namespace Details {
    using namespace Models;
    constexpr static const char* DefaultServiceApiVersion = "2020-02-10";
    constexpr static const char* PathDnsSuffixDefault = "dfs.core.windows.net";
    constexpr static const char* QueryFileSystemResource = "resource";
    constexpr static const char* QueryTimeout = "timeout";
    constexpr static const char* QueryRecursive = "recursive";
    constexpr static const char* QueryContinuationToken = "continuation";
    constexpr static const char* QueryPathSetAccessControlRecursiveMode = "mode";
    constexpr static const char* QueryForceFlag = "forceflag";
    constexpr static const char* QueryPath = "directory";
    constexpr static const char* QueryPrefix = "prefix";
    constexpr static const char* QueryPageSizeHint = "maxresults";
    constexpr static const char* QueryUpn = "upn";
    constexpr static const char* QueryPosition = "position";
    constexpr static const char* QueryRetainUncommittedData = "retainuncommitteddata";
    constexpr static const char* QueryClose = "close";
    constexpr static const char* QueryPathResourceType = "resource";
    constexpr static const char* QueryPathRenameMode = "mode";
    constexpr static const char* QueryPathGetPropertiesAction = "action";
    constexpr static const char* QueryAction = "action";
    constexpr static const char* QueryMaxRecords = "maxrecords";
    constexpr static const char* HeaderVersion = "x-ms-version";
    constexpr static const char* HeaderIfMatch = "if-match";
    constexpr static const char* HeaderIfModifiedSince = "if-modified-since";
    constexpr static const char* HeaderIfNoneMatch = "if-none-match";
    constexpr static const char* HeaderIfUnmodifiedSince = "if-unmodified-since";
    constexpr static const char* HeaderLeaseId = "x-ms-lease-id";
    constexpr static const char* HeaderProposedLeaseId = "x-ms-proposed-lease-id";
    constexpr static const char* HeaderProperties = "x-ms-properties";
    constexpr static const char* HeaderSourceIfMatch = "x-ms-source-if-match";
    constexpr static const char* HeaderSourceIfModifiedSince = "x-ms-source-if-modified-since";
    constexpr static const char* HeaderSourceIfNoneMatch = "x-ms-source-if-none-match";
    constexpr static const char* HeaderSourceIfUnmodifiedSince = "x-ms-source-if-unmodified-since";
    constexpr static const char* HeaderSourceLeaseId = "x-ms-source-lease-id";
    constexpr static const char* HeaderCacheControl = "x-ms-cache-control";
    constexpr static const char* HeaderContentDisposition = "x-ms-content-disposition";
    constexpr static const char* HeaderContentEncoding = "x-ms-content-encoding";
    constexpr static const char* HeaderContentLanguage = "x-ms-content-language";
    constexpr static const char* HeaderContentType = "x-ms-content-type";
    constexpr static const char* HeaderTransactionalContentHashMd5 = "content-md5";
    constexpr static const char* HeaderContentHashMd5 = "x-ms-content-md5";
    constexpr static const char* HeaderTransactionalContentHashCrc64 = "x-ms-content-crc64";
    constexpr static const char* HeaderUmask = "x-ms-umask";
    constexpr static const char* HeaderPermissions = "x-ms-permissions";
    constexpr static const char* HeaderRenameSource = "x-ms-rename-source";
    constexpr static const char* HeaderOwner = "x-ms-owner";
    constexpr static const char* HeaderGroup = "x-ms-group";
    constexpr static const char* HeaderAcl = "x-ms-acl";
    constexpr static const char* HeaderContentLength = "content-length";
    constexpr static const char* HeaderExpiryOptions = "x-ms-expiry-option";
    constexpr static const char* HeaderExpiresOn = "x-ms-expiry-time";
    constexpr static const char* HeaderDate = "date";
    constexpr static const char* HeaderRequestId = "x-ms-request-id";
    constexpr static const char* HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderContinuationToken = "x-ms-continuation";
    constexpr static const char* HeaderErrorCode = "x-ms-error-code";
    constexpr static const char* HeaderETag = "etag";
    constexpr static const char* HeaderLastModified = "last-modified";
    constexpr static const char* HeaderAcceptRanges = "accept-ranges";
    constexpr static const char* HeaderResourceType = "x-ms-resource-type";
    constexpr static const char* HeaderLeaseDuration = "x-ms-lease-duration";
    constexpr static const char* HeaderLeaseState = "x-ms-lease-state";
    constexpr static const char* HeaderLeaseStatus = "x-ms-lease-status";
    constexpr static const char* HeaderRequestIsServerEncrypted = "x-ms-request-server-encrypted";

    struct SetAccessControlRecursiveResponse
    {
      int32_t NumberOfSuccessfulDirectories = int32_t();
      int32_t NumberOfSuccessfulFiles = int32_t();
      int32_t NumberOfFailures = int32_t();
      std::vector<AclFailedEntry> FailedEntries;
    };

    struct PathList
    {
      std::vector<PathItem> Items;
    };

    struct FileSystemListPathsResult
    {
      std::vector<PathItem> Items;
      std::string RequestId;
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct PathCreateResult
    {
      Core::ETag ETag;
      Azure::Core::Nullable<Core::DateTime> LastModified;
      std::string RequestId;
      Azure::Core::Nullable<int64_t> ContentLength;
    };

    struct PathGetPropertiesResult
    {
      Azure::Core::Nullable<std::string> AcceptRanges;
      PathHttpHeaders HttpHeaders;
      Core::ETag ETag;
      Core::DateTime LastModified;
      std::string RequestId;
      Azure::Core::Nullable<std::string> ResourceType;
      Azure::Core::Nullable<std::string> Properties;
      Azure::Core::Nullable<std::string> Owner;
      Azure::Core::Nullable<std::string> Group;
      Azure::Core::Nullable<std::string> Permissions;
      Azure::Core::Nullable<std::string> Acl;
      Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
      Azure::Core::Nullable<LeaseStateType> LeaseState;
      Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct PathDeleteResult
    {
      std::string RequestId;
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct PathSetAccessControlResult
    {
      Core::ETag ETag;
      Core::DateTime LastModified;
      std::string RequestId;
    };

    struct PathSetAccessControlRecursiveResult
    {
      int32_t NumberOfSuccessfulDirectories = int32_t();
      int32_t NumberOfSuccessfulFiles = int32_t();
      int32_t NumberOfFailures = int32_t();
      std::vector<AclFailedEntry> FailedEntries;
      Azure::Core::Nullable<std::string> ContinuationToken;
      std::string RequestId;
    };

    struct PathFlushDataResult
    {
      Core::ETag ETag;
      Core::DateTime LastModified;
      int64_t ContentLength = int64_t();
      std::string RequestId;
    };

    struct PathAppendDataResult
    {
      std::string RequestId;
      Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
      bool IsServerEncrypted = bool();
    };

    class DataLakeRestClient {
    public:
      class FileSystem {
      public:
        struct ListPathsOptions
        {
          FileSystemResourceType Resource;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<std::string> Directory;
          bool RecursiveRequired = bool();
          Azure::Core::Nullable<int32_t> MaxResults;
          Azure::Core::Nullable<bool> Upn;
        };

        static Azure::Response<FileSystemListPathsResult> ListPaths(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListPathsOptions& listPathsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter((listPathsOptions.Resource.ToString())));
          if (listPathsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listPathsOptions.Timeout.GetValue())));
          }
          request.SetHeader(Details::HeaderVersion, listPathsOptions.ApiVersionParameter);
          if (listPathsOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listPathsOptions.ContinuationToken.GetValue()));
          }
          if (listPathsOptions.Directory.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPath,
                Storage::Details::UrlEncodeQueryParameter(listPathsOptions.Directory.GetValue()));
          }
          request.GetUrl().AppendQueryParameter(
              Details::QueryRecursive,
              Storage::Details::UrlEncodeQueryParameter(
                  (listPathsOptions.RecursiveRequired ? "true" : "false")));
          if (listPathsOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPageSizeHint,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listPathsOptions.MaxResults.GetValue())));
          }
          if (listPathsOptions.Upn.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryUpn,
                Storage::Details::UrlEncodeQueryParameter(
                    (listPathsOptions.Upn.GetValue() ? "true" : "false")));
          }
          return ListPathsParseResult(context, pipeline.Send(request, context));
        }

      private:
        static Azure::Response<FileSystemListPathsResult> ListPathsParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            const auto& bodyBuffer = response.GetBody();
            FileSystemListPathsResult result = bodyBuffer.empty()
                ? FileSystemListPathsResult()
                : FileSystemListPathsResultFromPathList(
                    PathListFromJson(Azure::Core::Internal::Json::json::parse(bodyBuffer)));
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            return Azure::Response<FileSystemListPathsResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static PathItem PathItemFromJson(const Azure::Core::Internal::Json::json& node)
        {
          PathItem result;
          result.Name = node["name"].get<std::string>();
          if (node.contains("isDirectory"))
          {
            result.IsDirectory = (node["isDirectory"].get<std::string>() == "true");
          }
          result.LastModified = Core::DateTime::Parse(
              node["lastModified"].get<std::string>(), Core::DateTime::DateFormat::Rfc1123);
          result.ETag = node["etag"].get<std::string>();
          if (node.contains("contentLength"))
          {
            result.FileSize = std::stoll(node["contentLength"].get<std::string>());
          }
          result.Owner = node["owner"].get<std::string>();
          result.Group = node["group"].get<std::string>();
          result.Permissions = node["permissions"].get<std::string>();
          return result;
        }

        static PathList PathListFromJson(const Azure::Core::Internal::Json::json& node)
        {
          PathList result;
          for (const auto& element : node["paths"])
          {
            result.Items.emplace_back(PathItemFromJson(element));
          }
          return result;
        }

        static FileSystemListPathsResult FileSystemListPathsResultFromPathList(PathList object)
        {
          FileSystemListPathsResult result;
          result.Items = std::move(object.Items);

          return result;
        }
      };

      class Path {
      public:
        struct CreateOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<PathResourceType> Resource;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<PathRenameMode> Mode;
          Azure::Core::Nullable<std::string> CacheControl;
          Azure::Core::Nullable<std::string> ContentEncoding;
          Azure::Core::Nullable<std::string> ContentLanguage;
          Azure::Core::Nullable<std::string> ContentDisposition;
          Azure::Core::Nullable<std::string> ContentType;
          Azure::Core::Nullable<std::string> RenameSource;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> SourceLeaseId;
          Azure::Core::Nullable<std::string> Properties;
          Azure::Core::Nullable<std::string> Permissions;
          Azure::Core::Nullable<std::string> Umask;
          Core::ETag IfMatch;
          Core::ETag IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
          Core::ETag SourceIfMatch;
          Core::ETag SourceIfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> SourceIfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> SourceIfUnmodifiedSince;
        };

        static Azure::Response<PathCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(Details::HeaderContentLength, "0");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.SetHeader(Details::HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.Resource.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathResourceType,
                Storage::Details::UrlEncodeQueryParameter(
                    (createOptions.Resource.GetValue().ToString())));
          }
          if (createOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    createOptions.ContinuationToken.GetValue()));
          }
          if (createOptions.Mode.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathRenameMode,
                Storage::Details::UrlEncodeQueryParameter(
                    (createOptions.Mode.GetValue().ToString())));
          }
          if (createOptions.CacheControl.HasValue())
          {
            request.SetHeader(Details::HeaderCacheControl, createOptions.CacheControl.GetValue());
          }
          if (createOptions.ContentEncoding.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentEncoding, createOptions.ContentEncoding.GetValue());
          }
          if (createOptions.ContentLanguage.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentLanguage, createOptions.ContentLanguage.GetValue());
          }
          if (createOptions.ContentDisposition.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentDisposition, createOptions.ContentDisposition.GetValue());
          }
          if (createOptions.ContentType.HasValue())
          {
            request.SetHeader(Details::HeaderContentType, createOptions.ContentType.GetValue());
          }
          if (createOptions.RenameSource.HasValue())
          {
            request.SetHeader(Details::HeaderRenameSource, createOptions.RenameSource.GetValue());
          }
          if (createOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(Details::HeaderLeaseId, createOptions.LeaseIdOptional.GetValue());
          }
          if (createOptions.SourceLeaseId.HasValue())
          {
            request.SetHeader(Details::HeaderSourceLeaseId, createOptions.SourceLeaseId.GetValue());
          }
          if (createOptions.Properties.HasValue())
          {
            request.SetHeader(Details::HeaderProperties, createOptions.Properties.GetValue());
          }
          if (createOptions.Permissions.HasValue())
          {
            request.SetHeader(Details::HeaderPermissions, createOptions.Permissions.GetValue());
          }
          if (createOptions.Umask.HasValue())
          {
            request.SetHeader(Details::HeaderUmask, createOptions.Umask.GetValue());
          }
          if (createOptions.IfMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfMatch, createOptions.IfMatch.ToString());
          }
          if (createOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfNoneMatch, createOptions.IfNoneMatch.ToString());
          }
          if (createOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfModifiedSince,
                createOptions.IfModifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfUnmodifiedSince,
                createOptions.IfUnmodifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.SourceIfMatch.HasValue())
          {
            request.SetHeader(Details::HeaderSourceIfMatch, createOptions.SourceIfMatch.ToString());
          }
          if (createOptions.SourceIfNoneMatch.HasValue())
          {
            request.SetHeader(
                Details::HeaderSourceIfNoneMatch, createOptions.SourceIfNoneMatch.ToString());
          }
          if (createOptions.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderSourceIfModifiedSince,
                createOptions.SourceIfModifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderSourceIfUnmodifiedSince,
                createOptions.SourceIfUnmodifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return CreateParseResult(context, pipeline.Send(request, context));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<PathGetPropertiesAction> Action;
          Azure::Core::Nullable<bool> Upn;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Core::ETag IfMatch;
          Core::ETag IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Response<PathGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.SetHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.Action.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathGetPropertiesAction,
                Storage::Details::UrlEncodeQueryParameter(
                    (getPropertiesOptions.Action.GetValue().ToString())));
          }
          if (getPropertiesOptions.Upn.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryUpn,
                Storage::Details::UrlEncodeQueryParameter(
                    (getPropertiesOptions.Upn.GetValue() ? "true" : "false")));
          }
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                Details::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.GetValue());
          }
          if (getPropertiesOptions.IfMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfMatch, getPropertiesOptions.IfMatch.ToString());
          }
          if (getPropertiesOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfNoneMatch, getPropertiesOptions.IfNoneMatch.ToString());
          }
          if (getPropertiesOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfModifiedSince,
                getPropertiesOptions.IfModifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (getPropertiesOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfUnmodifiedSince,
                getPropertiesOptions.IfUnmodifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return GetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<bool> RecursiveOptional;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Core::ETag IfMatch;
          Core::ETag IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Response<PathDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.SetHeader(Details::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.RecursiveOptional.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryRecursive,
                Storage::Details::UrlEncodeQueryParameter(
                    (deleteOptions.RecursiveOptional.GetValue() ? "true" : "false")));
          }
          if (deleteOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    deleteOptions.ContinuationToken.GetValue()));
          }
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(Details::HeaderLeaseId, deleteOptions.LeaseIdOptional.GetValue());
          }
          if (deleteOptions.IfMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfMatch, deleteOptions.IfMatch.ToString());
          }
          if (deleteOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfNoneMatch, deleteOptions.IfNoneMatch.ToString());
          }
          if (deleteOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfModifiedSince,
                deleteOptions.IfModifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (deleteOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfUnmodifiedSince,
                deleteOptions.IfUnmodifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return DeleteParseResult(context, pipeline.Send(request, context));
        }

        struct SetAccessControlOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> Owner;
          Azure::Core::Nullable<std::string> Group;
          Azure::Core::Nullable<std::string> Permissions;
          Azure::Core::Nullable<std::string> Acl;
          Core::ETag IfMatch;
          Core::ETag IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Response<PathSetAccessControlResult> SetAccessControl(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetAccessControlOptions& setAccessControlOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(Details::QueryAction, "setAccessControl");
          if (setAccessControlOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setAccessControlOptions.Timeout.GetValue())));
          }
          if (setAccessControlOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                Details::HeaderLeaseId, setAccessControlOptions.LeaseIdOptional.GetValue());
          }
          if (setAccessControlOptions.Owner.HasValue())
          {
            request.SetHeader(Details::HeaderOwner, setAccessControlOptions.Owner.GetValue());
          }
          if (setAccessControlOptions.Group.HasValue())
          {
            request.SetHeader(Details::HeaderGroup, setAccessControlOptions.Group.GetValue());
          }
          if (setAccessControlOptions.Permissions.HasValue())
          {
            request.SetHeader(
                Details::HeaderPermissions, setAccessControlOptions.Permissions.GetValue());
          }
          if (setAccessControlOptions.Acl.HasValue())
          {
            request.SetHeader(Details::HeaderAcl, setAccessControlOptions.Acl.GetValue());
          }
          if (setAccessControlOptions.IfMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfMatch, setAccessControlOptions.IfMatch.ToString());
          }
          if (setAccessControlOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfNoneMatch, setAccessControlOptions.IfNoneMatch.ToString());
          }
          if (setAccessControlOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfModifiedSince,
                setAccessControlOptions.IfModifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (setAccessControlOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfUnmodifiedSince,
                setAccessControlOptions.IfUnmodifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          request.SetHeader(Details::HeaderVersion, setAccessControlOptions.ApiVersionParameter);
          return SetAccessControlParseResult(context, pipeline.Send(request, context));
        }

        struct SetAccessControlRecursiveOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<std::string> ContinuationToken;
          PathSetAccessControlRecursiveMode Mode;
          Azure::Core::Nullable<bool> ForceFlag;
          Azure::Core::Nullable<int32_t> MaxRecords;
          Azure::Core::Nullable<std::string> Acl;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Response<PathSetAccessControlRecursiveResult> SetAccessControlRecursive(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetAccessControlRecursiveOptions& setAccessControlRecursiveOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(Details::QueryAction, "setAccessControlRecursive");
          if (setAccessControlRecursiveOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setAccessControlRecursiveOptions.Timeout.GetValue())));
          }
          if (setAccessControlRecursiveOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    setAccessControlRecursiveOptions.ContinuationToken.GetValue()));
          }
          request.GetUrl().AppendQueryParameter(
              Details::QueryPathSetAccessControlRecursiveMode,
              Storage::Details::UrlEncodeQueryParameter(
                  (setAccessControlRecursiveOptions.Mode.ToString())));
          if (setAccessControlRecursiveOptions.ForceFlag.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryForceFlag,
                Storage::Details::UrlEncodeQueryParameter(
                    (setAccessControlRecursiveOptions.ForceFlag.GetValue() ? "true" : "false")));
          }
          if (setAccessControlRecursiveOptions.MaxRecords.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryMaxRecords,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setAccessControlRecursiveOptions.MaxRecords.GetValue())));
          }
          if (setAccessControlRecursiveOptions.Acl.HasValue())
          {
            request.SetHeader(Details::HeaderAcl, setAccessControlRecursiveOptions.Acl.GetValue());
          }
          request.SetHeader(
              Details::HeaderVersion, setAccessControlRecursiveOptions.ApiVersionParameter);
          return SetAccessControlRecursiveParseResult(context, pipeline.Send(request, context));
        }

        struct FlushDataOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<int64_t> Position;
          Azure::Core::Nullable<bool> RetainUncommittedData;
          Azure::Core::Nullable<bool> Close;
          Azure::Core::Nullable<int64_t> ContentLength;
          Azure::Core::Nullable<Storage::ContentHash> ContentMd5;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> CacheControl;
          Azure::Core::Nullable<std::string> ContentType;
          Azure::Core::Nullable<std::string> ContentDisposition;
          Azure::Core::Nullable<std::string> ContentEncoding;
          Azure::Core::Nullable<std::string> ContentLanguage;
          Core::ETag IfMatch;
          Core::ETag IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Response<PathFlushDataResult> FlushData(
            const Azure::Core::Http::Url& url,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const FlushDataOptions& flushDataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(Details::QueryAction, "flush");
          if (flushDataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(flushDataOptions.Timeout.GetValue())));
          }
          if (flushDataOptions.Position.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPosition,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(flushDataOptions.Position.GetValue())));
          }
          if (flushDataOptions.RetainUncommittedData.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryRetainUncommittedData,
                Storage::Details::UrlEncodeQueryParameter(
                    (flushDataOptions.RetainUncommittedData.GetValue() ? "true" : "false")));
          }
          if (flushDataOptions.Close.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryClose,
                Storage::Details::UrlEncodeQueryParameter(
                    (flushDataOptions.Close.GetValue() ? "true" : "false")));
          }
          if (flushDataOptions.ContentLength.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentLength,
                std::to_string(flushDataOptions.ContentLength.GetValue()));
          }
          if (flushDataOptions.ContentMd5.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentHashMd5,
                Storage::Details::ToBase64String(flushDataOptions.ContentMd5.GetValue()));
          }
          if (flushDataOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(Details::HeaderLeaseId, flushDataOptions.LeaseIdOptional.GetValue());
          }
          if (flushDataOptions.CacheControl.HasValue())
          {
            request.SetHeader(
                Details::HeaderCacheControl, flushDataOptions.CacheControl.GetValue());
          }
          if (flushDataOptions.ContentType.HasValue())
          {
            request.SetHeader(Details::HeaderContentType, flushDataOptions.ContentType.GetValue());
          }
          if (flushDataOptions.ContentDisposition.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentDisposition, flushDataOptions.ContentDisposition.GetValue());
          }
          if (flushDataOptions.ContentEncoding.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentEncoding, flushDataOptions.ContentEncoding.GetValue());
          }
          if (flushDataOptions.ContentLanguage.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentLanguage, flushDataOptions.ContentLanguage.GetValue());
          }
          if (flushDataOptions.IfMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfMatch, flushDataOptions.IfMatch.ToString());
          }
          if (flushDataOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(Details::HeaderIfNoneMatch, flushDataOptions.IfNoneMatch.ToString());
          }
          if (flushDataOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfModifiedSince,
                flushDataOptions.IfModifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (flushDataOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                Details::HeaderIfUnmodifiedSince,
                flushDataOptions.IfUnmodifiedSince.GetValue().ToString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          request.SetHeader(Details::HeaderVersion, flushDataOptions.ApiVersionParameter);
          return FlushDataParseResult(context, pipeline.Send(request, context));
        }

        struct AppendDataOptions
        {
          Azure::Core::Nullable<int64_t> Position;
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<int64_t> ContentLength;
          Azure::Core::Nullable<Storage::ContentHash> TransactionalContentMd5;
          Azure::Core::Nullable<Storage::ContentHash> TransactionalContentCrc64;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Response<PathAppendDataResult> AppendData(
            const Azure::Core::Http::Url& url,
            Azure::IO::BodyStream& bodyStream,
            Azure::Core::Internal::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AppendDataOptions& appendDataOptions)
        {
          Azure::Core::Http::Request request(
              Azure::Core::Http::HttpMethod::Patch, url, &bodyStream);
          request.GetUrl().AppendQueryParameter(Details::QueryAction, "append");
          if (appendDataOptions.Position.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPosition,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(appendDataOptions.Position.GetValue())));
          }
          if (appendDataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(appendDataOptions.Timeout.GetValue())));
          }
          if (appendDataOptions.ContentLength.HasValue())
          {
            request.SetHeader(
                Details::HeaderContentLength,
                std::to_string(appendDataOptions.ContentLength.GetValue()));
          }
          if (appendDataOptions.TransactionalContentMd5.HasValue())
          {
            request.SetHeader(
                Details::HeaderTransactionalContentHashMd5,
                Storage::Details::ToBase64String(
                    appendDataOptions.TransactionalContentMd5.GetValue()));
          }
          if (appendDataOptions.TransactionalContentCrc64.HasValue())
          {
            request.SetHeader(
                Details::HeaderTransactionalContentHashCrc64,
                Storage::Details::ToBase64String(
                    appendDataOptions.TransactionalContentCrc64.GetValue()));
          }
          if (appendDataOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(Details::HeaderLeaseId, appendDataOptions.LeaseIdOptional.GetValue());
          }
          request.SetHeader(Details::HeaderVersion, appendDataOptions.ApiVersionParameter);
          return AppendDataParseResult(context, pipeline.Send(request, context));
        }

      private:
        static Azure::Response<PathCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // The file or directory was created.
            PathCreateResult result;
            if (response.GetHeaders().find(Details::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Core::ETag(response.GetHeaders().at(Details::HeaderETag));
            }
            if (response.GetHeaders().find(Details::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderLastModified),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            if (response.GetHeaders().find(Details::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.ContentLength
                  = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            }
            return Azure::Response<PathCreateResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<PathGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Returns all properties for the file or directory.
            PathGetPropertiesResult result;
            if (response.GetHeaders().find(Details::HeaderAcceptRanges)
                != response.GetHeaders().end())
            {
              result.AcceptRanges = response.GetHeaders().at(Details::HeaderAcceptRanges);
            }
            if (response.GetHeaders().find("cache-control") != response.GetHeaders().end())
            {
              result.HttpHeaders.CacheControl = response.GetHeaders().at("cache-control");
            }
            if (response.GetHeaders().find("content-disposition") != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentDisposition
                  = response.GetHeaders().at("content-disposition");
            }
            if (response.GetHeaders().find("content-encoding") != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentEncoding = response.GetHeaders().at("content-encoding");
            }
            if (response.GetHeaders().find("content-language") != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentLanguage = response.GetHeaders().at("content-language");
            }
            if (response.GetHeaders().find("content-type") != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentType = response.GetHeaders().at("content-type");
            }
            if (response.GetHeaders().find(Details::HeaderContentHashMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentHashMd5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Core::ETag(response.GetHeaders().at(Details::HeaderETag));
            }
            if (response.GetHeaders().find(Details::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderLastModified),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            if (response.GetHeaders().find(Details::HeaderResourceType)
                != response.GetHeaders().end())
            {
              result.ResourceType = response.GetHeaders().at(Details::HeaderResourceType);
            }
            if (response.GetHeaders().find(Details::HeaderProperties)
                != response.GetHeaders().end())
            {
              result.Properties = response.GetHeaders().at(Details::HeaderProperties);
            }
            if (response.GetHeaders().find(Details::HeaderOwner) != response.GetHeaders().end())
            {
              result.Owner = response.GetHeaders().at(Details::HeaderOwner);
            }
            if (response.GetHeaders().find(Details::HeaderGroup) != response.GetHeaders().end())
            {
              result.Group = response.GetHeaders().at(Details::HeaderGroup);
            }
            if (response.GetHeaders().find(Details::HeaderPermissions)
                != response.GetHeaders().end())
            {
              result.Permissions = response.GetHeaders().at(Details::HeaderPermissions);
            }
            if (response.GetHeaders().find(Details::HeaderAcl) != response.GetHeaders().end())
            {
              result.Acl = response.GetHeaders().at(Details::HeaderAcl);
            }
            if (response.GetHeaders().find(Details::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration
                  = LeaseDurationType(response.GetHeaders().at(Details::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = LeaseStateType(response.GetHeaders().at(Details::HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatusType(response.GetHeaders().at(Details::HeaderLeaseStatus));
            }
            return Azure::Response<PathGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<PathDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The file was deleted.
            PathDeleteResult result;
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            return Azure::Response<PathDeleteResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<PathSetAccessControlResult> SetAccessControlParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control response.
            PathSetAccessControlResult result;
            if (response.GetHeaders().find(Details::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Core::ETag(response.GetHeaders().at(Details::HeaderETag));
            }
            if (response.GetHeaders().find(Details::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderLastModified),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            return Azure::Response<PathSetAccessControlResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<PathSetAccessControlRecursiveResult>
        SetAccessControlRecursiveParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control recursive response.
            const auto& bodyBuffer = response.GetBody();
            PathSetAccessControlRecursiveResult result = bodyBuffer.empty()
                ? PathSetAccessControlRecursiveResult()
                : PathSetAccessControlRecursiveResultFromSetAccessControlRecursiveResponse(
                    SetAccessControlRecursiveResponseFromJson(
                        Azure::Core::Internal::Json::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            return Azure::Response<PathSetAccessControlRecursiveResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static AclFailedEntry AclFailedEntryFromJson(const Azure::Core::Internal::Json::json& node)
        {
          AclFailedEntry result;
          result.Name = node["name"].get<std::string>();
          result.Type = node["type"].get<std::string>();
          result.ErrorMessage = node["errorMessage"].get<std::string>();
          return result;
        }

        static SetAccessControlRecursiveResponse SetAccessControlRecursiveResponseFromJson(
            const Azure::Core::Internal::Json::json& node)
        {
          SetAccessControlRecursiveResponse result;
          result.NumberOfSuccessfulDirectories = node["directoriesSuccessful"].get<int32_t>();
          result.NumberOfSuccessfulFiles = node["filesSuccessful"].get<int32_t>();
          result.NumberOfFailures = node["failureCount"].get<int32_t>();
          for (const auto& element : node["failedEntries"])
          {
            result.FailedEntries.emplace_back(AclFailedEntryFromJson(element));
          }
          return result;
        }

        static PathSetAccessControlRecursiveResult
        PathSetAccessControlRecursiveResultFromSetAccessControlRecursiveResponse(
            SetAccessControlRecursiveResponse object)
        {
          PathSetAccessControlRecursiveResult result;
          result.NumberOfSuccessfulDirectories = object.NumberOfSuccessfulDirectories;
          result.NumberOfSuccessfulFiles = object.NumberOfSuccessfulFiles;
          result.NumberOfFailures = object.NumberOfFailures;
          result.FailedEntries = std::move(object.FailedEntries);

          return result;
        }
        static Azure::Response<PathFlushDataResult> FlushDataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The data was flushed (written) to the file successfully.
            PathFlushDataResult result;
            if (response.GetHeaders().find(Details::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Core::ETag(response.GetHeaders().at(Details::HeaderETag));
            }
            if (response.GetHeaders().find(Details::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderLastModified),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(Details::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.ContentLength
                  = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            }
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            return Azure::Response<PathFlushDataResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<PathAppendDataResult> AppendDataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Append data to file control response.
            PathAppendDataResult result;
            result.RequestId = response.GetHeaders().at(Details::HeaderRequestId);
            if (response.GetHeaders().find(Details::HeaderContentHashMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentHashMd5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderTransactionalContentHashCrc64)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderTransactionalContentHashCrc64),
                  HashAlgorithm::Crc64);
            }
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Response<PathAppendDataResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

    }; // class DataLakeRestClient

  } // namespace Details

}}}} // namespace Azure::Storage::Files::DataLake
