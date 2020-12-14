
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_exception.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace Details {
    constexpr static const char* DefaultServiceApiVersion = "2020-02-10";
    constexpr static const char* PathDnsSuffixDefault = "dfs.core.windows.net";
    constexpr static const char* QueryFileSystemResource = "resource";
    constexpr static const char* QueryTimeout = "timeout";
    constexpr static const char* QueryRecursiveOptional = "recursive";
    constexpr static const char* QueryRecursiveRequired = "recursive";
    constexpr static const char* QueryContinuationToken = "continuation";
    constexpr static const char* QueryPathSetAccessControlRecursiveMode = "mode";
    constexpr static const char* QueryForceFlag = "forceflag";
    constexpr static const char* QueryDirectory = "directory";
    constexpr static const char* QueryPrefix = "prefix";
    constexpr static const char* QueryMaxResults = "maxresults";
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
    constexpr static const char* HeaderApiVersionParameter = "x-ms-version";
    constexpr static const char* HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderIfMatch = "if-match";
    constexpr static const char* HeaderIfModifiedSince = "if-modified-since";
    constexpr static const char* HeaderIfNoneMatch = "if-none-match";
    constexpr static const char* HeaderIfUnmodifiedSince = "if-unmodified-since";
    constexpr static const char* HeaderLeaseIdOptional = "x-ms-lease-id";
    constexpr static const char* HeaderLeaseIdRequired = "x-ms-lease-id";
    constexpr static const char* HeaderProposedLeaseIdOptional = "x-ms-proposed-lease-id";
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
    constexpr static const char* HeaderTransactionalContentMd5 = "content-md5";
    constexpr static const char* HeaderContentMd5 = "x-ms-content-md5";
    constexpr static const char* HeaderTransactionalContentCrc64 = "x-ms-content-crc64";
    constexpr static const char* HeaderUmask = "x-ms-umask";
    constexpr static const char* HeaderPermissions = "x-ms-permissions";
    constexpr static const char* HeaderRenameSource = "x-ms-rename-source";
    constexpr static const char* HeaderOwner = "x-ms-owner";
    constexpr static const char* HeaderGroup = "x-ms-group";
    constexpr static const char* HeaderAcl = "x-ms-acl";
    constexpr static const char* HeaderContentLength = "content-length";
    constexpr static const char* HeaderPathExpiryOptions = "x-ms-expiry-option";
    constexpr static const char* HeaderPathExpiryTime = "x-ms-expiry-time";
    constexpr static const char* HeaderDate = "date";
    constexpr static const char* HeaderXMsRequestId = "x-ms-request-id";
    constexpr static const char* HeaderXMsClientRequestId = "x-ms-client-request-id";
    constexpr static const char* HeaderXMsVersion = "x-ms-version";
    constexpr static const char* HeaderXMsContinuation = "x-ms-continuation";
    constexpr static const char* HeaderXMsErrorCode = "x-ms-error-code";
    constexpr static const char* HeaderETag = "etag";
    constexpr static const char* HeaderLastModified = "last-modified";
    constexpr static const char* HeaderXMsNamespaceEnabled = "x-ms-namespace-enabled";
    constexpr static const char* HeaderXMsProperties = "x-ms-properties";
    constexpr static const char* HeaderPathLeaseAction = "x-ms-lease-action";
    constexpr static const char* HeaderXMsLeaseDuration = "x-ms-lease-duration";
    constexpr static const char* HeaderXMsLeaseBreakPeriod = "x-ms-lease-break-period";
    constexpr static const char* HeaderXMsLeaseId = "x-ms-lease-id";
    constexpr static const char* HeaderXMsLeaseTime = "x-ms-lease-time";
    constexpr static const char* HeaderRange = "range";
    constexpr static const char* HeaderXMsRangeGetContentMd5 = "x-ms-range-get-content-md5";
    constexpr static const char* HeaderAcceptRanges = "accept-ranges";
    constexpr static const char* HeaderContentRange = "content-range";
    constexpr static const char* HeaderContentMD5 = "content-md5";
    constexpr static const char* HeaderXMsResourceType = "x-ms-resource-type";
    constexpr static const char* HeaderXMsLeaseState = "x-ms-lease-state";
    constexpr static const char* HeaderXMsLeaseStatus = "x-ms-lease-status";
    constexpr static const char* HeaderXMsContentMd5 = "x-ms-content-md5";
    constexpr static const char* HeaderXMsOwner = "x-ms-owner";
    constexpr static const char* HeaderXMsGroup = "x-ms-group";
    constexpr static const char* HeaderXMsPermissions = "x-ms-permissions";
    constexpr static const char* HeaderXMsAcl = "x-ms-acl";
    constexpr static const char* HeaderXMsContentCrc64 = "x-ms-content-crc64";
    constexpr static const char* HeaderXMsRequestServerEncrypted = "x-ms-request-server-encrypted";
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
    // Mode "set" sets POSIX access control rights on files and directories, "modify" modifies one
    // or more POSIX access control rights  that pre-exist on files and directories, "remove"
    // removes one or more POSIX access control rights  that were present earlier on files and
    // directories
    enum class PathSetAccessControlRecursiveMode
    {
      Set,
      Modify,
      Remove,
      Unknown
    };

    // Required. Indicates mode of the expiry time
    enum class PathExpiryOptions
    {
      NeverExpire,
      RelativeToCreation,
      RelativeToNow,
      Absolute,
      Unknown
    };

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
      std::string LastModified;
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
      std::string LastModified;
      std::string ETag;
    };

    struct FileSystemList
    {
      std::vector<Models::FileSystem> Filesystems;
    };

    // Required only for Create File and Create Directory. The value must be "file" or "directory".
    enum class PathResourceType
    {
      Directory,
      File,
      Unknown
    };

    // Optional. Valid only when namespace is enabled. This parameter determines the behavior of the
    // rename operation. The value must be "legacy" or "posix", and the default value will be
    // "posix".
    enum class PathRenameMode
    {
      Legacy,
      Posix,
      Unknown
    };

    // There are five lease actions: "acquire", "break", "change", "renew", and "release". Use
    // "acquire" and specify the "x-ms-proposed-lease-id" and "x-ms-lease-duration" to acquire a new
    // lease. Use "break" to break an existing lease. When a lease is broken, the lease break period
    // is allowed to elapse, during which time no lease operation except break and release can be
    // performed on the file. When a lease is successfully broken, the response indicates the
    // interval in seconds until a new lease can be acquired. Use "change" and specify the current
    // lease ID in "x-ms-lease-id" and the new lease ID in "x-ms-proposed-lease-id" to change the
    // lease ID of an active lease. Use "renew" and specify the "x-ms-lease-id" to renew an existing
    // lease. Use "release" and specify the "x-ms-lease-id" to release a lease.
    enum class PathLeaseAction
    {
      Acquire,
      Break,
      Change,
      Renew,
      Release,
      Unknown
    };

    // Lease state of the resource.
    enum class LeaseStateType
    {
      Available,
      Leased,
      Expired,
      Breaking,
      Broken,
      Unknown
    };

    // The lease status of the resource.
    enum class LeaseStatusType
    {
      Locked,
      Unlocked,
      Unknown
    };

    // Optional. If the value is "getStatus" only the system defined properties for the path are
    // returned. If the value is "getAccessControl" the access control list is returned in the
    // response headers (Hierarchical Namespace must be enabled for the account), otherwise the
    // properties are returned.
    enum class PathGetPropertiesAction
    {
      GetAccessControl,
      GetStatus,
      Unknown
    };

    struct ServiceListFileSystemsResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
      std::vector<Models::FileSystem> Filesystems;
    };

    struct FileSystemCreateResult
    {
      std::string ETag;
      std::string LastModified;
      std::string NamespaceEnabled;
    };

    struct FileSystemSetPropertiesResult
    {
      std::string ETag;
      std::string LastModified;
    };

    struct FileSystemGetPropertiesResult
    {
      std::string ETag;
      std::string LastModified;
      std::string Properties;
      std::string NamespaceEnabled;
    };

    struct FileSystemDeleteResult
    {
    };

    struct FileSystemListPathsResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
      std::vector<Models::Path> Paths;
    };

    struct PathCreateResult
    {
      Azure::Core::Nullable<std::string> ETag;
      Azure::Core::Nullable<std::string> LastModified;
      Azure::Core::Nullable<std::string> ContinuationToken;
      Azure::Core::Nullable<int64_t> ContentLength;
    };

    struct PathLeaseResult
    {
      std::string ETag;
      std::string LastModified;
      std::string LeaseId;
      std::string LeaseTime;
    };

    struct PathReadResult
    {
      std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
      std::string AcceptRanges;
      PathHttpHeaders HttpHeaders;
      int64_t ContentLength = int64_t();
      Azure::Core::Nullable<std::string> ContentRange;
      Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
      std::string ETag;
      std::string LastModified;
      std::string ResourceType;
      Azure::Core::Nullable<std::string> Properties;
      Azure::Core::Nullable<std::string> LeaseDuration;
      Models::LeaseStateType LeaseState = Models::LeaseStateType::Unknown;
      Models::LeaseStatusType LeaseStatus = Models::LeaseStatusType::Unknown;
    };

    struct PathGetPropertiesResult
    {
      Azure::Core::Nullable<std::string> AcceptRanges;
      PathHttpHeaders HttpHeaders;
      int64_t ContentLength = int64_t();
      Azure::Core::Nullable<std::string> ContentRange;
      std::string ETag;
      std::string LastModified;
      Azure::Core::Nullable<std::string> ResourceType;
      Azure::Core::Nullable<std::string> Properties;
      Azure::Core::Nullable<std::string> Owner;
      Azure::Core::Nullable<std::string> Group;
      Azure::Core::Nullable<std::string> Permissions;
      Azure::Core::Nullable<std::string> Acl;
      Azure::Core::Nullable<std::string> LeaseDuration;
      Azure::Core::Nullable<Models::LeaseStateType> LeaseState;
      Azure::Core::Nullable<Models::LeaseStatusType> LeaseStatus;
    };

    struct PathDeleteResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
    };

    struct PathSetAccessControlResult
    {
      std::string ETag;
      std::string LastModified;
    };

    struct PathSetAccessControlRecursiveResult
    {
      Azure::Core::Nullable<std::string> ContinuationToken;
      int32_t DirectoriesSuccessful = int32_t();
      int32_t FilesSuccessful = int32_t();
      int32_t FailureCount = int32_t();
      std::vector<Models::AclFailedEntry> FailedEntries;
    };

    struct PathFlushDataResult
    {
      std::string ETag;
      std::string LastModified;
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
      std::string LastModified;
    };

  } // namespace Models
  namespace Details {
    inline std::string PathSetAccessControlRecursiveModeToString(
        const Models::PathSetAccessControlRecursiveMode& pathSetAccessControlRecursiveMode)
    {
      switch (pathSetAccessControlRecursiveMode)
      {
        case Models::PathSetAccessControlRecursiveMode::Set:
          return "set";
        case Models::PathSetAccessControlRecursiveMode::Modify:
          return "modify";
        case Models::PathSetAccessControlRecursiveMode::Remove:
          return "remove";
        default:
          return std::string();
      }
    }

    inline Models::PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveModeFromString(
        const std::string& pathSetAccessControlRecursiveMode)
    {
      if (pathSetAccessControlRecursiveMode == "set")
      {
        return Models::PathSetAccessControlRecursiveMode::Set;
      }
      if (pathSetAccessControlRecursiveMode == "modify")
      {
        return Models::PathSetAccessControlRecursiveMode::Modify;
      }
      if (pathSetAccessControlRecursiveMode == "remove")
      {
        return Models::PathSetAccessControlRecursiveMode::Remove;
      }
      throw std::runtime_error(
          "Cannot convert " + pathSetAccessControlRecursiveMode
          + " to PathSetAccessControlRecursiveMode");
    }

    inline std::string PathExpiryOptionsToString(const Models::PathExpiryOptions& pathExpiryOptions)
    {
      switch (pathExpiryOptions)
      {
        case Models::PathExpiryOptions::NeverExpire:
          return "NeverExpire";
        case Models::PathExpiryOptions::RelativeToCreation:
          return "RelativeToCreation";
        case Models::PathExpiryOptions::RelativeToNow:
          return "RelativeToNow";
        case Models::PathExpiryOptions::Absolute:
          return "Absolute";
        default:
          return std::string();
      }
    }

    inline Models::PathExpiryOptions PathExpiryOptionsFromString(
        const std::string& pathExpiryOptions)
    {
      if (pathExpiryOptions == "NeverExpire")
      {
        return Models::PathExpiryOptions::NeverExpire;
      }
      if (pathExpiryOptions == "RelativeToCreation")
      {
        return Models::PathExpiryOptions::RelativeToCreation;
      }
      if (pathExpiryOptions == "RelativeToNow")
      {
        return Models::PathExpiryOptions::RelativeToNow;
      }
      if (pathExpiryOptions == "Absolute")
      {
        return Models::PathExpiryOptions::Absolute;
      }
      throw std::runtime_error("Cannot convert " + pathExpiryOptions + " to PathExpiryOptions");
    }

    inline std::string PathResourceTypeToString(const Models::PathResourceType& pathResourceType)
    {
      switch (pathResourceType)
      {
        case Models::PathResourceType::Directory:
          return "directory";
        case Models::PathResourceType::File:
          return "file";
        default:
          return std::string();
      }
    }

    inline Models::PathResourceType PathResourceTypeFromString(const std::string& pathResourceType)
    {
      if (pathResourceType == "directory")
      {
        return Models::PathResourceType::Directory;
      }
      if (pathResourceType == "file")
      {
        return Models::PathResourceType::File;
      }
      throw std::runtime_error("Cannot convert " + pathResourceType + " to PathResourceType");
    }

    inline std::string PathRenameModeToString(const Models::PathRenameMode& pathRenameMode)
    {
      switch (pathRenameMode)
      {
        case Models::PathRenameMode::Legacy:
          return "legacy";
        case Models::PathRenameMode::Posix:
          return "posix";
        default:
          return std::string();
      }
    }

    inline Models::PathRenameMode PathRenameModeFromString(const std::string& pathRenameMode)
    {
      if (pathRenameMode == "legacy")
      {
        return Models::PathRenameMode::Legacy;
      }
      if (pathRenameMode == "posix")
      {
        return Models::PathRenameMode::Posix;
      }
      throw std::runtime_error("Cannot convert " + pathRenameMode + " to PathRenameMode");
    }

    inline std::string PathLeaseActionToString(const Models::PathLeaseAction& pathLeaseAction)
    {
      switch (pathLeaseAction)
      {
        case Models::PathLeaseAction::Acquire:
          return "acquire";
        case Models::PathLeaseAction::Break:
          return "break";
        case Models::PathLeaseAction::Change:
          return "change";
        case Models::PathLeaseAction::Renew:
          return "renew";
        case Models::PathLeaseAction::Release:
          return "release";
        default:
          return std::string();
      }
    }

    inline Models::PathLeaseAction PathLeaseActionFromString(const std::string& pathLeaseAction)
    {
      if (pathLeaseAction == "acquire")
      {
        return Models::PathLeaseAction::Acquire;
      }
      if (pathLeaseAction == "break")
      {
        return Models::PathLeaseAction::Break;
      }
      if (pathLeaseAction == "change")
      {
        return Models::PathLeaseAction::Change;
      }
      if (pathLeaseAction == "renew")
      {
        return Models::PathLeaseAction::Renew;
      }
      if (pathLeaseAction == "release")
      {
        return Models::PathLeaseAction::Release;
      }
      throw std::runtime_error("Cannot convert " + pathLeaseAction + " to PathLeaseAction");
    }

    inline std::string LeaseStateTypeToString(const Models::LeaseStateType& leaseStateType)
    {
      switch (leaseStateType)
      {
        case Models::LeaseStateType::Available:
          return "available";
        case Models::LeaseStateType::Leased:
          return "leased";
        case Models::LeaseStateType::Expired:
          return "expired";
        case Models::LeaseStateType::Breaking:
          return "breaking";
        case Models::LeaseStateType::Broken:
          return "broken";
        default:
          return std::string();
      }
    }

    inline Models::LeaseStateType LeaseStateTypeFromString(const std::string& leaseStateType)
    {
      if (leaseStateType == "available")
      {
        return Models::LeaseStateType::Available;
      }
      if (leaseStateType == "leased")
      {
        return Models::LeaseStateType::Leased;
      }
      if (leaseStateType == "expired")
      {
        return Models::LeaseStateType::Expired;
      }
      if (leaseStateType == "breaking")
      {
        return Models::LeaseStateType::Breaking;
      }
      if (leaseStateType == "broken")
      {
        return Models::LeaseStateType::Broken;
      }
      throw std::runtime_error("Cannot convert " + leaseStateType + " to LeaseStateType");
    }

    inline std::string LeaseStatusTypeToString(const Models::LeaseStatusType& leaseStatusType)
    {
      switch (leaseStatusType)
      {
        case Models::LeaseStatusType::Locked:
          return "locked";
        case Models::LeaseStatusType::Unlocked:
          return "unlocked";
        default:
          return std::string();
      }
    }

    inline Models::LeaseStatusType LeaseStatusTypeFromString(const std::string& leaseStatusType)
    {
      if (leaseStatusType == "locked")
      {
        return Models::LeaseStatusType::Locked;
      }
      if (leaseStatusType == "unlocked")
      {
        return Models::LeaseStatusType::Unlocked;
      }
      throw std::runtime_error("Cannot convert " + leaseStatusType + " to LeaseStatusType");
    }

    inline std::string PathGetPropertiesActionToString(
        const Models::PathGetPropertiesAction& pathGetPropertiesAction)
    {
      switch (pathGetPropertiesAction)
      {
        case Models::PathGetPropertiesAction::GetAccessControl:
          return "getAccessControl";
        case Models::PathGetPropertiesAction::GetStatus:
          return "getStatus";
        default:
          return std::string();
      }
    }

    inline Models::PathGetPropertiesAction PathGetPropertiesActionFromString(
        const std::string& pathGetPropertiesAction)
    {
      if (pathGetPropertiesAction == "getAccessControl")
      {
        return Models::PathGetPropertiesAction::GetAccessControl;
      }
      if (pathGetPropertiesAction == "getStatus")
      {
        return Models::PathGetPropertiesAction::GetStatus;
      }
      throw std::runtime_error(
          "Cannot convert " + pathGetPropertiesAction + " to PathGetPropertiesAction");
    }

    class DataLakeRestClient {
    public:
      class Service {
      public:
        struct ListFileSystemsOptions
        {
          Azure::Core::Nullable<std::string>
              Prefix; // Filters results to filesystems within the specified prefix.
          Azure::Core::Nullable<std::string>
              ContinuationToken; // Optional.  When deleting a directory, the number of paths that
                                 // are deleted with each invocation is limited.  If the number of
                                 // paths to be deleted exceeds this limit, a continuation token is
                                 // returned in this response header.  When a continuation token is
                                 // returned in the response, it must be specified in a subsequent
                                 // invocation of the delete operation to continue deleting the
                                 // directory.
          Azure::Core::Nullable<int32_t>
              MaxResults; // An optional value that specifies the maximum number of items to return.
                          // If omitted or greater than 5,000, the response will include up to 5,000
                          // items.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::ServiceListFileSystemsResult> ListFileSystems(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
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
                Details::QueryMaxResults,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFileSystemsOptions.MaxResults.GetValue())));
          }
          if (listFileSystemsOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, listFileSystemsOptions.ClientRequestId.GetValue());
          }
          if (listFileSystemsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listFileSystemsOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, listFileSystemsOptions.ApiVersionParameter);
          return ListFileSystemsParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::ServiceListFileSystemsResult>
        ListFileSystemsParseResult(
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // OK
            const auto& bodyBuffer = response.GetBody();
            Models::ServiceListFileSystemsResult result = bodyBuffer.empty()
                ? Models::ServiceListFileSystemsResult()
                : ServiceListFileSystemsResultFromFileSystemList(
                    FileSystemListFromJson(nlohmann::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderXMsContinuation)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderXMsContinuation);
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

        static Storage::Files::DataLake::Models::FileSystem FileSystemFromJson(
            const nlohmann::json& node)
        {
          Storage::Files::DataLake::Models::FileSystem result;
          result.Name = node["name"].get<std::string>();
          result.LastModified = node["lastModified"].get<std::string>();
          result.ETag = node["etag"].get<std::string>();
          return result;
        }

        static Storage::Files::DataLake::Models::FileSystemList FileSystemListFromJson(
            const nlohmann::json& node)
        {
          Storage::Files::DataLake::Models::FileSystemList result;
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
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              Properties; // Optional. User-defined properties to be stored with the filesystem, in
                          // the format of a comma-separated list of name and value pairs "n1=v1,
                          // n2=v2, ...", where each value is a base64 encoded string. Note that the
                          // string may only contain ASCII characters in the ISO-8859-1 character
                          // set.  If the filesystem exists, any properties not included in the list
                          // will be removed.  All properties are removed if the header is omitted.
                          // To merge new and existing properties, first get all existing properties
                          // and the current E-Tag, then make a conditional request with the E-Tag
                          // and include values for all properties.
        };

        static Azure::Core::Response<Models::FileSystemCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter("filesystem"));
          if (createOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, createOptions.ClientRequestId.GetValue());
          }
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderApiVersionParameter, createOptions.ApiVersionParameter);
          if (createOptions.Properties.HasValue())
          {
            request.AddHeader(Details::HeaderProperties, createOptions.Properties.GetValue());
          }
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct SetPropertiesOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              Properties; // Optional. User-defined properties to be stored with the filesystem, in
                          // the format of a comma-separated list of name and value pairs "n1=v1,
                          // n2=v2, ...", where each value is a base64 encoded string. Note that the
                          // string may only contain ASCII characters in the ISO-8859-1 character
                          // set.  If the filesystem exists, any properties not included in the list
                          // will be removed.  All properties are removed if the header is omitted.
                          // To merge new and existing properties, first get all existing properties
                          // and the current E-Tag, then make a conditional request with the E-Tag
                          // and include values for all properties.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::FileSystemSetPropertiesResult> SetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const SetPropertiesOptions& setPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter("filesystem"));
          if (setPropertiesOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, setPropertiesOptions.ClientRequestId.GetValue());
          }
          if (setPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(setPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, setPropertiesOptions.ApiVersionParameter);
          if (setPropertiesOptions.Properties.HasValue())
          {
            request.AddHeader(
                Details::HeaderProperties, setPropertiesOptions.Properties.GetValue());
          }
          if (setPropertiesOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince, setPropertiesOptions.IfModifiedSince.GetValue());
          }
          if (setPropertiesOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                setPropertiesOptions.IfUnmodifiedSince.GetValue());
          }
          return SetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::FileSystemGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter("filesystem"));
          if (getPropertiesOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, getPropertiesOptions.ClientRequestId.GetValue());
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, getPropertiesOptions.ApiVersionParameter);
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::FileSystemDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter("filesystem"));
          if (deleteOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, deleteOptions.ClientRequestId.GetValue());
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderApiVersionParameter, deleteOptions.ApiVersionParameter);
          if (deleteOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince, deleteOptions.IfModifiedSince.GetValue());
          }
          if (deleteOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince, deleteOptions.IfUnmodifiedSince.GetValue());
          }
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct ListPathsOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ContinuationToken; // Optional.  When deleting a directory, the number of paths that
                                 // are deleted with each invocation is limited.  If the number of
                                 // paths to be deleted exceeds this limit, a continuation token is
                                 // returned in this response header.  When a continuation token is
                                 // returned in the response, it must be specified in a subsequent
                                 // invocation of the delete operation to continue deleting the
                                 // directory.
          Azure::Core::Nullable<std::string>
              Directory; // Optional.  Filters results to paths within the specified directory. An
                         // error occurs if the directory does not exist.
          bool RecursiveRequired = bool(); // Required
          Azure::Core::Nullable<int32_t>
              MaxResults; // An optional value that specifies the maximum number of items to return.
                          // If omitted or greater than 5,000, the response will include up to 5,000
                          // items.
          Azure::Core::Nullable<bool>
              Upn; // Optional. Valid only when Hierarchical Namespace is enabled for the account.
                   // If "true", the user identity values returned in the x-ms-owner, x-ms-group,
                   // and x-ms-acl response headers will be transformed from Azure Active Directory
                   // Object IDs to User Principal Names.  If "false", the values will be returned
                   // as Azure Active Directory Object IDs. The default value is false. Note that
                   // group and application Object IDs are not translated because they do not have
                   // unique friendly names.
        };

        static Azure::Core::Response<Models::FileSystemListPathsResult> ListPaths(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const ListPathsOptions& listPathsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(
              Details::QueryFileSystemResource,
              Storage::Details::UrlEncodeQueryParameter("filesystem"));
          if (listPathsOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, listPathsOptions.ClientRequestId.GetValue());
          }
          if (listPathsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(listPathsOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, listPathsOptions.ApiVersionParameter);
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
                Details::QueryDirectory,
                Storage::Details::UrlEncodeQueryParameter(listPathsOptions.Directory.GetValue()));
          }
          request.GetUrl().AppendQueryParameter(
              Details::QueryRecursiveRequired,
              Storage::Details::UrlEncodeQueryParameter(
                  (listPathsOptions.RecursiveRequired ? "true" : "false")));
          if (listPathsOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryMaxResults,
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // Created
            Models::FileSystemCreateResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            result.NamespaceEnabled = response.GetHeaders().at(Details::HeaderXMsNamespaceEnabled);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            Models::FileSystemSetPropertiesResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            Models::FileSystemGetPropertiesResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            result.Properties = response.GetHeaders().at(Details::HeaderXMsProperties);
            result.NamespaceEnabled = response.GetHeaders().at(Details::HeaderXMsNamespaceEnabled);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            const auto& bodyBuffer = response.GetBody();
            Models::FileSystemListPathsResult result = bodyBuffer.empty()
                ? Models::FileSystemListPathsResult()
                : FileSystemListPathsResultFromPathList(
                    PathListFromJson(nlohmann::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderXMsContinuation)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderXMsContinuation);
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

        static Storage::Files::DataLake::Models::Path PathFromJson(const nlohmann::json& node)
        {
          Storage::Files::DataLake::Models::Path result;
          result.Name = node["name"].get<std::string>();
          if (node.contains("isDirectory"))
          {
            result.IsDirectory = (node["isDirectory"].get<std::string>() == "true");
          }
          result.LastModified = node["lastModified"].get<std::string>();
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

        static Storage::Files::DataLake::Models::PathList PathListFromJson(
            const nlohmann::json& node)
        {
          Storage::Files::DataLake::Models::PathList result;
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
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<Models::PathResourceType>
              Resource; // Required only for Create File and Create Directory. The value must be
                        // "file" or "directory".
          Azure::Core::Nullable<std::string>
              ContinuationToken; // Optional.  When deleting a directory, the number of paths that
                                 // are deleted with each invocation is limited.  If the number of
                                 // paths to be deleted exceeds this limit, a continuation token is
                                 // returned in this response header.  When a continuation token is
                                 // returned in the response, it must be specified in a subsequent
                                 // invocation of the delete operation to continue deleting the
                                 // directory.
          Azure::Core::Nullable<Models::PathRenameMode>
              Mode; // Optional. Valid only when namespace is enabled. This parameter determines the
                    // behavior of the rename operation. The value must be "legacy" or "posix", and
                    // the default value will be "posix".
          Azure::Core::Nullable<std::string>
              CacheControl; // Optional. Sets the blob's cache control. If specified, this property
                            // is stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              ContentEncoding; // Optional. Sets the blob's content encoding. If specified, this
                               // property is stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              ContentLanguage; // Optional. Set the blob's content language. If specified, this
                               // property is stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              ContentDisposition; // Optional. Sets the blob's Content-Disposition header.
          Azure::Core::Nullable<std::string>
              ContentType; // Optional. Sets the blob's content type. If specified, this property is
                           // stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              RenameSource; // An optional file or directory to be renamed.  The value must have the
                            // following format: "/{filesystem}/{path}".  If "x-ms-properties" is
                            // specified, the properties will overwrite the existing properties;
                            // otherwise, the existing properties will be preserved. This value must
                            // be a URL percent-encoded string. Note that the string may only
                            // contain ASCII characters in the ISO-8859-1 character set.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string>
              SourceLeaseId; // A lease ID for the source path. If specified, the source path must
                             // have an active lease and the lease ID must match.
          Azure::Core::Nullable<std::string>
              Properties; // Optional. User-defined properties to be stored with the filesystem, in
                          // the format of a comma-separated list of name and value pairs "n1=v1,
                          // n2=v2, ...", where each value is a base64 encoded string. Note that the
                          // string may only contain ASCII characters in the ISO-8859-1 character
                          // set.  If the filesystem exists, any properties not included in the list
                          // will be removed.  All properties are removed if the header is omitted.
                          // To merge new and existing properties, first get all existing properties
                          // and the current E-Tag, then make a conditional request with the E-Tag
                          // and include values for all properties.
          Azure::Core::Nullable<std::string>
              Permissions; // Optional and only valid if Hierarchical Namespace is enabled for the
                           // account. Sets POSIX access permissions for the file owner, the file
                           // owning group, and others. Each class may be granted read, write, or
                           // execute permission.  The sticky bit is also supported.  Both symbolic
                           // (rwxrw-rw-) and 4-digit octal notation (e.g. 0766) are supported.
          Azure::Core::Nullable<std::string>
              Umask; // Optional and only valid if Hierarchical Namespace is enabled for the
                     // account. When creating a file or directory and the parent folder does not
                     // have a default ACL, the umask restricts the permissions of the file or
                     // directory to be created.  The resulting permission is given by p bitwise and
                     // not u, where p is the permission and u is the umask.  For example, if p is
                     // 0777 and u is 0057, then the resulting permission is 0720.  The default
                     // permission is 0777 for a directory and 0666 for a file.  The default umask
                     // is 0027.  The umask must be specified in 4-digit octal notation (e.g. 0766).
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
          Azure::Core::Nullable<std::string> SourceIfMatch; // Specify an ETag value to operate only
                                                            // on blobs with a matching value.
          Azure::Core::Nullable<std::string>
              SourceIfNoneMatch; // Specify an ETag value to operate only on blobs without a
                                 // matching value.
          Azure::Core::Nullable<std::string>
              SourceIfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              SourceIfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::PathCreateResult> Create(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          if (createOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, createOptions.ClientRequestId.GetValue());
          }
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(createOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderApiVersionParameter, createOptions.ApiVersionParameter);
          if (createOptions.Resource.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathResourceType,
                Storage::Details::UrlEncodeQueryParameter(
                    PathResourceTypeToString(createOptions.Resource.GetValue())));
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
                    PathRenameModeToString(createOptions.Mode.GetValue())));
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
            request.AddHeader(
                Details::HeaderLeaseIdOptional, createOptions.LeaseIdOptional.GetValue());
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
                Details::HeaderIfModifiedSince, createOptions.IfModifiedSince.GetValue());
          }
          if (createOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince, createOptions.IfUnmodifiedSince.GetValue());
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
                createOptions.SourceIfModifiedSince.GetValue());
          }
          if (createOptions.SourceIfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderSourceIfUnmodifiedSince,
                createOptions.SourceIfUnmodifiedSince.GetValue());
          }
          return CreateParseResult(context, pipeline.Send(context, request));
        }

        struct LeaseOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Models::PathLeaseAction XMsLeaseAction = Models::PathLeaseAction::
              Unknown; // There are five lease actions: "acquire", "break", "change", "renew", and
                       // "release". Use "acquire" and specify the "x-ms-proposed-lease-id" and
                       // "x-ms-lease-duration" to acquire a new lease. Use "break" to break an
                       // existing lease. When a lease is broken, the lease break period is allowed
                       // to elapse, during which time no lease operation except break and release
                       // can be performed on the file. When a lease is successfully broken, the
                       // response indicates the interval in seconds until a new lease can be
                       // acquired. Use "change" and specify the current lease ID in "x-ms-lease-id"
                       // and the new lease ID in "x-ms-proposed-lease-id" to change the lease ID of
                       // an active lease. Use "renew" and specify the "x-ms-lease-id" to renew an
                       // existing lease. Use "release" and specify the "x-ms-lease-id" to release a
                       // lease.
          Azure::Core::Nullable<int32_t>
              XMsLeaseDuration; // The lease duration is required to acquire a lease, and specifies
                                // the duration of the lease in seconds.  The lease duration must be
                                // between 15 and 60 seconds or -1 for infinite lease.
          Azure::Core::Nullable<int32_t>
              XMsLeaseBreakPeriod; // The lease break period duration is optional to break a lease,
                                   // and  specifies the break period of the lease in seconds.  The
                                   // lease break  duration must be between 0 and 60 seconds.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string>
              ProposedLeaseIdOptional; // Proposed lease ID, in a GUID string format. The Blob
                                       // service returns 400 (Invalid request) if the proposed
                                       // lease ID is not in the correct format. See Guid
                                       // Constructor (String) for a list of valid GUID string
                                       // formats.
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::PathLeaseResult> Lease(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const LeaseOptions& leaseOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, url);
          request.AddHeader(Details::HeaderContentLength, "0");
          if (leaseOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, leaseOptions.ClientRequestId.GetValue());
          }
          if (leaseOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(leaseOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderApiVersionParameter, leaseOptions.ApiVersionParameter);
          request.AddHeader(
              Details::HeaderPathLeaseAction, PathLeaseActionToString(leaseOptions.XMsLeaseAction));
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
            request.AddHeader(
                Details::HeaderLeaseIdOptional, leaseOptions.LeaseIdOptional.GetValue());
          }
          if (leaseOptions.ProposedLeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderProposedLeaseIdOptional,
                leaseOptions.ProposedLeaseIdOptional.GetValue());
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
                Details::HeaderIfModifiedSince, leaseOptions.IfModifiedSince.GetValue());
          }
          if (leaseOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince, leaseOptions.IfUnmodifiedSince.GetValue());
          }
          return LeaseParseResult(context, pipeline.Send(context, request));
        }

        struct ReadOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              Range; // The HTTP Range request header specifies one or more byte ranges of the
                     // resource to be retrieved.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<bool>
              XMsRangeGetContentMd5; // Optional. When this header is set to "true" and specified
                                     // together with the Range header, the service returns the MD5
                                     // hash for the range, as long as the range is less than or
                                     // equal to 4MB in size. If this header is specified without
                                     // the Range header, the service returns status code 400 (Bad
                                     // Request). If this header is set to true when the range
                                     // exceeds 4 MB in size, the service returns status code 400
                                     // (Bad Request).
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::PathReadResult> Read(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const ReadOptions& readOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url, true);
          if (readOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, readOptions.ClientRequestId.GetValue());
          }
          if (readOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(readOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderApiVersionParameter, readOptions.ApiVersionParameter);
          if (readOptions.Range.HasValue())
          {
            request.AddHeader(Details::HeaderRange, readOptions.Range.GetValue());
          }
          if (readOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseIdOptional, readOptions.LeaseIdOptional.GetValue());
          }
          if (readOptions.XMsRangeGetContentMd5.HasValue())
          {
            request.AddHeader(
                Details::HeaderXMsRangeGetContentMd5,
                (readOptions.XMsRangeGetContentMd5.GetValue() ? "true" : "false"));
          }
          if (readOptions.IfMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfMatch, readOptions.IfMatch.GetValue());
          }
          if (readOptions.IfNoneMatch.HasValue())
          {
            request.AddHeader(Details::HeaderIfNoneMatch, readOptions.IfNoneMatch.GetValue());
          }
          if (readOptions.IfModifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfModifiedSince, readOptions.IfModifiedSince.GetValue());
          }
          if (readOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince, readOptions.IfUnmodifiedSince.GetValue());
          }
          return ReadParseResult(context, pipeline.Send(context, request));
        }

        struct GetPropertiesOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<Models::PathGetPropertiesAction>
              Action; // Optional. If the value is "getStatus" only the system defined properties
                      // for the path are returned. If the value is "getAccessControl" the access
                      // control list is returned in the response headers (Hierarchical Namespace
                      // must be enabled for the account), otherwise the properties are returned.
          Azure::Core::Nullable<bool>
              Upn; // Optional. Valid only when Hierarchical Namespace is enabled for the account.
                   // If "true", the user identity values returned in the x-ms-owner, x-ms-group,
                   // and x-ms-acl response headers will be transformed from Azure Active Directory
                   // Object IDs to User Principal Names.  If "false", the values will be returned
                   // as Azure Active Directory Object IDs. The default value is false. Note that
                   // group and application Object IDs are not translated because they do not have
                   // unique friendly names.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::PathGetPropertiesResult> GetProperties(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          if (getPropertiesOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, getPropertiesOptions.ClientRequestId.GetValue());
          }
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.GetValue())));
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.Action.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryPathGetPropertiesAction,
                Storage::Details::UrlEncodeQueryParameter(
                    PathGetPropertiesActionToString(getPropertiesOptions.Action.GetValue())));
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
                Details::HeaderLeaseIdOptional, getPropertiesOptions.LeaseIdOptional.GetValue());
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
                Details::HeaderIfModifiedSince, getPropertiesOptions.IfModifiedSince.GetValue());
          }
          if (getPropertiesOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                getPropertiesOptions.IfUnmodifiedSince.GetValue());
          }
          return GetPropertiesParseResult(context, pipeline.Send(context, request));
        }

        struct DeleteOptions
        {
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<bool> RecursiveOptional; // Required
          Azure::Core::Nullable<std::string>
              ContinuationToken; // Optional.  When deleting a directory, the number of paths that
                                 // are deleted with each invocation is limited.  If the number of
                                 // paths to be deleted exceeds this limit, a continuation token is
                                 // returned in this response header.  When a continuation token is
                                 // returned in the response, it must be specified in a subsequent
                                 // invocation of the delete operation to continue deleting the
                                 // directory.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
        };

        static Azure::Core::Response<Models::PathDeleteResult> Delete(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          if (deleteOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, deleteOptions.ClientRequestId.GetValue());
          }
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryTimeout,
                Storage::Details::UrlEncodeQueryParameter(
                    std::to_string(deleteOptions.Timeout.GetValue())));
          }
          request.AddHeader(Details::HeaderApiVersionParameter, deleteOptions.ApiVersionParameter);
          if (deleteOptions.RecursiveOptional.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                Details::QueryRecursiveOptional,
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
            request.AddHeader(
                Details::HeaderLeaseIdOptional, deleteOptions.LeaseIdOptional.GetValue());
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
                Details::HeaderIfModifiedSince, deleteOptions.IfModifiedSince.GetValue());
          }
          if (deleteOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince, deleteOptions.IfUnmodifiedSince.GetValue());
          }
          return DeleteParseResult(context, pipeline.Send(context, request));
        }

        struct SetAccessControlOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string> Owner; // Optional. The owner of the blob or directory.
          Azure::Core::Nullable<std::string>
              Group; // Optional. The owning group of the blob or directory.
          Azure::Core::Nullable<std::string>
              Permissions; // Optional and only valid if Hierarchical Namespace is enabled for the
                           // account. Sets POSIX access permissions for the file owner, the file
                           // owning group, and others. Each class may be granted read, write, or
                           // execute permission.  The sticky bit is also supported.  Both symbolic
                           // (rwxrw-rw-) and 4-digit octal notation (e.g. 0766) are supported.
          Azure::Core::Nullable<std::string>
              Acl; // Sets POSIX access control rights on files and directories. The value is a
                   // comma-separated list of access control entries. Each access control entry
                   // (ACE) consists of a scope, a type, a user or group identifier, and permissions
                   // in the format "[scope:][type]:[id]:[permissions]".
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::PathSetAccessControlResult> SetAccessControl(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
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
                Details::HeaderLeaseIdOptional, setAccessControlOptions.LeaseIdOptional.GetValue());
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
                Details::HeaderIfModifiedSince, setAccessControlOptions.IfModifiedSince.GetValue());
          }
          if (setAccessControlOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince,
                setAccessControlOptions.IfUnmodifiedSince.GetValue());
          }
          if (setAccessControlOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, setAccessControlOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, setAccessControlOptions.ApiVersionParameter);
          return SetAccessControlParseResult(context, pipeline.Send(context, request));
        }

        struct SetAccessControlRecursiveOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          Azure::Core::Nullable<std::string>
              ContinuationToken; // Optional.  When deleting a directory, the number of paths that
                                 // are deleted with each invocation is limited.  If the number of
                                 // paths to be deleted exceeds this limit, a continuation token is
                                 // returned in this response header.  When a continuation token is
                                 // returned in the response, it must be specified in a subsequent
                                 // invocation of the delete operation to continue deleting the
                                 // directory.
          Models::PathSetAccessControlRecursiveMode Mode
              = Models::PathSetAccessControlRecursiveMode::
                  Unknown; // Mode "set" sets POSIX access control rights on files and directories,
                           // "modify" modifies one or more POSIX access control rights  that
                           // pre-exist on files and directories, "remove" removes one or more POSIX
                           // access control rights  that were present earlier on files and
                           // directories
          Azure::Core::Nullable<bool>
              ForceFlag; // Optional. Valid for "SetAccessControlRecursive" operation. If set to
                         // false, the operation will terminate quickly on encountering user errors
                         // (4XX). If true, the operation will ignore user errors and proceed with
                         // the operation on other sub-entities of the directory. Continuation token
                         // will only be returned when forceFlag is true in case of user errors. If
                         // not set the default value is false for this.
          Azure::Core::Nullable<int32_t>
              MaxRecords; // Optional. It specifies the maximum number of files or directories on
                          // which the acl change will be applied. If omitted or greater than 2,000,
                          // the request will process up to 2,000 items
          Azure::Core::Nullable<std::string>
              Acl; // Sets POSIX access control rights on files and directories. The value is a
                   // comma-separated list of access control entries. Each access control entry
                   // (ACE) consists of a scope, a type, a user or group identifier, and permissions
                   // in the format "[scope:][type]:[id]:[permissions]".
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::PathSetAccessControlRecursiveResult>
        SetAccessControlRecursive(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
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
              Storage::Details::UrlEncodeQueryParameter(PathSetAccessControlRecursiveModeToString(
                  setAccessControlRecursiveOptions.Mode)));
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
                Details::HeaderClientRequestId,
                setAccessControlRecursiveOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter,
              setAccessControlRecursiveOptions.ApiVersionParameter);
          return SetAccessControlRecursiveParseResult(context, pipeline.Send(context, request));
        }

        struct FlushDataOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          Azure::Core::Nullable<int64_t>
              Position; // This parameter allows the caller to upload data in parallel and control
                        // the order in which it is appended to the file.  It is required when
                        // uploading data to be appended to the file and when flushing previously
                        // uploaded data to the file.  The value must be the position where the data
                        // is to be appended.  Uploaded data is not immediately flushed, or written,
                        // to the file.  To flush, the previously uploaded data must be contiguous,
                        // the position parameter must be specified and equal to the length of the
                        // file after all data has been written, and there must not be a request
                        // entity body included with the request.
          Azure::Core::Nullable<bool>
              RetainUncommittedData; // Valid only for flush operations.  If "true", uncommitted
                                     // data is retained after the flush operation completes;
                                     // otherwise, the uncommitted data is deleted after the flush
                                     // operation.  The default is false.  Data at offsets less than
                                     // the specified position are written to the file when flush
                                     // succeeds, but this optional parameter allows data after the
                                     // flush position to be retained for a future flush operation.
          Azure::Core::Nullable<bool>
              Close; // Azure Storage Events allow applications to receive notifications when files
                     // change. When Azure Storage Events are enabled, a file changed event is
                     // raised. This event has a property indicating whether this is the final
                     // change to distinguish the difference between an intermediate flush to a file
                     // stream and the final close of a file stream. The close query parameter is
                     // valid only when the action is "flush" and change notifications are enabled.
                     // If the value of close is "true" and the flush operation completes
                     // successfully, the service raises a file change notification with a property
                     // indicating that this is the final update (the file stream has been closed).
                     // If "false" a change notification is raised indicating the file has changed.
                     // The default is false. This query parameter is set to true by the Hadoop ABFS
                     // driver to indicate that the file stream has been closed."
          Azure::Core::Nullable<int64_t>
              ContentLength; // Required for "Append Data" and "Flush Data".  Must be 0 for "Flush
                             // Data".  Must be the length of the request content in bytes for
                             // "Append Data".
          Azure::Core::Nullable<Storage::ContentHash>
              ContentMd5; // Specify the transactional md5 for the body, to be validated by the
                          // service.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string>
              CacheControl; // Optional. Sets the blob's cache control. If specified, this property
                            // is stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              ContentType; // Optional. Sets the blob's content type. If specified, this property is
                           // stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              ContentDisposition; // Optional. Sets the blob's Content-Disposition header.
          Azure::Core::Nullable<std::string>
              ContentEncoding; // Optional. Sets the blob's content encoding. If specified, this
                               // property is stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              ContentLanguage; // Optional. Set the blob's content language. If specified, this
                               // property is stored with the blob and returned with a read request.
          Azure::Core::Nullable<std::string>
              IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
          Azure::Core::Nullable<std::string> IfNoneMatch; // Specify an ETag value to operate only
                                                          // on blobs without a matching value.
          Azure::Core::Nullable<std::string>
              IfModifiedSince; // Specify this header value to operate only on a blob if it has been
                               // modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              IfUnmodifiedSince; // Specify this header value to operate only on a blob if it has
                                 // not been modified since the specified date/time.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::PathFlushDataResult> FlushData(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
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
                Details::HeaderContentMd5,
                Storage::Details::ToBase64String(flushDataOptions.ContentMd5.GetValue()));
          }
          if (flushDataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseIdOptional, flushDataOptions.LeaseIdOptional.GetValue());
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
                Details::HeaderIfModifiedSince, flushDataOptions.IfModifiedSince.GetValue());
          }
          if (flushDataOptions.IfUnmodifiedSince.HasValue())
          {
            request.AddHeader(
                Details::HeaderIfUnmodifiedSince, flushDataOptions.IfUnmodifiedSince.GetValue());
          }
          if (flushDataOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, flushDataOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, flushDataOptions.ApiVersionParameter);
          return FlushDataParseResult(context, pipeline.Send(context, request));
        }

        struct AppendDataOptions
        {
          Azure::Core::Nullable<int64_t>
              Position; // This parameter allows the caller to upload data in parallel and control
                        // the order in which it is appended to the file.  It is required when
                        // uploading data to be appended to the file and when flushing previously
                        // uploaded data to the file.  The value must be the position where the data
                        // is to be appended.  Uploaded data is not immediately flushed, or written,
                        // to the file.  To flush, the previously uploaded data must be contiguous,
                        // the position parameter must be specified and equal to the length of the
                        // file after all data has been written, and there must not be a request
                        // entity body included with the request.
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          Azure::Core::Nullable<int64_t>
              ContentLength; // Required for "Append Data" and "Flush Data".  Must be 0 for "Flush
                             // Data".  Must be the length of the request content in bytes for
                             // "Append Data".
          Azure::Core::Nullable<Storage::ContentHash>
              TransactionalContentMd5; // Specify the transactional md5 for the body, to be
                                       // validated by the service.
          Azure::Core::Nullable<Storage::ContentHash>
              TransactionalContentCrc64; // Specify the transactional crc64 for the body, to be
                                         // validated by the service.
          Azure::Core::Nullable<std::string>
              LeaseIdOptional; // If specified, the operation only succeeds if the resource's lease
                               // is active and matches this ID.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        };

        static Azure::Core::Response<Models::PathAppendDataResult> AppendData(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::BodyStream& bodyStream,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
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
                Details::HeaderTransactionalContentMd5,
                Storage::Details::ToBase64String(
                    appendDataOptions.TransactionalContentMd5.GetValue()));
          }
          if (appendDataOptions.TransactionalContentCrc64.HasValue())
          {
            request.AddHeader(
                Details::HeaderTransactionalContentCrc64,
                Storage::Details::ToBase64String(
                    appendDataOptions.TransactionalContentCrc64.GetValue()));
          }
          if (appendDataOptions.LeaseIdOptional.HasValue())
          {
            request.AddHeader(
                Details::HeaderLeaseIdOptional, appendDataOptions.LeaseIdOptional.GetValue());
          }
          if (appendDataOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, appendDataOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(
              Details::HeaderApiVersionParameter, appendDataOptions.ApiVersionParameter);
          return AppendDataParseResult(context, pipeline.Send(context, request));
        }

        struct SetExpiryOptions
        {
          Azure::Core::Nullable<int32_t>
              Timeout; // The timeout parameter is expressed in seconds.
                       // For more information, see <a
                       // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                       // Timeouts for Blob Service Operations.</a>
          std::string ApiVersionParameter
              = Details::DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
          Azure::Core::Nullable<std::string>
              ClientRequestId; // Provides a client-generated, opaque value with a 1 KB character
                               // limit that is recorded in the analytics logs when storage
                               // analytics logging is enabled.
          Models::PathExpiryOptions XMsExpiryOption
              = Models::PathExpiryOptions::Unknown; // Required. Indicates mode of the expiry time
          Azure::Core::Nullable<std::string> PathExpiryTime; // The time to set the blob to expiry
        };

        static Azure::Core::Response<Models::PathSetExpiryResult> SetExpiry(
            const Azure::Core::Http::Url& url,
            Azure::Core::Http::HttpPipeline& pipeline,
            const Azure::Core::Context& context,
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
          request.AddHeader(
              Details::HeaderApiVersionParameter, setExpiryOptions.ApiVersionParameter);
          if (setExpiryOptions.ClientRequestId.HasValue())
          {
            request.AddHeader(
                Details::HeaderClientRequestId, setExpiryOptions.ClientRequestId.GetValue());
          }
          request.AddHeader(
              Details::HeaderPathExpiryOptions,
              PathExpiryOptionsToString(setExpiryOptions.XMsExpiryOption));
          if (setExpiryOptions.PathExpiryTime.HasValue())
          {
            request.AddHeader(
                Details::HeaderPathExpiryTime, setExpiryOptions.PathExpiryTime.GetValue());
          }
          return SetExpiryParseResult(context, pipeline.Send(context, request));
        }

      private:
        static Azure::Core::Response<Models::PathCreateResult> CreateParseResult(
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
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
              result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            }
            if (response.GetHeaders().find(Details::HeaderXMsContinuation)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderXMsContinuation);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The "renew", "change" or "release" action was successful.
            Models::PathLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            if (response.GetHeaders().find(Details::HeaderXMsLeaseId)
                != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::HeaderXMsLeaseId);
            }
            return Azure::Core::Response<Models::PathLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
          {
            // A new lease has been created.  The "acquire" action was successful.
            Models::PathLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            if (response.GetHeaders().find(Details::HeaderXMsLeaseId)
                != response.GetHeaders().end())
            {
              result.LeaseId = response.GetHeaders().at(Details::HeaderXMsLeaseId);
            }
            return Azure::Core::Response<Models::PathLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // The "break" lease action was successful.
            Models::PathLeaseResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            result.LeaseTime = response.GetHeaders().at(Details::HeaderXMsLeaseTime);
            return Azure::Core::Response<Models::PathLeaseResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathReadResult> ReadParseResult(
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Ok
            Models::PathReadResult result;
            result.BodyStream = response.GetBodyStream();
            result.AcceptRanges = response.GetHeaders().at(Details::HeaderAcceptRanges);
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
            if (response.GetHeaders().find(Details::HeaderContentMD5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentMD5), HashAlgorithm::Md5);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            result.ResourceType = response.GetHeaders().at(Details::HeaderXMsResourceType);
            if (response.GetHeaders().find(Details::HeaderXMsProperties)
                != response.GetHeaders().end())
            {
              result.Properties = response.GetHeaders().at(Details::HeaderXMsProperties);
            }
            if (response.GetHeaders().find(Details::HeaderXMsLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = response.GetHeaders().at(Details::HeaderXMsLeaseDuration);
            }
            result.LeaseState
                = LeaseStateTypeFromString(response.GetHeaders().at(Details::HeaderXMsLeaseState));
            result.LeaseStatus = LeaseStatusTypeFromString(
                response.GetHeaders().at(Details::HeaderXMsLeaseStatus));
            return Azure::Core::Response<Models::PathReadResult>(
                std::move(result), std::move(responsePtr));
          }
          else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::PartialContent)
          {
            // Partial content
            Models::PathReadResult result;
            result.BodyStream = response.GetBodyStream();
            result.AcceptRanges = response.GetHeaders().at(Details::HeaderAcceptRanges);
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
            if (response.GetHeaders().find(Details::HeaderContentMD5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentMD5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderXMsContentMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderXMsContentMd5), HashAlgorithm::Md5);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            result.ResourceType = response.GetHeaders().at(Details::HeaderXMsResourceType);
            if (response.GetHeaders().find(Details::HeaderXMsProperties)
                != response.GetHeaders().end())
            {
              result.Properties = response.GetHeaders().at(Details::HeaderXMsProperties);
            }
            if (response.GetHeaders().find(Details::HeaderXMsLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = response.GetHeaders().at(Details::HeaderXMsLeaseDuration);
            }
            result.LeaseState
                = LeaseStateTypeFromString(response.GetHeaders().at(Details::HeaderXMsLeaseState));
            result.LeaseStatus = LeaseStatusTypeFromString(
                response.GetHeaders().at(Details::HeaderXMsLeaseStatus));
            return Azure::Core::Response<Models::PathReadResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            unused(context);
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Core::Response<Models::PathGetPropertiesResult> GetPropertiesParseResult(
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
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
            if (response.GetHeaders().find(Details::HeaderContentMD5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentMD5), HashAlgorithm::Md5);
            }
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
            if (response.GetHeaders().find(Details::HeaderXMsResourceType)
                != response.GetHeaders().end())
            {
              result.ResourceType = response.GetHeaders().at(Details::HeaderXMsResourceType);
            }
            if (response.GetHeaders().find(Details::HeaderXMsProperties)
                != response.GetHeaders().end())
            {
              result.Properties = response.GetHeaders().at(Details::HeaderXMsProperties);
            }
            if (response.GetHeaders().find(Details::HeaderXMsOwner) != response.GetHeaders().end())
            {
              result.Owner = response.GetHeaders().at(Details::HeaderXMsOwner);
            }
            if (response.GetHeaders().find(Details::HeaderXMsGroup) != response.GetHeaders().end())
            {
              result.Group = response.GetHeaders().at(Details::HeaderXMsGroup);
            }
            if (response.GetHeaders().find(Details::HeaderXMsPermissions)
                != response.GetHeaders().end())
            {
              result.Permissions = response.GetHeaders().at(Details::HeaderXMsPermissions);
            }
            if (response.GetHeaders().find(Details::HeaderXMsAcl) != response.GetHeaders().end())
            {
              result.Acl = response.GetHeaders().at(Details::HeaderXMsAcl);
            }
            if (response.GetHeaders().find(Details::HeaderXMsLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration = response.GetHeaders().at(Details::HeaderXMsLeaseDuration);
            }
            if (response.GetHeaders().find(Details::HeaderXMsLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState = LeaseStateTypeFromString(
                  response.GetHeaders().at(Details::HeaderXMsLeaseState));
            }
            if (response.GetHeaders().find(Details::HeaderXMsLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus = LeaseStatusTypeFromString(
                  response.GetHeaders().at(Details::HeaderXMsLeaseStatus));
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The file was deleted.
            Models::PathDeleteResult result;
            if (response.GetHeaders().find(Details::HeaderXMsContinuation)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderXMsContinuation);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control response.
            Models::PathSetAccessControlResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control recursive response.
            const auto& bodyBuffer = response.GetBody();
            Models::PathSetAccessControlRecursiveResult result = bodyBuffer.empty()
                ? Models::PathSetAccessControlRecursiveResult()
                : PathSetAccessControlRecursiveResultFromSetAccessControlRecursiveResponse(
                    SetAccessControlRecursiveResponseFromJson(nlohmann::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(Details::HeaderXMsContinuation)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(Details::HeaderXMsContinuation);
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

        static Storage::Files::DataLake::Models::AclFailedEntry AclFailedEntryFromJson(
            const nlohmann::json& node)
        {
          Storage::Files::DataLake::Models::AclFailedEntry result;
          result.Name = node["name"].get<std::string>();
          result.Type = node["type"].get<std::string>();
          result.ErrorMessage = node["errorMessage"].get<std::string>();
          return result;
        }

        static Storage::Files::DataLake::Models::SetAccessControlRecursiveResponse
        SetAccessControlRecursiveResponseFromJson(const nlohmann::json& node)
        {
          Storage::Files::DataLake::Models::SetAccessControlRecursiveResponse result;
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The data was flushed (written) to the file successfully.
            Models::PathFlushDataResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Append data to file control response.
            Models::PathAppendDataResult result;
            if (response.GetHeaders().find(Details::HeaderContentMD5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderContentMD5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(Details::HeaderXMsContentCrc64)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = Storage::Details::FromBase64String(
                  response.GetHeaders().at(Details::HeaderXMsContentCrc64), HashAlgorithm::Crc64);
            }
            result.IsServerEncrypted
                = response.GetHeaders().at(Details::HeaderXMsRequestServerEncrypted) == "true";
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
            const Azure::Core::Context& context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          /* const */ auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The blob expiry was set successfully.
            Models::PathSetExpiryResult result;
            result.ETag = response.GetHeaders().at(Details::HeaderETag);
            result.LastModified = response.GetHeaders().at(Details::HeaderLastModified);
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
