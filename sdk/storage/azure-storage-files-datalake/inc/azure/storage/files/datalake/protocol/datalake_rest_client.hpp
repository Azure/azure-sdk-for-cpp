
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include <azure/core/datetime.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace Details {
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
    constexpr static const char* QueryResource = "resource";
    constexpr static const char* QueryPathResourceType = "resource";
    constexpr static const char* QueryPathRenameMode = "mode";
    constexpr static const char* QueryPathGetPropertiesAction = "action";
    constexpr static const char* QueryAction = "action";
    constexpr static const char* QueryMaxRecords = "maxrecords";
    constexpr static const char* QueryComp = "comp";
    constexpr static const char* HeaderVersion = "x-ms-version";
    constexpr static const char* HeaderRequestId = "x-ms-client-request-id";
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
    constexpr static const char* HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderContinuationToken = "x-ms-continuation";
    constexpr static const char* HeaderErrorCode = "x-ms-error-code";
    constexpr static const char* HeaderETag = "etag";
    constexpr static const char* HeaderLastModified = "last-modified";
    constexpr static const char* HeaderNamespaceEnabled = "x-ms-namespace-enabled";
    constexpr static const char* HeaderPathLeaseAction = "x-ms-lease-action";
    constexpr static const char* HeaderXMsLeaseDuration = "x-ms-lease-duration";
    constexpr static const char* HeaderXMsLeaseBreakPeriod = "x-ms-lease-break-period";
    constexpr static const char* HeaderLeaseTime = "x-ms-lease-time";
    constexpr static const char* HeaderAcceptRanges = "accept-ranges";
    constexpr static const char* HeaderContentRange = "content-range";
    constexpr static const char* HeaderResourceType = "x-ms-resource-type";
    constexpr static const char* HeaderLeaseState = "x-ms-lease-state";
    constexpr static const char* HeaderLeaseStatus = "x-ms-lease-status";
    constexpr static const char* HeaderRequestIsServerEncrypted = "x-ms-request-server-encrypted";
  } // namespace Details
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
      const std::string& Get() const { return m_value; }

      const static FileSystemResourceType Filesystem;

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
      const std::string& Get() const { return m_value; }

      const static PathSetAccessControlRecursiveMode Set;
      const static PathSetAccessControlRecursiveMode Modify;
      const static PathSetAccessControlRecursiveMode Remove;

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
      const std::string& Get() const { return m_value; }

      const static PathExpiryOptions NeverExpire;
      const static PathExpiryOptions RelativeToCreation;
      const static PathExpiryOptions RelativeToNow;
      const static PathExpiryOptions Absolute;

    private:
      std::string m_value;
    }; // extensible enum PathExpiryOptions

    struct AclFailedEntry
    {
      std::string Name;
      std::string Type;
      std::string ErrorMessage;
    };

    struct SetAccessControlRecursiveResponse
    {
      int32_t DirectoriesSuccessful = int32_t();
      int32_t FilesSuccessful = int32_t();
      int32_t FailureCount = int32_t();
      std::vector<Models::AclFailedEntry> FailedEntries;
    };

    struct Path
    {
      std::string Name;
      bool IsDirectory = bool();
      Core::DateTime LastModified;
      std::string ETag;
      int64_t ContentLength = int64_t();
      std::string Owner;
      std::string Group;
      std::string Permissions;
    };

    struct PathList
    {
      std::vector<Models::Path> Paths;
    };

    struct FileSystem
    {
      std::string Name;
      Core::DateTime LastModified;
      std::string ETag;
    };

    struct FileSystemList
    {
      std::vector<Models::FileSystem> Filesystems;
    };

    // The value must be "account" for all account operations.
    class AccountResourceType {
    public:
      AccountResourceType() = default;
      explicit AccountResourceType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const AccountResourceType& other) const { return m_value == other.m_value; }
      bool operator!=(const AccountResourceType& other) const { return !(*this == other); }
      const std::string& Get() const { return m_value; }

      const static AccountResourceType Account;

    private:
      std::string m_value;
    }; // extensible enum AccountResourceType

    // Required only for Create File and Create Directory. The value must be "file" or "directory".
    class PathResourceType {
    public:
      PathResourceType() = default;
      explicit PathResourceType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathResourceType& other) const { return m_value == other.m_value; }
      bool operator!=(const PathResourceType& other) const { return !(*this == other); }
      const std::string& Get() const { return m_value; }

      const static PathResourceType Directory;
      const static PathResourceType File;

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
      const std::string& Get() const { return m_value; }

      const static PathRenameMode Legacy;
      const static PathRenameMode Posix;

    private:
      std::string m_value;
    }; // extensible enum PathRenameMode

    // There are five lease actions: "acquire", "break", "change", "renew", and "release". Use
    // "acquire" and specify the "x-ms-proposed-lease-id" and "x-ms-lease-duration" to acquire a new
    // lease. Use "break" to break an existing lease. When a lease is broken, the lease break period
    // is allowed to elapse, during which time no lease operation except break and release can be
    // performed on the file. When a lease is successfully broken, the response indicates the
    // interval in seconds until a new lease can be acquired. Use "change" and specify the current
    // lease ID in "x-ms-lease-id" and the new lease ID in "x-ms-proposed-lease-id" to change the
    // lease ID of an active lease. Use "renew" and specify the "x-ms-lease-id" to renew an existing
    // lease. Use "release" and specify the "x-ms-lease-id" to release a lease.
    class PathLeaseAction {
    public:
      PathLeaseAction() = default;
      explicit PathLeaseAction(std::string value) : m_value(std::move(value)) {}
      bool operator==(const PathLeaseAction& other) const { return m_value == other.m_value; }
      bool operator!=(const PathLeaseAction& other) const { return !(*this == other); }
      const std::string& Get() const { return m_value; }

      const static PathLeaseAction Acquire;
      const static PathLeaseAction Break;
      const static PathLeaseAction Change;
      const static PathLeaseAction Renew;
      const static PathLeaseAction Release;

    private:
      std::string m_value;
    }; // extensible enum PathLeaseAction

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
      const std::string& Get() const { return m_value; }

      const static PathGetPropertiesAction GetAccessControl;
      const static PathGetPropertiesAction GetStatus;

    private:
      std::string m_value;
    }; // extensible enum PathGetPropertiesAction

    // Lease state of the resource.
    class LeaseStateType {
    public:
      LeaseStateType() = default;
      explicit LeaseStateType(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseStateType& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseStateType& other) const { return !(*this == other); }
      const std::string& Get() const { return m_value; }

      const static LeaseStateType Available;
      const static LeaseStateType Leased;
      const static LeaseStateType Expired;
      const static LeaseStateType Breaking;
      const static LeaseStateType Broken;

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
      const std::string& Get() const { return m_value; }

      const static LeaseStatusType Locked;
      const static LeaseStatusType Unlocked;

    private:
      std::string m_value;
    }; // extensible enum LeaseStatusType

    struct ServiceListFileSystemsResult
    {
      std::vector<Models::FileSystem> Filesystems;
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct FileSystemCreateResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      std::string NamespaceEnabled;
    };

    struct FileSystemSetPropertiesResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct FileSystemGetPropertiesResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      std::string Properties;
      std::string NamespaceEnabled;
    };

    struct FileSystemDeleteResult
    {
    };

    struct FileSystemListPathsResult
    {
      std::vector<Models::Path> Paths;
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct PathCreateResult
    {
      Azure::Core::Nullable<std::string> ETag;
      Azure::Core::Nullable<Core::DateTime> LastModified;
      Azure::Core::Nullable<std::string> ContinuationToken;
      Azure::Core::Nullable<int64_t> ContentLength;
    };

    struct PathLeaseResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      std::string LeaseId;
      int32_t LeaseTime = int32_t();
    };

    struct PathGetPropertiesResult
    {
      Azure::Core::Nullable<std::string> AcceptRanges;
      PathHttpHeaders HttpHeaders;
      int64_t ContentLength = int64_t();
      Azure::Core::Nullable<std::string> ContentRange;
      std::string ETag;
      Core::DateTime LastModified;
      Azure::Core::Nullable<std::string> ResourceType;
      Azure::Core::Nullable<std::string> Properties;
      Azure::Core::Nullable<std::string> Owner;
      Azure::Core::Nullable<std::string> Group;
      Azure::Core::Nullable<std::string> Permissions;
      Azure::Core::Nullable<std::string> Acl;
      Azure::Core::Nullable<std::string> LeaseDuration;
      Azure::Core::Nullable<LeaseStateType> LeaseState;
      Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct PathDeleteResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct PathSetAccessControlResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

    struct PathSetAccessControlRecursiveResult
    {
      int32_t DirectoriesSuccessful = int32_t();
      int32_t FilesSuccessful = int32_t();
      int32_t FailureCount = int32_t();
      std::vector<Models::AclFailedEntry> FailedEntries;
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct PathFlushDataResult
    {
      std::string ETag;
      Core::DateTime LastModified;
      int64_t ContentLength = int64_t();
    };

    struct PathAppendDataResult
    {
      Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
      bool IsServerEncrypted = bool();
    };

    struct PathSetExpiryResult
    {
      std::string ETag;
      Core::DateTime LastModified;
    };

  } // namespace Models
  namespace Details {

    class DataLakeRestClient {
    public:
      class Service {
      public:
        struct ListFileSystemsOptions
        {
          Azure::Core::Nullable<std::string> Prefix;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<int32_t> MaxResults;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Core::Response<Models::ServiceListFileSystemsResult> ListFileSystems(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListFileSystemsOptions& listFileSystemsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(Details::QueryResource, "account");
          if (listFileSystemsOptions.Prefix.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPrefix,
                Storage::Details::UrlEncodeQueryParameter(
                    listFileSystemsOptions.Prefix.GetValue()));
          }
          if (listFileSystemsOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryContinuationToken,
                Storage::Details::UrlEncodeQueryParameter(
                    listFileSystemsOptions.ContinuationToken.GetValue()));
          }
          if (listFileSystemsOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPageSizeHint,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFileSystemsOptions.MaxResults.GetValue())));
          }
          if (listFileSystemsOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, listFileSystemsOptions.ClientRequestId.GetValue());
          }
          if (listFileSystemsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFileSystemsOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, listFileSystemsOptions.ApiVersionParameter);
          return ListFileSystemsParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::ServiceListFileSystemsResult>
        ListFileSystemsParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // OK
            const auto& bodyBuffer = response.GetBody();
            Models::ServiceListFileSystemsResult result = bodyBuffer.empty()
                ? Models::ServiceListFileSystemsResult()
                : ServiceListFileSystemsResultFromFileSystemList(
                    FileSystemListFromJson(nlohmann::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            return Azure::Core::Response<Models::ServiceListFileSystemsResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::FileSystem FileSystemFromJson(const nlohmann::json& node)
        {
          Models::FileSystem result;
          result.Name = node["name"].get<std::string>();
          result.LastModified = Core::DateTime::Parse(
              node["lastModified"].get<std::string>(), Core::DateTime::DateFormat::Rfc1123);
          result.ETag = node["etag"].get<std::string>();
          return result;
        }

        static Models::FileSystemList FileSystemListFromJson(const nlohmann::json& node)
        {
          Models::FileSystemList result;
          for (const auto& element : node["filesystems"])
          {
            result.Filesystems.emplace_back(FileSystemFromJson(element));
          }
          return result;
        }

        static Models::ServiceListFileSystemsResult ServiceListFileSystemsResultFromFileSystemList(
            Models::FileSystemList object)
        {
          Models::ServiceListFileSystemsResult result;
          result.Filesystems = std::move(object.Filesystems);

          return result;
        }
      };

      class FileSystem {
      public:
        struct CreateOptions
        {
          Models::FileSystemResourceType Resource;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<std::string> Properties;
        };

        static Azure::Core::Response<Models::FileSystemCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter((createOptions.Resource.Get())));
          if (createOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(Details::HeaderRequestId, createOptions.ClientRequestId.GetValue());
          }
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.Properties.HasValue())
          {
            request.AddHeader(Details::HeaderProperties, createOptions.Properties.GetValue());
          }
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct SetPropertiesOptions
        {
          Models::FileSystemResourceType Resource;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<std::string> Properties;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Core::Response<Models::FileSystemSetPropertiesResult> SetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetPropertiesOptions& setPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter((setPropertiesOptions.Resource.Get())));
          if (setPropertiesOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, setPropertiesOptions.ClientRequestId.GetValue());
          }
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setPropertiesOptions.ApiVersionParameter);
          if (setPropertiesOptions.Properties.HasValue())
          {
            request.AddHeader(
                Details::HeaderProperties, setPropertiesOptions.Properties.GetValue());
          }
          if (setPropertiesOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                setPropertiesOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (setPropertiesOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                setPropertiesOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return SetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Models::FileSystemResourceType Resource;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Core::Response<Models::FileSystemGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter((getPropertiesOptions.Resource.Get())));
          if (getPropertiesOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, getPropertiesOptions.ClientRequestId.GetValue());
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Models::FileSystemResourceType Resource;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Core::Response<Models::FileSystemDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter((deleteOptions.Resource.Get())));
          if (deleteOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(Details::HeaderRequestId, deleteOptions.ClientRequestId.GetValue());
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                deleteOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (deleteOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                deleteOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct ListDataLakePathsOptions
        {
          Models::FileSystemResourceType Resource;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<std::string> Directory;
          bool RecursiveRequired = bool();
          Azure::Core::Nullable<int32_t> MaxResults;
          Azure::Core::Nullable<bool> Upn;
        };

        static Azure::Core::Response<Models::FileSystemListPathsResult> ListPaths(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListDataLakePathsOptions& listPathsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter((listPathsOptions.Resource.Get())));
          if (listPathsOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, listPathsOptions.ClientRequestId.GetValue());
          }
          if (listPathsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listPathsOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, listPathsOptions.ApiVersionParameter);
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
          return ListPathsParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::FileSystemCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Created
            Models::FileSystemCreateResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.NamespaceEnabled = response.GetHeaders().at(Details::HeaderNamespaceEnabled);
            return Azure::Core::Response<Models::FileSystemCreateResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileSystemSetPropertiesResult>
        SetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            Models::FileSystemSetPropertiesResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::FileSystemSetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileSystemGetPropertiesResult>
        GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            Models::FileSystemGetPropertiesResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.Properties = response.GetHeaders().at(Details::HeaderProperties);
            result.NamespaceEnabled = response.GetHeaders().at(Details::HeaderNamespaceEnabled);
            return Azure::Core::Response<Models::FileSystemGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileSystemDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Accepted
            Models::FileSystemDeleteResult result;
            return Azure::Core::Response<Models::FileSystemDeleteResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::FileSystemListPathsResult> ListPathsParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            const auto& bodyBuffer = response.GetBody();
            Models::FileSystemListPathsResult result = bodyBuffer.empty()
                ? Models::FileSystemListPathsResult()
                : FileSystemListPathsResultFromPathList(
                    PathListFromJson(nlohmann::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            return Azure::Core::Response<Models::FileSystemListPathsResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::Path PathFromJson(const nlohmann::json& node)
        {
          Models::Path result;
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
            result.ContentLength = std::stoll(node["contentLength"].get<std::string>());
          }
          result.Owner = node["owner"].get<std::string>();
          result.Group = node["group"].get<std::string>();
          result.Permissions = node["permissions"].get<std::string>();
          return result;
        }

        static Models::PathList PathListFromJson(const nlohmann::json& node)
        {
          Models::PathList result;
          for (const auto& element : node["paths"])
          {
            result.Paths.emplace_back(PathFromJson(element));
          }
          return result;
        }

        static Models::FileSystemListPathsResult FileSystemListPathsResultFromPathList(
            Models::PathList object)
        {
          Models::FileSystemListPathsResult result;
          result.Paths = std::move(object.Paths);

          return result;
        }
      };

      class Path {
      public:
        struct CreateOptions
        {
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<Models::PathResourceType> Resource;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<Models::PathRenameMode> Mode;
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
          Azure::Core::Nullable<std::string> IfMatch;
          Azure::Core::Nullable<std::string> IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
          Azure::Core::Nullable<std::string> SourceIfMatch;
          Azure::Core::Nullable<std::string> SourceIfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> SourceIfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> SourceIfUnmodifiedSince;
        };

        static Azure::Core::Response<Models::PathCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          if (createOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(Details::HeaderRequestId, createOptions.ClientRequestId.GetValue());
          }
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.Resource.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathResourceType,
                Storage::Details::UrlEncodeQueryParameter(
                    (createOptions.Resource.GetValue().Get())));
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
                Storage::Details::UrlEncodeQueryParameter((createOptions.Mode.GetValue().Get())));
          }
          if (createOptions.CacheControl.HasValue())
          {
            request.AddHeader(Details::HeaderCacheControl, createOptions.CacheControl.GetValue());
          }
          if (createOptions.ContentEncoding.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentEncoding, createOptions.ContentEncoding.GetValue());
          }
          if (createOptions.ContentLanguage.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentLanguage, createOptions.ContentLanguage.GetValue());
          }
          if (createOptions.ContentDisposition.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentDisposition, createOptions.ContentDisposition.GetValue());
          }
          if (createOptions.ContentType.HasValue())
          {
            request.AddHeader(Details::HeaderContentType, createOptions.ContentType.GetValue());
          }
          if (createOptions.RenameSource.HasValue())
          {
            request.AddHeader(Details::HeaderRenameSource, createOptions.RenameSource.GetValue());
          }
          if (createOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, createOptions.LeaseIdOptional.GetValue());
          }
          if (createOptions.SourceLeaseId.HasValue())
          {
            request.AddHeader(Details::HeaderSourceLeaseId, createOptions.SourceLeaseId.GetValue());
          }
          if (createOptions.Properties.HasValue())
          {
            request.AddHeader(Details::HeaderProperties, createOptions.Properties.GetValue());
          }
          if (createOptions.Permissions.HasValue())
          {
            request.AddHeader(Details::HeaderPermissions, createOptions.Permissions.GetValue());
          }
          if (createOptions.Umask.HasValue())
          {
            request.AddHeader(Details::HeaderUmask, createOptions.Umask.GetValue());
          }
          if (createOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, createOptions.IfMatch.GetValue());
          }
          if (createOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfNoneMatch, createOptions.IfNoneMatch.GetValue());
          }
          if (createOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                createOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                createOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.SourceIfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderSourceIfMatch, createOptions.SourceIfMatch.GetValue());
          }
          if (createOptions.SourceIfNoneMatch.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceIfNoneMatch, createOptions.SourceIfNoneMatch.GetValue());
          }
          if (createOptions.SourceIfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceIfModifiedSince,
                createOptions.SourceIfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.SourceIfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceIfUnmodifiedSince,
                createOptions.SourceIfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct LeaseOptions
        {
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Models::PathLeaseAction XMsLeaseAction;
          Azure::Core::Nullable<int32_t> XMsLeaseDuration;
          Azure::Core::Nullable<int32_t> XMsLeaseBreakPeriod;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> ProposedLeaseIdOptional;
          Azure::Core::Nullable<std::string> IfMatch;
          Azure::Core::Nullable<std::string> IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Core::Response<Models::PathLeaseResult> Lease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const LeaseOptions& leaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          if (leaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(Details::HeaderRequestId, leaseOptions.ClientRequestId.GetValue());
          }
          if (leaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(leaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, leaseOptions.ApiVersionParameter);
          request.AddHeader(Details::HeaderPathLeaseAction, (leaseOptions.XMsLeaseAction.Get()));
          if (leaseOptions.XMsLeaseDuration.HasValue())
          {
            request.AddHeader(
                Details::HeaderXMsLeaseDuration,
                std::to_string(leaseOptions.XMsLeaseDuration.GetValue()));
          }
          if (leaseOptions.XMsLeaseBreakPeriod.HasValue())
          {
            request.AddHeader(
                Details::HeaderXMsLeaseBreakPeriod,
                std::to_string(leaseOptions.XMsLeaseBreakPeriod.GetValue()));
          }
          if (leaseOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, leaseOptions.LeaseIdOptional.GetValue());
          }
          if (leaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderProposedLeaseId, leaseOptions.ProposedLeaseIdOptional.GetValue());
          }
          if (leaseOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, leaseOptions.IfMatch.GetValue());
          }
          if (leaseOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfNoneMatch, leaseOptions.IfNoneMatch.GetValue());
          }
          if (leaseOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                leaseOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (leaseOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                leaseOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return LeaseParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<Models::PathGetPropertiesAction> Action;
          Azure::Core::Nullable<bool> Upn;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> IfMatch;
          Azure::Core::Nullable<std::string> IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Core::Response<Models::PathGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          if (getPropertiesOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, getPropertiesOptions.ClientRequestId.GetValue());
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.Action.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathGetPropertiesAction,
                Storage::Details::UrlEncodeQueryParameter(
                    (getPropertiesOptions.Action.GetValue().Get())));
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
            request.AddHeader(
                Details::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.GetValue());
          }
          if (getPropertiesOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, getPropertiesOptions.IfMatch.GetValue());
          }
          if (getPropertiesOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfNoneMatch, getPropertiesOptions.IfNoneMatch.GetValue());
          }
          if (getPropertiesOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                getPropertiesOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (getPropertiesOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                getPropertiesOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<std::string> ClientRequestId;
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<bool> RecursiveOptional;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> IfMatch;
          Azure::Core::Nullable<std::string> IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
        };

        static Azure::Core::Response<Models::PathDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          if (deleteOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(Details::HeaderRequestId, deleteOptions.ClientRequestId.GetValue());
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, deleteOptions.ApiVersionParameter);
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
            request.AddHeader(Details::HeaderLeaseId, deleteOptions.LeaseIdOptional.GetValue());
          }
          if (deleteOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, deleteOptions.IfMatch.GetValue());
          }
          if (deleteOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfNoneMatch, deleteOptions.IfNoneMatch.GetValue());
          }
          if (deleteOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                deleteOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (deleteOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                deleteOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct SetAccessControlOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> Owner;
          Azure::Core::Nullable<std::string> Group;
          Azure::Core::Nullable<std::string> Permissions;
          Azure::Core::Nullable<std::string> Acl;
          Azure::Core::Nullable<std::string> IfMatch;
          Azure::Core::Nullable<std::string> IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
          Azure::Core::Nullable<std::string> ClientRequestId;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Core::Response<Models::PathSetAccessControlResult> SetAccessControl(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
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
            request.AddHeader(
                Details::HeaderLeaseId, setAccessControlOptions.LeaseIdOptional.GetValue());
          }
          if (setAccessControlOptions.Owner.HasValue())
          {
            request.AddHeader(Details::HeaderOwner, setAccessControlOptions.Owner.GetValue());
          }
          if (setAccessControlOptions.Group.HasValue())
          {
            request.AddHeader(Details::HeaderGroup, setAccessControlOptions.Group.GetValue());
          }
          if (setAccessControlOptions.Permissions.HasValue())
          {
            request.AddHeader(
                Details::HeaderPermissions, setAccessControlOptions.Permissions.GetValue());
          }
          if (setAccessControlOptions.Acl.HasValue())
          {
            request.AddHeader(Details::HeaderAcl, setAccessControlOptions.Acl.GetValue());
          }
          if (setAccessControlOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, setAccessControlOptions.IfMatch.GetValue());
          }
          if (setAccessControlOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfNoneMatch, setAccessControlOptions.IfNoneMatch.GetValue());
          }
          if (setAccessControlOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                setAccessControlOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (setAccessControlOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                setAccessControlOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (setAccessControlOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, setAccessControlOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, setAccessControlOptions.ApiVersionParameter);
          return SetAccessControlParseResult(context, pipeline.Send(context, request));
        }

        struct SetAccessControlRecursiveOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<std::string> ContinuationToken;
          Models::PathSetAccessControlRecursiveMode Mode;
          Azure::Core::Nullable<bool> ForceFlag;
          Azure::Core::Nullable<int32_t> MaxRecords;
          Azure::Core::Nullable<std::string> Acl;
          Azure::Core::Nullable<std::string> ClientRequestId;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Core::Response<Models::PathSetAccessControlRecursiveResult>
        SetAccessControlRecursive(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
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
                  (setAccessControlRecursiveOptions.Mode.Get())));
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
            request.AddHeader(Details::HeaderAcl, setAccessControlRecursiveOptions.Acl.GetValue());
          }
          if (setAccessControlRecursiveOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId,
                setAccessControlRecursiveOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(
              Details::HeaderVersion, setAccessControlRecursiveOptions.ApiVersionParameter);
          return SetAccessControlRecursiveParseResult(context, pipeline.Send(context, request));
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
          Azure::Core::Nullable<std::string> IfMatch;
          Azure::Core::Nullable<std::string> IfNoneMatch;
          Azure::Core::Nullable<Core::DateTime> IfModifiedSince;
          Azure::Core::Nullable<Core::DateTime> IfUnmodifiedSince;
          Azure::Core::Nullable<std::string> ClientRequestId;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Core::Response<Models::PathFlushDataResult> FlushData(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
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
            request.AddHeader(
                Details::HeaderContentLength,
                std::to_string(flushDataOptions.ContentLength.GetValue()));
          }
          if (flushDataOptions.ContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentHashMd5,
                Storage::Details::ToBase64String(flushDataOptions.ContentMd5.GetValue()));
          }
          if (flushDataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, flushDataOptions.LeaseIdOptional.GetValue());
          }
          if (flushDataOptions.CacheControl.HasValue())
          {
            request.AddHeader(
                Details::HeaderCacheControl, flushDataOptions.CacheControl.GetValue());
          }
          if (flushDataOptions.ContentType.HasValue())
          {
            request.AddHeader(Details::HeaderContentType, flushDataOptions.ContentType.GetValue());
          }
          if (flushDataOptions.ContentDisposition.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentDisposition, flushDataOptions.ContentDisposition.GetValue());
          }
          if (flushDataOptions.ContentEncoding.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentEncoding, flushDataOptions.ContentEncoding.GetValue());
          }
          if (flushDataOptions.ContentLanguage.HasValue())
          {
            request.AddHeader(
                Details::HeaderContentLanguage, flushDataOptions.ContentLanguage.GetValue());
          }
          if (flushDataOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, flushDataOptions.IfMatch.GetValue());
          }
          if (flushDataOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfNoneMatch, flushDataOptions.IfNoneMatch.GetValue());
          }
          if (flushDataOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince,
                flushDataOptions.IfModifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (flushDataOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                flushDataOptions.IfUnmodifiedSince.GetValue().GetString(
                    Core::DateTime::DateFormat::Rfc1123));
          }
          if (flushDataOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, flushDataOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, flushDataOptions.ApiVersionParameter);
          return FlushDataParseResult(context, pipeline.Send(context, request));
        }

        struct AppendDataOptions
        {
          Azure::Core::Nullable<int64_t> Position;
          Azure::Core::Nullable<int32_t> Timeout;
          Azure::Core::Nullable<int64_t> ContentLength;
          Azure::Core::Nullable<Storage::ContentHash> TransactionalContentMd5;
          Azure::Core::Nullable<Storage::ContentHash> TransactionalContentCrc64;
          Azure::Core::Nullable<std::string> LeaseIdOptional;
          Azure::Core::Nullable<std::string> ClientRequestId;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
        };

        static Azure::Core::Response<Models::PathAppendDataResult> AppendData(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::BodyStream& bodyStream,
            Azure::Core::Http::HttpPipeline& pipeline,
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
            request.AddHeader(
                Details::HeaderContentLength,
                std::to_string(appendDataOptions.ContentLength.GetValue()));
          }
          if (appendDataOptions.TransactionalContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderTransactionalContentHashMd5,
                Storage::Details::ToBase64String(
                    appendDataOptions.TransactionalContentMd5.GetValue()));
          }
          if (appendDataOptions.TransactionalContentCrc64.HasValue())
          {
            request.AddHeader(
                Details::HeaderTransactionalContentHashCrc64,
                Storage::Details::ToBase64String(
                    appendDataOptions.TransactionalContentCrc64.GetValue()));
          }
          if (appendDataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(Details::HeaderLeaseId, appendDataOptions.LeaseIdOptional.GetValue());
          }
          if (appendDataOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, appendDataOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(Details::HeaderVersion, appendDataOptions.ApiVersionParameter);
          return AppendDataParseResult(context, pipeline.Send(context, request));
        }

        struct SetExpiryOptions
        {
          Azure::Core::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = Details::DefaultServiceApiVersion;
          Azure::Core::Nullable<std::string> ClientRequestId;
          Models::PathExpiryOptions XMsExpiryOption;
          Azure::Core::Nullable<std::string> PathExpiryTime;
        };

        static Azure::Core::Response<Models::PathSetExpiryResult> SetExpiry(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetExpiryOptions& setExpiryOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(Details::QueryComp, "expiry");
          if (setExpiryOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setExpiryOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderVersion, setExpiryOptions.ApiVersionParameter);
          if (setExpiryOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderRequestId, setExpiryOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(Details::HeaderExpiryOptions, (setExpiryOptions.XMsExpiryOption.Get()));
          if (setExpiryOptions.PathExpiryTime.HasValue())
          {
            request.AddHeader(Details::HeaderExpiresOn, setExpiryOptions.PathExpiryTime.GetValue());
          }
          return SetExpiryParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::PathCreateResult> CreateParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // The file or directory was created.
            Models::PathCreateResult result;
            if (response.GetHeaders().find(Details::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = response.GetHeaders().at(Details::HeaderETag);
            }
            if (response.GetHeaders().find(Details::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = Core::DateTime::Parse(
                  response.GetHeaders().at(Details::HeaderLastModified),
                  Core::DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            if (response.GetHeaders().find(Details::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.ContentLength
                  = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            }
            return Azure::Core::Response<Models::PathCreateResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathLeaseResult> LeaseParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The "renew", "change" or "release" action was successful.
            Models::PathLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            }
            return Azure::Core::Response<Models::PathLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // A new lease has been created.  The "acquire" action was successful.
            Models::PathLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderLeaseId) != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::HeaderLeaseId);
            }
            return Azure::Core::Response<Models::PathLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The "break" lease action was successful.
            Models::PathLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            result.LeaseTime = std::stoi(response.GetHeaders().at(Details::HeaderLeaseTime));
            return Azure::Core::Response<Models::PathLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathGetPropertiesResult> GetPropertiesParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Returns all properties for the file or directory.
            Models::PathGetPropertiesResult result;
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
            if (response.GetHeaders().find(Details::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.ContentLength
                  = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            }
            if (response.GetHeaders().find(Details::HeaderContentRange)
                != response.GetHeaders().end())
            {
              result.ContentRange = response.GetHeaders().at(Details::HeaderContentRange);
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
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
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
            if (response.GetHeaders().find(Details::HeaderXMsLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = response.GetHeaders().at(Details::HeaderXMsLeaseDuration);
            }
            if (response.GetHeaders().find(Details::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState
                  = Models::LeaseStateType(response.GetHeaders().at(Details::HeaderLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = Models::LeaseStatusType(response.GetHeaders().at(Details::HeaderLeaseStatus));
            }
            return Azure::Core::Response<Models::PathGetPropertiesResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathDeleteResult> DeleteParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The file was deleted.
            Models::PathDeleteResult result;
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            return Azure::Core::Response<Models::PathDeleteResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathSetAccessControlResult>
        SetAccessControlParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control response.
            Models::PathSetAccessControlResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::PathSetAccessControlResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathSetAccessControlRecursiveResult>
        SetAccessControlRecursiveParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control recursive response.
            const auto& bodyBuffer = response.GetBody();
            Models::PathSetAccessControlRecursiveResult result = bodyBuffer.empty()
                ? Models::PathSetAccessControlRecursiveResult()
                : PathSetAccessControlRecursiveResultFromSetAccessControlRecursiveResponse(
                    SetAccessControlRecursiveResponseFromJson(nlohmann::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderContinuationToken);
            }
            return Azure::Core::Response<Models::PathSetAccessControlRecursiveResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Models::AclFailedEntry AclFailedEntryFromJson(const nlohmann::json& node)
        {
          Models::AclFailedEntry result;
          result.Name = node["name"].get<std::string>();
          result.Type = node["type"].get<std::string>();
          result.ErrorMessage = node["errorMessage"].get<std::string>();
          return result;
        }

        static Models::SetAccessControlRecursiveResponse SetAccessControlRecursiveResponseFromJson(
            const nlohmann::json& node)
        {
          Models::SetAccessControlRecursiveResponse result;
          result.DirectoriesSuccessful = node["directoriesSuccessful"].get<int32_t>();
          result.FilesSuccessful = node["filesSuccessful"].get<int32_t>();
          result.FailureCount = node["failureCount"].get<int32_t>();
          for (const auto& element : node["failedEntries"])
          {
            result.FailedEntries.emplace_back(AclFailedEntryFromJson(element));
          }
          return result;
        }

        static Models::PathSetAccessControlRecursiveResult
        PathSetAccessControlRecursiveResultFromSetAccessControlRecursiveResponse(
            Models::SetAccessControlRecursiveResponse object)
        {
          Models::PathSetAccessControlRecursiveResult result;
          result.DirectoriesSuccessful = object.DirectoriesSuccessful;
          result.FilesSuccessful = object.FilesSuccessful;
          result.FailureCount = object.FailureCount;
          result.FailedEntries = std::move(object.FailedEntries);

          return result;
        }
        static Azure::Core::Response<Models::PathFlushDataResult> FlushDataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The data was flushed (written) to the file successfully.
            Models::PathFlushDataResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            if (response.GetHeaders().find(Details::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.ContentLength
                  = std::stoll(response.GetHeaders().at(Details::HeaderContentLength));
            }
            return Azure::Core::Response<Models::PathFlushDataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathAppendDataResult> AppendDataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Append data to file control response.
            Models::PathAppendDataResult result;
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
            return Azure::Core::Response<Models::PathAppendDataResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathSetExpiryResult> SetExpiryParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The blob expiry was set successfully.
            Models::PathSetExpiryResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = Core::DateTime::Parse(
                response.GetHeaders().at(Details::HeaderLastModified),
                Core::DateTime::DateFormat::Rfc1123);
            return Azure::Core::Response<Models::PathSetExpiryResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

    }; // class DataLakeRestClient

  } // namespace Details

}}}} // namespace Azure::Storage::Files::DataLake
