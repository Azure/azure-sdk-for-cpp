
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_error.hpp"
#include "http/http.hpp"
#include "http/pipeline.hpp"
#include "json.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace DataLake {

  namespace Details {
    constexpr static const char* c_DefaultServiceApiVersion = "";
    constexpr static const char* c_PathDnsSuffixDefault = "dfs.core.windows.net";
    constexpr static const char* c_QueryFileSystemResource = "resource";
    constexpr static const char* c_QueryTimeout = "timeout";
    constexpr static const char* c_QueryRecursiveOptional = "recursive";
    constexpr static const char* c_QueryRecursiveRequired = "recursive";
    constexpr static const char* c_QueryContinuation = "continuation";
    constexpr static const char* c_QueryPathSetAccessControlRecursiveMode = "mode";
    constexpr static const char* c_QueryDirectory = "directory";
    constexpr static const char* c_QueryPrefix = "prefix";
    constexpr static const char* c_QueryMaxResults = "maxResults";
    constexpr static const char* c_QueryUpn = "upn";
    constexpr static const char* c_QueryPosition = "position";
    constexpr static const char* c_QueryRetainUncommittedData = "retainUncommittedData";
    constexpr static const char* c_QueryClose = "close";
    constexpr static const char* c_QueryResource = "resource";
    constexpr static const char* c_QueryPathResourceType = "resource";
    constexpr static const char* c_QueryPathRenameMode = "mode";
    constexpr static const char* c_QueryPathUpdateAction = "action";
    constexpr static const char* c_QueryMaxRecords = "maxRecords";
    constexpr static const char* c_QueryPathGetPropertiesAction = "action";
    constexpr static const char* c_QueryAction = "action";
    constexpr static const char* c_HeaderApiVersionParameter = "x-ms-version";
    constexpr static const char* c_HeaderClientRequestId = "x-ms-client-request-id";
    constexpr static const char* c_HeaderIfMatch = "If-Match";
    constexpr static const char* c_HeaderIfModifiedSince = "If-Modified-Since";
    constexpr static const char* c_HeaderIfNoneMatch = "If-None-Match";
    constexpr static const char* c_HeaderIfUnmodifiedSince = "If-Unmodified-Since";
    constexpr static const char* c_HeaderLeaseIdOptional = "x-ms-lease-id";
    constexpr static const char* c_HeaderLeaseIdRequired = "x-ms-lease-id";
    constexpr static const char* c_HeaderProposedLeaseIdOptional = "x-ms-proposed-lease-id";
    constexpr static const char* c_HeaderProperties = "x-ms-properties";
    constexpr static const char* c_HeaderSourceIfMatch = "x-ms-source-if-match";
    constexpr static const char* c_HeaderSourceIfModifiedSince = "x-ms-source-if-modified-since";
    constexpr static const char* c_HeaderSourceIfNoneMatch = "x-ms-source-if-none-match";
    constexpr static const char* c_HeaderSourceIfUnmodifiedSince
        = "x-ms-source-if-unmodified-since";
    constexpr static const char* c_HeaderSourceLeaseId = "x-ms-source-lease-id";
    constexpr static const char* c_HeaderCacheControl = "x-ms-cache-control";
    constexpr static const char* c_HeaderContentDisposition = "x-ms-content-disposition";
    constexpr static const char* c_HeaderContentEncoding = "x-ms-content-encoding";
    constexpr static const char* c_HeaderContentLanguage = "x-ms-content-language";
    constexpr static const char* c_HeaderContentType = "x-ms-content-type";
    constexpr static const char* c_HeaderTransactionalContentMD5 = "Content-MD5";
    constexpr static const char* c_HeaderContentMD5 = "x-ms-content-md5";
    constexpr static const char* c_HeaderUmask = "x-ms-umask";
    constexpr static const char* c_HeaderPermissions = "x-ms-permissions";
    constexpr static const char* c_HeaderRenameSource = "x-ms-rename-source";
    constexpr static const char* c_HeaderOwner = "x-ms-owner";
    constexpr static const char* c_HeaderGroup = "x-ms-group";
    constexpr static const char* c_HeaderAcl = "x-ms-acl";
    constexpr static const char* c_HeaderContentLength = "Content-Length";
    constexpr static const char* c_HeaderDate = "Date";
    constexpr static const char* c_HeaderXMsRequestId = "x-ms-request-id";
    constexpr static const char* c_HeaderXMsVersion = "x-ms-version";
    constexpr static const char* c_HeaderXMsContinuation = "x-ms-continuation";
    constexpr static const char* c_HeaderXMsErrorCode = "x-ms-error-code";
    constexpr static const char* c_HeaderETag = "ETag";
    constexpr static const char* c_HeaderLastModified = "Last-Modified";
    constexpr static const char* c_HeaderXMsNamespaceEnabled = "x-ms-namespace-enabled";
    constexpr static const char* c_HeaderXMsProperties = "x-ms-properties";
    constexpr static const char* c_HeaderAcceptRanges = "Accept-Ranges";
    constexpr static const char* c_HeaderContentRange = "Content-Range";
    constexpr static const char* c_HeaderPathLeaseAction = "x-ms-lease-action";
    constexpr static const char* c_HeaderXMsLeaseDuration = "x-ms-lease-duration";
    constexpr static const char* c_HeaderXMsLeaseBreakPeriod = "x-ms-lease-break-period";
    constexpr static const char* c_HeaderXMsLeaseId = "x-ms-lease-id";
    constexpr static const char* c_HeaderXMsLeaseTime = "x-ms-lease-time";
    constexpr static const char* c_HeaderRange = "Range";
    constexpr static const char* c_HeaderXMsRangeGetContentMd5 = "x-ms-range-get-content-md5";
    constexpr static const char* c_HeaderXMsResourceType = "x-ms-resource-type";
    constexpr static const char* c_HeaderXMsLeaseState = "x-ms-lease-state";
    constexpr static const char* c_HeaderXMsLeaseStatus = "x-ms-lease-status";
    constexpr static const char* c_HeaderXMsContentMd5 = "x-ms-content-md5";
    constexpr static const char* c_HeaderXMsOwner = "x-ms-owner";
    constexpr static const char* c_HeaderXMsGroup = "x-ms-group";
    constexpr static const char* c_HeaderXMsPermissions = "x-ms-permissions";
    constexpr static const char* c_HeaderXMsAcl = "x-ms-acl";
    constexpr static const char* c_HeaderXMsClientRequestId = "x-ms-client-request-id";
  } // namespace Details
  // Mode "set" sets POSIX access control rights on files and directories, "modify" modifies one or
  // more POSIX access control rights  that pre-exist on files and directories, "remove" removes one
  // or more POSIX access control rights  that were present earlier on files and directories
  enum class PathSetAccessControlRecursiveMode
  {
    Set,
    Modify,
    Remove,
    Unknown
  };

  inline std::string PathSetAccessControlRecursiveModeToString(
      const PathSetAccessControlRecursiveMode& pathSetAccessControlRecursiveMode)
  {
    switch (pathSetAccessControlRecursiveMode)
    {
      case PathSetAccessControlRecursiveMode::Set:
        return "set";
      case PathSetAccessControlRecursiveMode::Modify:
        return "modify";
      case PathSetAccessControlRecursiveMode::Remove:
        return "remove";
      default:
        return std::string();
    }
  }

  inline PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveModeFromString(
      const std::string& pathSetAccessControlRecursiveMode)
  {
    if (pathSetAccessControlRecursiveMode == "set")
    {
      return PathSetAccessControlRecursiveMode::Set;
    }
    if (pathSetAccessControlRecursiveMode == "modify")
    {
      return PathSetAccessControlRecursiveMode::Modify;
    }
    if (pathSetAccessControlRecursiveMode == "remove")
    {
      return PathSetAccessControlRecursiveMode::Remove;
    }
    throw std::runtime_error(
        "Cannot convert " + pathSetAccessControlRecursiveMode
        + " to PathSetAccessControlRecursiveMode");
  }

  struct AclFailedEntry
  {
    std::string Name;
    std::string Type;
    std::string ErrorMessage;

    static AclFailedEntry CreateFromJson(const nlohmann::json& node)
    {
      AclFailedEntry result;
      result.Name = node["name"].get<std::string>();
      result.Type = node["type"].get<std::string>();
      result.ErrorMessage = node["errorMessage"].get<std::string>();
      return result;
    }
  };

  struct SetAccessControlRecursiveResponse
  {
    int32_t DirectoriesSuccessful = int32_t();
    int32_t FilesSuccessful = int32_t();
    int32_t FailureCount = int32_t();
    std::vector<AclFailedEntry> FailedEntries;

    static SetAccessControlRecursiveResponse CreateFromJson(const nlohmann::json& node)
    {
      SetAccessControlRecursiveResponse result;
      result.DirectoriesSuccessful = std::stoi(node["directoriesSuccessful"].get<std::string>());
      result.FilesSuccessful = std::stoi(node["filesSuccessful"].get<std::string>());
      result.FailureCount = std::stoi(node["failureCount"].get<std::string>());
      for (const auto& element : node["failedEntries"])
      {
        result.FailedEntries.emplace_back(AclFailedEntry::CreateFromJson(element));
      }
      return result;
    }
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

    static Path CreateFromJson(const nlohmann::json& node)
    {
      Path result;
      result.Name = node["name"].get<std::string>();
      result.IsDirectory = (node["isDirectory"].get<std::string>() == "true");
      result.LastModified = node["lastModified"].get<std::string>();
      result.ETag = node["eTag"].get<std::string>();
      result.ContentLength = std::stoll(node["contentLength"].get<std::string>());
      result.Owner = node["owner"].get<std::string>();
      result.Group = node["group"].get<std::string>();
      result.Permissions = node["permissions"].get<std::string>();
      return result;
    }
  };

  struct PathList
  {
    std::vector<Path> Paths;

    static PathList CreateFromJson(const nlohmann::json& node)
    {
      PathList result;
      for (const auto& element : node["paths"])
      {
        result.Paths.emplace_back(Path::CreateFromJson(element));
      }
      return result;
    }
  };

  struct FileSystem
  {
    std::string Name;
    std::string LastModified;
    std::string ETag;

    static FileSystem CreateFromJson(const nlohmann::json& node)
    {
      FileSystem result;
      result.Name = node["name"].get<std::string>();
      result.LastModified = node["lastModified"].get<std::string>();
      result.ETag = node["eTag"].get<std::string>();
      return result;
    }
  };

  struct FileSystemList
  {
    std::vector<FileSystem> Filesystems;

    static FileSystemList CreateFromJson(const nlohmann::json& node)
    {
      FileSystemList result;
      for (const auto& element : node["filesystems"])
      {
        result.Filesystems.emplace_back(FileSystem::CreateFromJson(element));
      }
      return result;
    }
  };

  struct StorageError
  {

    // The service error response object.
    struct Error
    {
      std::string Code; // The service error code.
      std::string Message; // The service error message.
    };

    Error Error; // The service error response object.
  };

  // Required only for Create File and Create Directory. The value must be "file" or "directory".
  enum class PathResourceType
  {
    Directory,
    File,
    Unknown
  };

  inline std::string PathResourceTypeToString(const PathResourceType& pathResourceType)
  {
    switch (pathResourceType)
    {
      case PathResourceType::Directory:
        return "directory";
      case PathResourceType::File:
        return "file";
      default:
        return std::string();
    }
  }

  inline PathResourceType PathResourceTypeFromString(const std::string& pathResourceType)
  {
    if (pathResourceType == "directory")
    {
      return PathResourceType::Directory;
    }
    if (pathResourceType == "file")
    {
      return PathResourceType::File;
    }
    throw std::runtime_error("Cannot convert " + pathResourceType + " to PathResourceType");
  }

  // Optional. Valid only when namespace is enabled. This parameter determines the behavior of the
  // rename operation. The value must be "legacy" or "posix", and the default value will be "posix".
  enum class PathRenameMode
  {
    Legacy,
    Posix,
    Unknown
  };

  inline std::string PathRenameModeToString(const PathRenameMode& pathRenameMode)
  {
    switch (pathRenameMode)
    {
      case PathRenameMode::Legacy:
        return "legacy";
      case PathRenameMode::Posix:
        return "posix";
      default:
        return std::string();
    }
  }

  inline PathRenameMode PathRenameModeFromString(const std::string& pathRenameMode)
  {
    if (pathRenameMode == "legacy")
    {
      return PathRenameMode::Legacy;
    }
    if (pathRenameMode == "posix")
    {
      return PathRenameMode::Posix;
    }
    throw std::runtime_error("Cannot convert " + pathRenameMode + " to PathRenameMode");
  }

  // The action must be "append" to upload data to be appended to a file, "flush" to flush
  // previously uploaded data to a file, "setProperties" to set the properties of a file or
  // directory, "setAccessControl" to set the owner, group, permissions, or access control list for
  // a file or directory, or  "setAccessControlRecursive" to set the access control list for a
  // directory recursively. Note that Hierarchical Namespace must be enabled for the account in
  // order to use access control.  Also note that the Access Control List (ACL) includes permissions
  // for the owner, owning group, and others, so the x-ms-permissions and x-ms-acl request headers
  // are mutually exclusive.
  enum class PathUpdateAction
  {
    Append,
    Flush,
    SetProperties,
    SetAccessControl,
    SetAccessControlRecursive,
    Unknown
  };

  inline std::string PathUpdateActionToString(const PathUpdateAction& pathUpdateAction)
  {
    switch (pathUpdateAction)
    {
      case PathUpdateAction::Append:
        return "append";
      case PathUpdateAction::Flush:
        return "flush";
      case PathUpdateAction::SetProperties:
        return "setProperties";
      case PathUpdateAction::SetAccessControl:
        return "setAccessControl";
      case PathUpdateAction::SetAccessControlRecursive:
        return "setAccessControlRecursive";
      default:
        return std::string();
    }
  }

  inline PathUpdateAction PathUpdateActionFromString(const std::string& pathUpdateAction)
  {
    if (pathUpdateAction == "append")
    {
      return PathUpdateAction::Append;
    }
    if (pathUpdateAction == "flush")
    {
      return PathUpdateAction::Flush;
    }
    if (pathUpdateAction == "setProperties")
    {
      return PathUpdateAction::SetProperties;
    }
    if (pathUpdateAction == "setAccessControl")
    {
      return PathUpdateAction::SetAccessControl;
    }
    if (pathUpdateAction == "setAccessControlRecursive")
    {
      return PathUpdateAction::SetAccessControlRecursive;
    }
    throw std::runtime_error("Cannot convert " + pathUpdateAction + " to PathUpdateAction");
  }

  // There are five lease actions: "acquire", "break", "change", "renew", and "release". Use
  // "acquire" and specify the "x-ms-proposed-lease-id" and "x-ms-lease-duration" to acquire a new
  // lease. Use "break" to break an existing lease. When a lease is broken, the lease break period
  // is allowed to elapse, during which time no lease operation except break and release can be
  // performed on the file. When a lease is successfully broken, the response indicates the interval
  // in seconds until a new lease can be acquired. Use "change" and specify the current lease ID in
  // "x-ms-lease-id" and the new lease ID in "x-ms-proposed-lease-id" to change the lease ID of an
  // active lease. Use "renew" and specify the "x-ms-lease-id" to renew an existing lease. Use
  // "release" and specify the "x-ms-lease-id" to release a lease.
  enum class PathLeaseAction
  {
    Acquire,
    Break,
    Change,
    Renew,
    Release,
    Unknown
  };

  inline std::string PathLeaseActionToString(const PathLeaseAction& pathLeaseAction)
  {
    switch (pathLeaseAction)
    {
      case PathLeaseAction::Acquire:
        return "acquire";
      case PathLeaseAction::Break:
        return "break";
      case PathLeaseAction::Change:
        return "change";
      case PathLeaseAction::Renew:
        return "renew";
      case PathLeaseAction::Release:
        return "release";
      default:
        return std::string();
    }
  }

  inline PathLeaseAction PathLeaseActionFromString(const std::string& pathLeaseAction)
  {
    if (pathLeaseAction == "acquire")
    {
      return PathLeaseAction::Acquire;
    }
    if (pathLeaseAction == "break")
    {
      return PathLeaseAction::Break;
    }
    if (pathLeaseAction == "change")
    {
      return PathLeaseAction::Change;
    }
    if (pathLeaseAction == "renew")
    {
      return PathLeaseAction::Renew;
    }
    if (pathLeaseAction == "release")
    {
      return PathLeaseAction::Release;
    }
    throw std::runtime_error("Cannot convert " + pathLeaseAction + " to PathLeaseAction");
  }

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

  inline std::string PathGetPropertiesActionToString(
      const PathGetPropertiesAction& pathGetPropertiesAction)
  {
    switch (pathGetPropertiesAction)
    {
      case PathGetPropertiesAction::GetAccessControl:
        return "getAccessControl";
      case PathGetPropertiesAction::GetStatus:
        return "getStatus";
      default:
        return std::string();
    }
  }

  inline PathGetPropertiesAction PathGetPropertiesActionFromString(
      const std::string& pathGetPropertiesAction)
  {
    if (pathGetPropertiesAction == "getAccessControl")
    {
      return PathGetPropertiesAction::GetAccessControl;
    }
    if (pathGetPropertiesAction == "getStatus")
    {
      return PathGetPropertiesAction::GetStatus;
    }
    throw std::runtime_error(
        "Cannot convert " + pathGetPropertiesAction + " to PathGetPropertiesAction");
  }

  struct ServiceListFileSystemsResponse
  {
    std::string Date;
    std::string RequestId;
    std::string Version;
    std::string Continuation;
    std::string ContentType;
    std::vector<FileSystem> Filesystems;

    static ServiceListFileSystemsResponse ServiceListFileSystemsResponseFromFileSystemList(
        FileSystemList object)
    {
      ServiceListFileSystemsResponse result;
      result.Filesystems = std::move(object.Filesystems);

      return result;
    }
  };

  struct FileSystemCreateResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string ClientRequestId;
    std::string Version;
    std::string NamespaceEnabled;
  };

  struct FileSystemSetPropertiesResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
  };

  struct FileSystemGetPropertiesResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string Properties;
    std::string NamespaceEnabled;
  };

  struct FileSystemDeleteResponse
  {
    std::string RequestId;
    std::string Version;
    std::string Date;
  };

  struct FileSystemListPathsResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string Continuation;
    std::vector<Path> Paths;

    static FileSystemListPathsResponse FileSystemListPathsResponseFromPathList(PathList object)
    {
      FileSystemListPathsResponse result;
      result.Paths = std::move(object.Paths);

      return result;
    }
  };

  struct PathCreateResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string Continuation;
    int64_t ContentLength = int64_t();
  };

  struct PathUpdateResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string AcceptRanges;
    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    int64_t ContentLength = int64_t();
    std::string ContentRange;
    std::string ContentType;
    std::string ContentMD5;
    std::string Properties;
    std::string XMsContinuation;
    std::string RequestId;
    std::string Version;
    int32_t DirectoriesSuccessful = int32_t();
    int32_t FilesSuccessful = int32_t();
    int32_t FailureCount = int32_t();
    std::vector<AclFailedEntry> FailedEntries;

    static PathUpdateResponse PathUpdateResponseFromSetAccessControlRecursiveResponse(
        SetAccessControlRecursiveResponse object)
    {
      PathUpdateResponse result;
      result.DirectoriesSuccessful = object.DirectoriesSuccessful;
      result.FilesSuccessful = object.FilesSuccessful;
      result.FailureCount = object.FailureCount;
      result.FailedEntries = std::move(object.FailedEntries);

      return result;
    }
  };

  struct PathLeaseResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string LeaseId;
    std::string LeaseTime;
  };

  struct PathReadResponse
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
    std::string AcceptRanges;
    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    int64_t ContentLength = int64_t();
    std::string ContentRange;
    std::string ContentType;
    std::string ContentMD5;
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string ResourceType;
    std::string Properties;
    std::string LeaseDuration;
    std::string LeaseState;
    std::string LeaseStatus;
    std::string XMsContentMd5;
  };

  struct PathGetPropertiesResponse
  {
    std::string AcceptRanges;
    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    int64_t ContentLength = int64_t();
    std::string ContentRange;
    std::string ContentType;
    std::string ContentMD5;
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string ResourceType;
    std::string Properties;
    std::string Owner;
    std::string Group;
    std::string Permissions;
    std::string ACL;
    std::string LeaseDuration;
    std::string LeaseState;
    std::string LeaseStatus;
  };

  struct PathDeleteResponse
  {
    std::string Date;
    std::string RequestId;
    std::string Version;
    std::string Continuation;
  };

  struct PathSetAccessControlResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string ClientRequestId;
    std::string RequestId;
    std::string Version;
  };

  struct PathSetAccessControlRecursiveResponse
  {
    std::string Date;
    std::string ClientRequestId;
    std::string Continuation;
    std::string RequestId;
    std::string Version;
    int32_t DirectoriesSuccessful = int32_t();
    int32_t FilesSuccessful = int32_t();
    int32_t FailureCount = int32_t();
    std::vector<AclFailedEntry> FailedEntries;

    static PathSetAccessControlRecursiveResponse
    PathSetAccessControlRecursiveResponseFromSetAccessControlRecursiveResponse(
        SetAccessControlRecursiveResponse object)
    {
      PathSetAccessControlRecursiveResponse result;
      result.DirectoriesSuccessful = object.DirectoriesSuccessful;
      result.FilesSuccessful = object.FilesSuccessful;
      result.FailureCount = object.FailureCount;
      result.FailedEntries = std::move(object.FailedEntries);

      return result;
    }
  };

  struct PathFlushDataResponse
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    int64_t ContentLength = int64_t();
    std::string ClientRequestId;
    std::string RequestId;
    std::string Version;
  };

  struct PathAppendDataResponse
  {
    std::string Date;
    std::string RequestId;
    std::string ClientRequestId;
    std::string Version;
  };

  class DataLakeRestClient {
  public:
    class Service {
    public:
      struct ListFileSystemsOptions
      {
        std::string Prefix; // Filters results to filesystems within the specified prefix.
        std::string
            Continuation; // Optional.  When deleting a directory, the number of paths that are
                          // deleted with each invocation is limited.  If the number of paths to be
                          // deleted exceeds this limit, a continuation token is returned in this
                          // response header.  When a continuation token is returned in the
                          // response, it must be specified in a subsequent invocation of the delete
                          // operation to continue deleting the directory.
        int32_t MaxResults = int32_t(); // An optional value that specifies the maximum number of
                                        // items to return. If omitted or greater than 5,000, the
                                        // response will include up to 5,000 items.
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
      };

      static ServiceListFileSystemsResponse ListFileSystems(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const ListFileSystemsOptions& listFileSystemsOptions)
      {
        auto request = ListFileSystemsCreateRequest(std::move(url), listFileSystemsOptions);
        return ListFileSystemsParseResponse(pipeline.Send(context, request));
      }

    private:
      static Azure::Core::Http::Request ListFileSystemsCreateRequest(
          std::string url,
          const ListFileSystemsOptions& listFileSystemsOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddQueryParameter(Details::c_QueryResource, "account");
        if (!listFileSystemsOptions.Prefix.empty())
        {
          request.AddQueryParameter(Details::c_QueryPrefix, listFileSystemsOptions.Prefix);
        }
        if (!listFileSystemsOptions.Continuation.empty())
        {
          request.AddQueryParameter(
              Details::c_QueryContinuation, listFileSystemsOptions.Continuation);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryMaxResults, std::to_string(listFileSystemsOptions.MaxResults));
        if (!listFileSystemsOptions.ClientRequestId.empty())
        {
          request.AddHeader(
              Details::c_HeaderClientRequestId, listFileSystemsOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(listFileSystemsOptions.Timeout));
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, listFileSystemsOptions.ApiVersionParameter);
        return request;
      }

      static ServiceListFileSystemsResponse ListFileSystemsParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // OK
          ServiceListFileSystemsResponse result
              = ServiceListFileSystemsResponse::ServiceListFileSystemsResponseFromFileSystemList(
                  FileSystemList::CreateFromJson(nlohmann::json::parse(
                      *Azure::Core::Http::Response::ConstructBodyBufferFromStream(
                          response.GetBodyStream().get()))));
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContinuation)
              != response.GetHeaders().end())
          {
            result.Continuation = response.GetHeaders().at(Details::c_HeaderXMsContinuation);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentType)
              != response.GetHeaders().end())
          {
            result.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }
    };

    class FileSystem {
    public:
      struct CreateOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        std::string
            Properties; // Optional. User-defined properties to be stored with the filesystem, in
                        // the format of a comma-separated list of name and value pairs "n1=v1,
                        // n2=v2, ...", where each value is a base64 encoded string. Note that the
                        // string may only contain ASCII characters in the ISO-8859-1 character set.
                        // If the filesystem exists, any properties not included in the list will be
                        // removed.  All properties are removed if the header is omitted.  To merge
                        // new and existing properties, first get all existing properties and the
                        // current E-Tag, then make a conditional request with the E-Tag and include
                        // values for all properties.
      };

      static FileSystemCreateResponse Create(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const CreateOptions& createOptions)
      {
        auto request = CreateCreateRequest(std::move(url), createOptions);
        return CreateParseResponse(pipeline.Send(context, request));
      }

      struct SetPropertiesOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        std::string
            Properties; // Optional. User-defined properties to be stored with the filesystem, in
                        // the format of a comma-separated list of name and value pairs "n1=v1,
                        // n2=v2, ...", where each value is a base64 encoded string. Note that the
                        // string may only contain ASCII characters in the ISO-8859-1 character set.
                        // If the filesystem exists, any properties not included in the list will be
                        // removed.  All properties are removed if the header is omitted.  To merge
                        // new and existing properties, first get all existing properties and the
                        // current E-Tag, then make a conditional request with the E-Tag and include
                        // values for all properties.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static FileSystemSetPropertiesResponse SetProperties(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const SetPropertiesOptions& setPropertiesOptions)
      {
        auto request = SetPropertiesCreateRequest(std::move(url), setPropertiesOptions);
        return SetPropertiesParseResponse(pipeline.Send(context, request));
      }

      struct GetPropertiesOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
      };

      static FileSystemGetPropertiesResponse GetProperties(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const GetPropertiesOptions& getPropertiesOptions)
      {
        auto request = GetPropertiesCreateRequest(std::move(url), getPropertiesOptions);
        return GetPropertiesParseResponse(pipeline.Send(context, request));
      }

      struct DeleteOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static FileSystemDeleteResponse Delete(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const DeleteOptions& deleteOptions)
      {
        auto request = DeleteCreateRequest(std::move(url), deleteOptions);
        return DeleteParseResponse(pipeline.Send(context, request));
      }

      struct ListPathsOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        std::string
            Continuation; // Optional.  When deleting a directory, the number of paths that are
                          // deleted with each invocation is limited.  If the number of paths to be
                          // deleted exceeds this limit, a continuation token is returned in this
                          // response header.  When a continuation token is returned in the
                          // response, it must be specified in a subsequent invocation of the delete
                          // operation to continue deleting the directory.
        std::string Directory; // Optional.  Filters results to paths within the specified
                               // directory. An error occurs if the directory does not exist.
        bool RecursiveRequired = bool(); // Required
        int32_t MaxResults = int32_t(); // An optional value that specifies the maximum number of
                                        // items to return. If omitted or greater than 5,000, the
                                        // response will include up to 5,000 items.
        bool Upn
            = bool(); // Optional. Valid only when Hierarchical Namespace is enabled for the
                      // account. If "true", the user identity values returned in the x-ms-owner,
                      // x-ms-group, and x-ms-acl response headers will be transformed from Azure
                      // Active Directory Object IDs to User Principal Names.  If "false", the
                      // values will be returned as Azure Active Directory Object IDs. The default
                      // value is false. Note that group and application Object IDs are not
                      // translated because they do not have unique friendly names.
      };

      static FileSystemListPathsResponse ListPaths(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const ListPathsOptions& listPathsOptions)
      {
        auto request = ListPathsCreateRequest(std::move(url), listPathsOptions);
        return ListPathsParseResponse(pipeline.Send(context, request));
      }

    private:
      static Azure::Core::Http::Request CreateCreateRequest(
          std::string url,
          const CreateOptions& createOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddQueryParameter(Details::c_QueryFileSystemResource, "filesystem");
        if (!createOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, createOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(createOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, createOptions.ApiVersionParameter);
        if (!createOptions.Properties.empty())
        {
          request.AddHeader(Details::c_HeaderProperties, createOptions.Properties);
        }
        return request;
      }

      static FileSystemCreateResponse CreateParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
        {
          // Created
          FileSystemCreateResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.ClientRequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsNamespaceEnabled)
              != response.GetHeaders().end())
          {
            result.NamespaceEnabled
                = response.GetHeaders().at(Details::c_HeaderXMsNamespaceEnabled);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request SetPropertiesCreateRequest(
          std::string url,
          const SetPropertiesOptions& setPropertiesOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
        request.AddQueryParameter(Details::c_QueryFileSystemResource, "filesystem");
        if (!setPropertiesOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, setPropertiesOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(setPropertiesOptions.Timeout));
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, setPropertiesOptions.ApiVersionParameter);
        if (!setPropertiesOptions.Properties.empty())
        {
          request.AddHeader(Details::c_HeaderProperties, setPropertiesOptions.Properties);
        }
        if (!setPropertiesOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, setPropertiesOptions.IfModifiedSince);
        }
        if (!setPropertiesOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(
              Details::c_HeaderIfUnmodifiedSince, setPropertiesOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static FileSystemSetPropertiesResponse SetPropertiesParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Ok
          FileSystemSetPropertiesResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request GetPropertiesCreateRequest(
          std::string url,
          const GetPropertiesOptions& getPropertiesOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddQueryParameter(Details::c_QueryFileSystemResource, "filesystem");
        if (!getPropertiesOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, getPropertiesOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(getPropertiesOptions.Timeout));
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, getPropertiesOptions.ApiVersionParameter);
        return request;
      }

      static FileSystemGetPropertiesResponse GetPropertiesParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Ok
          FileSystemGetPropertiesResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsProperties)
              != response.GetHeaders().end())
          {
            result.Properties = response.GetHeaders().at(Details::c_HeaderXMsProperties);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsNamespaceEnabled)
              != response.GetHeaders().end())
          {
            result.NamespaceEnabled
                = response.GetHeaders().at(Details::c_HeaderXMsNamespaceEnabled);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request DeleteCreateRequest(
          std::string url,
          const DeleteOptions& deleteOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddQueryParameter(Details::c_QueryFileSystemResource, "filesystem");
        if (!deleteOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, deleteOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(deleteOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, deleteOptions.ApiVersionParameter);
        if (!deleteOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, deleteOptions.IfModifiedSince);
        }
        if (!deleteOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, deleteOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static FileSystemDeleteResponse DeleteParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
        {
          // Accepted
          FileSystemDeleteResponse result;
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request ListPathsCreateRequest(
          std::string url,
          const ListPathsOptions& listPathsOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddQueryParameter(Details::c_QueryFileSystemResource, "filesystem");
        if (!listPathsOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, listPathsOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(listPathsOptions.Timeout));
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, listPathsOptions.ApiVersionParameter);
        if (!listPathsOptions.Continuation.empty())
        {
          request.AddQueryParameter(Details::c_QueryContinuation, listPathsOptions.Continuation);
        }
        if (!listPathsOptions.Directory.empty())
        {
          request.AddQueryParameter(Details::c_QueryDirectory, listPathsOptions.Directory);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryRecursiveRequired,
            (listPathsOptions.RecursiveRequired ? "true" : "false"));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryMaxResults, std::to_string(listPathsOptions.MaxResults));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryUpn, (listPathsOptions.Upn ? "true" : "false"));
        return request;
      }

      static FileSystemListPathsResponse ListPathsParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Ok
          FileSystemListPathsResponse result
              = FileSystemListPathsResponse::FileSystemListPathsResponseFromPathList(
                  PathList::CreateFromJson(nlohmann::json::parse(
                      *Azure::Core::Http::Response::ConstructBodyBufferFromStream(
                          response.GetBodyStream().get()))));
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContinuation)
              != response.GetHeaders().end())
          {
            result.Continuation = response.GetHeaders().at(Details::c_HeaderXMsContinuation);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }
    };

    class Path {
    public:
      struct CreateOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        PathResourceType Resource
            = PathResourceType::Unknown; // Required only for Create File and Create Directory. The
                                         // value must be "file" or "directory".
        std::string
            Continuation; // Optional.  When deleting a directory, the number of paths that are
                          // deleted with each invocation is limited.  If the number of paths to be
                          // deleted exceeds this limit, a continuation token is returned in this
                          // response header.  When a continuation token is returned in the
                          // response, it must be specified in a subsequent invocation of the delete
                          // operation to continue deleting the directory.
        PathRenameMode Mode
            = PathRenameMode::Unknown; // Optional. Valid only when namespace is enabled. This
                                       // parameter determines the behavior of the rename operation.
                                       // The value must be "legacy" or "posix", and the default
                                       // value will be "posix".
        std::string
            CacheControl; // Optional. Sets the blob's cache control. If specified, this property is
                          // stored with the blob and returned with a read request.
        std::string
            ContentEncoding; // Optional. Sets the blob's content encoding. If specified, this
                             // property is stored with the blob and returned with a read request.
        std::string
            ContentLanguage; // Optional. Set the blob's content language. If specified, this
                             // property is stored with the blob and returned with a read request.
        std::string ContentDisposition; // Optional. Sets the blob's Content-Disposition header.
        std::string
            ContentType; // Optional. Sets the blob's content type. If specified, this property is
                         // stored with the blob and returned with a read request.
        std::string
            RenameSource; // An optional file or directory to be renamed.  The value must have the
                          // following format: "/{filesystem}/{path}".  If "x-ms-properties" is
                          // specified, the properties will overwrite the existing properties;
                          // otherwise, the existing properties will be preserved. This value must
                          // be a URL percent-encoded string. Note that the string may only contain
                          // ASCII characters in the ISO-8859-1 character set.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string SourceLeaseId; // A lease ID for the source path. If specified, the source path
                                   // must have an active lease and the leaase ID must match.
        std::string
            Properties; // Optional. User-defined properties to be stored with the filesystem, in
                        // the format of a comma-separated list of name and value pairs "n1=v1,
                        // n2=v2, ...", where each value is a base64 encoded string. Note that the
                        // string may only contain ASCII characters in the ISO-8859-1 character set.
                        // If the filesystem exists, any properties not included in the list will be
                        // removed.  All properties are removed if the header is omitted.  To merge
                        // new and existing properties, first get all existing properties and the
                        // current E-Tag, then make a conditional request with the E-Tag and include
                        // values for all properties.
        std::string
            Permissions; // Optional and only valid if Hierarchical Namespace is enabled for the
                         // account. Sets POSIX access permissions for the file owner, the file
                         // owning group, and others. Each class may be granted read, write, or
                         // execute permission.  The sticky bit is also supported.  Both symbolic
                         // (rwxrw-rw-) and 4-digit octal notation (e.g. 0766) are supported.
        std::string
            Umask; // Optional and only valid if Hierarchical Namespace is enabled for the account.
                   // When creating a file or directory and the parent folder does not have a
                   // default ACL, the umask restricts the permissions of the file or directory to
                   // be created.  The resulting permission is given by p bitwise and not u, where p
                   // is the permission and u is the umask.  For example, if p is 0777 and u is
                   // 0057, then the resulting permission is 0720.  The default permission is 0777
                   // for a directory and 0666 for a file.  The default umask is 0027.  The umask
                   // must be specified in 4-digit octal notation (e.g. 0766).
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
        std::string
            SourceIfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string SourceIfNoneMatch; // Specify an ETag value to operate only on blobs without a
                                       // matching value.
        std::string SourceIfModifiedSince; // Specify this header value to operate only on a blob if
                                           // it has been modified since the specified date/time.
        std::string
            SourceIfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                     // has not been modified since the specified date/time.
      };

      static PathCreateResponse Create(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const CreateOptions& createOptions)
      {
        auto request = CreateCreateRequest(std::move(url), createOptions);
        return CreateParseResponse(pipeline.Send(context, request));
      }

      struct UpdateOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        PathUpdateAction
            Action; // The action must be "append" to upload data to be appended to a file, "flush"
                    // to flush previously uploaded data to a file, "setProperties" to set the
                    // properties of a file or directory, "setAccessControl" to set the owner,
                    // group, permissions, or access control list for a file or directory, or
                    // "setAccessControlRecursive" to set the access control list for a directory
                    // recursively. Note that Hierarchical Namespace must be enabled for the account
                    // in order to use access control.  Also note that the Access Control List (ACL)
                    // includes permissions for the owner, owning group, and others, so the
                    // x-ms-permissions and x-ms-acl request headers are mutually exclusive.
        int32_t MaxRecords
            = int32_t(); // Optional. Valid for "SetAccessControlRecursive" operation. It specifies
                         // the maximum number of files or directories on which the acl change will
                         // be applied. If omitted or greater than 2,000, the request will process
                         // up to 2,000 items
        std::string Continuation; // Optional. The number of paths processed with each invocation is
                                  // limited. If the number of paths to be processed exceeds this
                                  // limit, a continuation token is returned in the response header
                                  // x-ms-continuation. When a continuation token is  returned in
                                  // the response, it must be percent-encoded and specified in a
                                  // subsequent invocation of setAcessControlRecursive operation.
        PathSetAccessControlRecursiveMode
            Mode; // Mode "set" sets POSIX access control rights on files and directories, "modify"
                  // modifies one or more POSIX access control rights  that pre-exist on files and
                  // directories, "remove" removes one or more POSIX access control rights  that
                  // were present earlier on files and directories
        int64_t Position
            = int64_t(); // This parameter allows the caller to upload data in parallel and control
                         // the order in which it is appended to the file.  It is required when
                         // uploading data to be appended to the file and when flushing previously
                         // uploaded data to the file.  The value must be the position where the
                         // data is to be appended.  Uploaded data is not immediately flushed, or
                         // written, to the file.  To flush, the previously uploaded data must be
                         // contiguous, the position parameter must be specified and equal to the
                         // length of the file after all data has been written, and there must not
                         // be a request entity body included with the request.
        bool RetainUncommittedData
            = bool(); // Valid only for flush operations.  If "true", uncommitted data is retained
                      // after the flush operation completes; otherwise, the uncommitted data is
                      // deleted after the flush operation.  The default is false.  Data at offsets
                      // less than the specified position are written to the file when flush
                      // succeeds, but this optional parameter allows data after the flush position
                      // to be retained for a future flush operation.
        bool Close
            = bool(); // Azure Storage Events allow applications to receive notifications when files
                      // change. When Azure Storage Events are enabled, a file changed event is
                      // raised. This event has a property indicating whether this is the final
                      // change to distinguish the difference between an intermediate flush to a
                      // file stream and the final close of a file stream. The close query parameter
                      // is valid only when the action is "flush" and change notifications are
                      // enabled. If the value of close is "true" and the flush operation completes
                      // successfully, the service raises a file change notification with a property
                      // indicating that this is the final update (the file stream has been closed).
                      // If "false" a change notification is raised indicating the file has changed.
                      // The default is false. This query parameter is set to true by the Hadoop
                      // ABFS driver to indicate that the file stream has been closed."
        int64_t ContentLength = int64_t(); // Required for "Append Data" and "Flush Data".  Must be
                                           // 0 for "Flush Data".  Must be the length of the request
                                           // content in bytes for "Append Data".
        std::string ContentMD5; // Specify the transactional md5 for the body, to be validated by
                                // the service.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string
            CacheControl; // Optional. Sets the blob's cache control. If specified, this property is
                          // stored with the blob and returned with a read request.
        std::string
            ContentType; // Optional. Sets the blob's content type. If specified, this property is
                         // stored with the blob and returned with a read request.
        std::string ContentDisposition; // Optional. Sets the blob's Content-Disposition header.
        std::string
            ContentEncoding; // Optional. Sets the blob's content encoding. If specified, this
                             // property is stored with the blob and returned with a read request.
        std::string
            ContentLanguage; // Optional. Set the blob's content language. If specified, this
                             // property is stored with the blob and returned with a read request.
        std::string
            Properties; // Optional. User-defined properties to be stored with the filesystem, in
                        // the format of a comma-separated list of name and value pairs "n1=v1,
                        // n2=v2, ...", where each value is a base64 encoded string. Note that the
                        // string may only contain ASCII characters in the ISO-8859-1 character set.
                        // If the filesystem exists, any properties not included in the list will be
                        // removed.  All properties are removed if the header is omitted.  To merge
                        // new and existing properties, first get all existing properties and the
                        // current E-Tag, then make a conditional request with the E-Tag and include
                        // values for all properties.
        std::string Owner; // Optional. The owner of the blob or directory.
        std::string Group; // Optional. The owning group of the blob or directory.
        std::string
            Permissions; // Optional and only valid if Hierarchical Namespace is enabled for the
                         // account. Sets POSIX access permissions for the file owner, the file
                         // owning group, and others. Each class may be granted read, write, or
                         // execute permission.  The sticky bit is also supported.  Both symbolic
                         // (rwxrw-rw-) and 4-digit octal notation (e.g. 0766) are supported.
        std::string Acl; // Sets POSIX access control rights on files and directories. The value is
                         // a comma-separated list of access control entries. Each access control
                         // entry (ACE) consists of a scope, a type, a user or group identifier, and
                         // permissions in the format "[scope:][type]:[id]:[permissions]".
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static PathUpdateResponse Update(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const UpdateOptions& updateOptions)
      {
        auto request = UpdateCreateRequest(std::move(url), std::move(content), updateOptions);
        return UpdateParseResponse(pipeline.Send(context, request));
      }

      struct LeaseOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        PathLeaseAction
            XMsLeaseAction; // There are five lease actions: "acquire", "break", "change", "renew",
                            // and "release". Use "acquire" and specify the "x-ms-proposed-lease-id"
                            // and "x-ms-lease-duration" to acquire a new lease. Use "break" to
                            // break an existing lease. When a lease is broken, the lease break
                            // period is allowed to elapse, during which time no lease operation
                            // except break and release can be performed on the file. When a lease
                            // is successfully broken, the response indicates the interval in
                            // seconds until a new lease can be acquired. Use "change" and specify
                            // the current lease ID in "x-ms-lease-id" and the new lease ID in
                            // "x-ms-proposed-lease-id" to change the lease ID of an active lease.
                            // Use "renew" and specify the "x-ms-lease-id" to renew an existing
                            // lease. Use "release" and specify the "x-ms-lease-id" to release a
                            // lease.
        int32_t XMsLeaseDuration
            = int32_t(); // The lease duration is required to acquire a lease, and specifies the
                         // duration of the lease in seconds.  The lease duration must be between 15
                         // and 60 seconds or -1 for infinite lease.
        int32_t XMsLeaseBreakPeriod
            = int32_t(); // The lease break period duration is optional to break a lease, and
                         // specifies the break period of the lease in seconds.  The lease break
                         // duration must be between 0 and 60 seconds.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string
            ProposedLeaseIdOptional; // Proposed lease ID, in a GUID string format. The Blob service
                                     // returns 400 (Invalid request) if the proposed lease ID is
                                     // not in the correct format. See Guid Constructor (String) for
                                     // a list of valid GUID string formats.
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static PathLeaseResponse Lease(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const LeaseOptions& leaseOptions)
      {
        auto request = LeaseCreateRequest(std::move(url), leaseOptions);
        return LeaseParseResponse(pipeline.Send(context, request));
      }

      struct ReadOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        std::string Range; // The HTTP Range request header specifies one or more byte ranges of the
                           // resource to be retrieved.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        bool XMsRangeGetContentMd5
            = bool(); // Optional. When this header is set to "true" and specified together with the
                      // Range header, the service returns the MD5 hash for the range, as long as
                      // the range is less than or equal to 4MB in size. If this header is specified
                      // without the Range header, the service returns status code 400 (Bad
                      // Request). If this header is set to true when the range exceeds 4 MB in
                      // size, the service returns status code 400 (Bad Request).
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static PathReadResponse Read(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const ReadOptions& readOptions)
      {
        auto request = ReadCreateRequest(std::move(url), readOptions);
        return ReadParseResponse(pipeline.Send(context, request));
      }

      struct GetPropertiesOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        PathGetPropertiesAction Action = PathGetPropertiesAction::
            Unknown; // Optional. If the value is "getStatus" only the system defined properties for
                     // the path are returned. If the value is "getAccessControl" the access control
                     // list is returned in the response headers (Hierarchical Namespace must be
                     // enabled for the account), otherwise the properties are returned.
        bool Upn
            = bool(); // Optional. Valid only when Hierarchical Namespace is enabled for the
                      // account. If "true", the user identity values returned in the x-ms-owner,
                      // x-ms-group, and x-ms-acl response headers will be transformed from Azure
                      // Active Directory Object IDs to User Principal Names.  If "false", the
                      // values will be returned as Azure Active Directory Object IDs. The default
                      // value is false. Note that group and application Object IDs are not
                      // translated because they do not have unique friendly names.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static PathGetPropertiesResponse GetProperties(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const GetPropertiesOptions& getPropertiesOptions)
      {
        auto request = GetPropertiesCreateRequest(std::move(url), getPropertiesOptions);
        return GetPropertiesParseResponse(pipeline.Send(context, request));
      }

      struct DeleteOptions
      {
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
        bool RecursiveOptional = bool(); // Required
        std::string
            Continuation; // Optional.  When deleting a directory, the number of paths that are
                          // deleted with each invocation is limited.  If the number of paths to be
                          // deleted exceeds this limit, a continuation token is returned in this
                          // response header.  When a continuation token is returned in the
                          // response, it must be specified in a subsequent invocation of the delete
                          // operation to continue deleting the directory.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
      };

      static PathDeleteResponse Delete(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const DeleteOptions& deleteOptions)
      {
        auto request = DeleteCreateRequest(std::move(url), deleteOptions);
        return DeleteParseResponse(pipeline.Send(context, request));
      }

      struct SetAccessControlOptions
      {
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string Owner; // Optional. The owner of the blob or directory.
        std::string Group; // Optional. The owning group of the blob or directory.
        std::string
            Permissions; // Optional and only valid if Hierarchical Namespace is enabled for the
                         // account. Sets POSIX access permissions for the file owner, the file
                         // owning group, and others. Each class may be granted read, write, or
                         // execute permission.  The sticky bit is also supported.  Both symbolic
                         // (rwxrw-rw-) and 4-digit octal notation (e.g. 0766) are supported.
        std::string Acl; // Sets POSIX access control rights on files and directories. The value is
                         // a comma-separated list of access control entries. Each access control
                         // entry (ACE) consists of a scope, a type, a user or group identifier, and
                         // permissions in the format "[scope:][type]:[id]:[permissions]".
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
      };

      static PathSetAccessControlResponse SetAccessControl(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const SetAccessControlOptions& setAccessControlOptions)
      {
        auto request = SetAccessControlCreateRequest(std::move(url), setAccessControlOptions);
        return SetAccessControlParseResponse(pipeline.Send(context, request));
      }

      struct SetAccessControlRecursiveOptions
      {
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        std::string
            Continuation; // Optional.  When deleting a directory, the number of paths that are
                          // deleted with each invocation is limited.  If the number of paths to be
                          // deleted exceeds this limit, a continuation token is returned in this
                          // response header.  When a continuation token is returned in the
                          // response, it must be specified in a subsequent invocation of the delete
                          // operation to continue deleting the directory.
        PathSetAccessControlRecursiveMode
            Mode; // Mode "set" sets POSIX access control rights on files and directories, "modify"
                  // modifies one or more POSIX access control rights  that pre-exist on files and
                  // directories, "remove" removes one or more POSIX access control rights  that
                  // were present earlier on files and directories
        int32_t MaxRecords
            = int32_t(); // Optional. It specifies the maximum number of files or directories on
                         // which the acl change will be applied. If omitted or greater than 2,000,
                         // the request will process up to 2,000 items
        std::string Acl; // Sets POSIX access control rights on files and directories. The value is
                         // a comma-separated list of access control entries. Each access control
                         // entry (ACE) consists of a scope, a type, a user or group identifier, and
                         // permissions in the format "[scope:][type]:[id]:[permissions]".
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
      };

      static PathSetAccessControlRecursiveResponse SetAccessControlRecursive(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const SetAccessControlRecursiveOptions& setAccessControlRecursiveOptions)
      {
        auto request = SetAccessControlRecursiveCreateRequest(
            std::move(url), setAccessControlRecursiveOptions);
        return SetAccessControlRecursiveParseResponse(pipeline.Send(context, request));
      }

      struct FlushDataOptions
      {
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        int64_t Position
            = int64_t(); // This parameter allows the caller to upload data in parallel and control
                         // the order in which it is appended to the file.  It is required when
                         // uploading data to be appended to the file and when flushing previously
                         // uploaded data to the file.  The value must be the position where the
                         // data is to be appended.  Uploaded data is not immediately flushed, or
                         // written, to the file.  To flush, the previously uploaded data must be
                         // contiguous, the position parameter must be specified and equal to the
                         // length of the file after all data has been written, and there must not
                         // be a request entity body included with the request.
        bool RetainUncommittedData
            = bool(); // Valid only for flush operations.  If "true", uncommitted data is retained
                      // after the flush operation completes; otherwise, the uncommitted data is
                      // deleted after the flush operation.  The default is false.  Data at offsets
                      // less than the specified position are written to the file when flush
                      // succeeds, but this optional parameter allows data after the flush position
                      // to be retained for a future flush operation.
        bool Close
            = bool(); // Azure Storage Events allow applications to receive notifications when files
                      // change. When Azure Storage Events are enabled, a file changed event is
                      // raised. This event has a property indicating whether this is the final
                      // change to distinguish the difference between an intermediate flush to a
                      // file stream and the final close of a file stream. The close query parameter
                      // is valid only when the action is "flush" and change notifications are
                      // enabled. If the value of close is "true" and the flush operation completes
                      // successfully, the service raises a file change notification with a property
                      // indicating that this is the final update (the file stream has been closed).
                      // If "false" a change notification is raised indicating the file has changed.
                      // The default is false. This query parameter is set to true by the Hadoop
                      // ABFS driver to indicate that the file stream has been closed."
        int64_t ContentLength = int64_t(); // Required for "Append Data" and "Flush Data".  Must be
                                           // 0 for "Flush Data".  Must be the length of the request
                                           // content in bytes for "Append Data".
        std::string ContentMD5; // Specify the transactional md5 for the body, to be validated by
                                // the service.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.
        std::string
            CacheControl; // Optional. Sets the blob's cache control. If specified, this property is
                          // stored with the blob and returned with a read request.
        std::string
            ContentType; // Optional. Sets the blob's content type. If specified, this property is
                         // stored with the blob and returned with a read request.
        std::string ContentDisposition; // Optional. Sets the blob's Content-Disposition header.
        std::string
            ContentEncoding; // Optional. Sets the blob's content encoding. If specified, this
                             // property is stored with the blob and returned with a read request.
        std::string
            ContentLanguage; // Optional. Set the blob's content language. If specified, this
                             // property is stored with the blob and returned with a read request.
        std::string
            IfMatch; // Specify an ETag value to operate only on blobs with a matching value.
        std::string
            IfNoneMatch; // Specify an ETag value to operate only on blobs without a matching value.
        std::string IfModifiedSince; // Specify this header value to operate only on a blob if it
                                     // has been modified since the specified date/time.
        std::string IfUnmodifiedSince; // Specify this header value to operate only on a blob if it
                                       // has not been modified since the specified date/time.
        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
      };

      static PathFlushDataResponse FlushData(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          const FlushDataOptions& flushDataOptions)
      {
        auto request = FlushDataCreateRequest(std::move(url), flushDataOptions);
        return FlushDataParseResponse(pipeline.Send(context, request));
      }

      struct AppendDataOptions
      {
        int64_t Position
            = int64_t(); // This parameter allows the caller to upload data in parallel and control
                         // the order in which it is appended to the file.  It is required when
                         // uploading data to be appended to the file and when flushing previously
                         // uploaded data to the file.  The value must be the position where the
                         // data is to be appended.  Uploaded data is not immediately flushed, or
                         // written, to the file.  To flush, the previously uploaded data must be
                         // contiguous, the position parameter must be specified and equal to the
                         // length of the file after all data has been written, and there must not
                         // be a request entity body included with the request.
        int32_t Timeout
            = int32_t(); // The timeout parameter is expressed in seconds. For more
                         // information, see <a
                         // href="https://docs.microsoft.com/en-us/rest/api/storageservices/fileservices/setting-timeouts-for-blob-service-operations">Setting
                         // Timeouts for Blob Service Operations.</a>
        int64_t ContentLength = int64_t(); // Required for "Append Data" and "Flush Data".  Must be
                                           // 0 for "Flush Data".  Must be the length of the request
                                           // content in bytes for "Append Data".
        std::string TransactionalContentMD5; // Specify the transactional md5 for the body, to be
                                             // validated by the service.
        std::string LeaseIdOptional; // If specified, the operation only succeeds if the resource's
                                     // lease is active and matches this ID.

        std::string ClientRequestId; // Provides a client-generated, opaque value with a 1 KB
                                     // character limit that is recorded in the analytics logs when
                                     // storage analytics logging is enabled.
        std::string ApiVersionParameter
            = Details::c_DefaultServiceApiVersion; // Specifies the version of the operation to use
                                                   // for this request.
      };

      static PathAppendDataResponse AppendData(
          std::string url,
          Azure::Core::Http::HttpPipeline& pipeline,
          Azure::Core::Context context,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const AppendDataOptions& appendDataOptions)
      {
        auto request
            = AppendDataCreateRequest(std::move(url), std::move(content), appendDataOptions);
        return AppendDataParseResponse(pipeline.Send(context, request));
      }

    private:
      static Azure::Core::Http::Request CreateCreateRequest(
          std::string url,
          const CreateOptions& createOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
        if (!createOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, createOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(createOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, createOptions.ApiVersionParameter);
        if (createOptions.Resource != PathResourceType::Unknown)
        {
          request.AddQueryParameter(
              Details::c_QueryPathResourceType, PathResourceTypeToString(createOptions.Resource));
        }
        if (!createOptions.Continuation.empty())
        {
          request.AddQueryParameter(Details::c_QueryContinuation, createOptions.Continuation);
        }
        if (createOptions.Mode != PathRenameMode::Unknown)
        {
          request.AddQueryParameter(
              Details::c_QueryPathRenameMode, PathRenameModeToString(createOptions.Mode));
        }
        if (!createOptions.CacheControl.empty())
        {
          request.AddHeader(Details::c_HeaderCacheControl, createOptions.CacheControl);
        }
        if (!createOptions.ContentEncoding.empty())
        {
          request.AddHeader(Details::c_HeaderContentEncoding, createOptions.ContentEncoding);
        }
        if (!createOptions.ContentLanguage.empty())
        {
          request.AddHeader(Details::c_HeaderContentLanguage, createOptions.ContentLanguage);
        }
        if (!createOptions.ContentDisposition.empty())
        {
          request.AddHeader(Details::c_HeaderContentDisposition, createOptions.ContentDisposition);
        }
        if (!createOptions.ContentType.empty())
        {
          request.AddHeader(Details::c_HeaderContentType, createOptions.ContentType);
        }
        if (!createOptions.RenameSource.empty())
        {
          request.AddHeader(Details::c_HeaderRenameSource, createOptions.RenameSource);
        }
        if (!createOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, createOptions.LeaseIdOptional);
        }
        if (!createOptions.SourceLeaseId.empty())
        {
          request.AddHeader(Details::c_HeaderSourceLeaseId, createOptions.SourceLeaseId);
        }
        if (!createOptions.Properties.empty())
        {
          request.AddHeader(Details::c_HeaderProperties, createOptions.Properties);
        }
        if (!createOptions.Permissions.empty())
        {
          request.AddHeader(Details::c_HeaderPermissions, createOptions.Permissions);
        }
        if (!createOptions.Umask.empty())
        {
          request.AddHeader(Details::c_HeaderUmask, createOptions.Umask);
        }
        if (!createOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, createOptions.IfMatch);
        }
        if (!createOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, createOptions.IfNoneMatch);
        }
        if (!createOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, createOptions.IfModifiedSince);
        }
        if (!createOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, createOptions.IfUnmodifiedSince);
        }
        if (!createOptions.SourceIfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderSourceIfMatch, createOptions.SourceIfMatch);
        }
        if (!createOptions.SourceIfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderSourceIfNoneMatch, createOptions.SourceIfNoneMatch);
        }
        if (!createOptions.SourceIfModifiedSince.empty())
        {
          request.AddHeader(
              Details::c_HeaderSourceIfModifiedSince, createOptions.SourceIfModifiedSince);
        }
        if (!createOptions.SourceIfUnmodifiedSince.empty())
        {
          request.AddHeader(
              Details::c_HeaderSourceIfUnmodifiedSince, createOptions.SourceIfUnmodifiedSince);
        }
        return request;
      }

      static PathCreateResponse CreateParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
        {
          // The file or directory was created.
          PathCreateResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContinuation)
              != response.GetHeaders().end())
          {
            result.Continuation = response.GetHeaders().at(Details::c_HeaderXMsContinuation);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLength)
              != response.GetHeaders().end())
          {
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request UpdateCreateRequest(
          std::string url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const UpdateOptions& updateOptions)
      {
        Azure::Core::Http::Request request(
            Azure::Core::Http::HttpMethod::Patch, std::move(url), std::move(content));
        if (!updateOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, updateOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(updateOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, updateOptions.ApiVersionParameter);
        request.AddQueryParameter(
            Details::c_QueryPathUpdateAction, PathUpdateActionToString(updateOptions.Action));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryMaxRecords, std::to_string(updateOptions.MaxRecords));
        if (!updateOptions.Continuation.empty())
        {
          request.AddQueryParameter(Details::c_QueryContinuation, updateOptions.Continuation);
        }
        request.AddQueryParameter(
            Details::c_QueryPathSetAccessControlRecursiveMode,
            PathSetAccessControlRecursiveModeToString(updateOptions.Mode));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryPosition, std::to_string(updateOptions.Position));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryRetainUncommittedData,
            (updateOptions.RetainUncommittedData ? "true" : "false"));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryClose, (updateOptions.Close ? "true" : "false"));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddHeader(
            Details::c_HeaderContentLength, std::to_string(updateOptions.ContentLength));
        if (!updateOptions.ContentMD5.empty())
        {
          request.AddHeader(Details::c_HeaderContentMD5, updateOptions.ContentMD5);
        }
        if (!updateOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, updateOptions.LeaseIdOptional);
        }
        if (!updateOptions.CacheControl.empty())
        {
          request.AddHeader(Details::c_HeaderCacheControl, updateOptions.CacheControl);
        }
        if (!updateOptions.ContentType.empty())
        {
          request.AddHeader(Details::c_HeaderContentType, updateOptions.ContentType);
        }
        if (!updateOptions.ContentDisposition.empty())
        {
          request.AddHeader(Details::c_HeaderContentDisposition, updateOptions.ContentDisposition);
        }
        if (!updateOptions.ContentEncoding.empty())
        {
          request.AddHeader(Details::c_HeaderContentEncoding, updateOptions.ContentEncoding);
        }
        if (!updateOptions.ContentLanguage.empty())
        {
          request.AddHeader(Details::c_HeaderContentLanguage, updateOptions.ContentLanguage);
        }
        if (!updateOptions.Properties.empty())
        {
          request.AddHeader(Details::c_HeaderProperties, updateOptions.Properties);
        }
        if (!updateOptions.Owner.empty())
        {
          request.AddHeader(Details::c_HeaderOwner, updateOptions.Owner);
        }
        if (!updateOptions.Group.empty())
        {
          request.AddHeader(Details::c_HeaderGroup, updateOptions.Group);
        }
        if (!updateOptions.Permissions.empty())
        {
          request.AddHeader(Details::c_HeaderPermissions, updateOptions.Permissions);
        }
        if (!updateOptions.Acl.empty())
        {
          request.AddHeader(Details::c_HeaderAcl, updateOptions.Acl);
        }
        if (!updateOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, updateOptions.IfMatch);
        }
        if (!updateOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, updateOptions.IfNoneMatch);
        }
        if (!updateOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, updateOptions.IfModifiedSince);
        }
        if (!updateOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, updateOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static PathUpdateResponse UpdateParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // The data was flushed (written) to the file or the properties were set successfully.
          // Response body is optional and is valid only for "SetAccessControlRecursive"
          PathUpdateResponse result
              = PathUpdateResponse::PathUpdateResponseFromSetAccessControlRecursiveResponse(
                  SetAccessControlRecursiveResponse::CreateFromJson(nlohmann::json::parse(
                      *Azure::Core::Http::Response::ConstructBodyBufferFromStream(
                          response.GetBodyStream().get()))));
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderAcceptRanges)
              != response.GetHeaders().end())
          {
            result.AcceptRanges = response.GetHeaders().at(Details::c_HeaderAcceptRanges);
          }
          if (response.GetHeaders().find(Details::c_HeaderCacheControl)
              != response.GetHeaders().end())
          {
            result.CacheControl = response.GetHeaders().at(Details::c_HeaderCacheControl);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
              != response.GetHeaders().end())
          {
            result.ContentDisposition
                = response.GetHeaders().at(Details::c_HeaderContentDisposition);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
              != response.GetHeaders().end())
          {
            result.ContentEncoding = response.GetHeaders().at(Details::c_HeaderContentEncoding);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
              != response.GetHeaders().end())
          {
            result.ContentLanguage = response.GetHeaders().at(Details::c_HeaderContentLanguage);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLength)
              != response.GetHeaders().end())
          {
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
          }
          if (response.GetHeaders().find(Details::c_HeaderContentRange)
              != response.GetHeaders().end())
          {
            result.ContentRange = response.GetHeaders().at(Details::c_HeaderContentRange);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentType)
              != response.GetHeaders().end())
          {
            result.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentMD5)
              != response.GetHeaders().end())
          {
            result.ContentMD5 = response.GetHeaders().at(Details::c_HeaderContentMD5);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsProperties)
              != response.GetHeaders().end())
          {
            result.Properties = response.GetHeaders().at(Details::c_HeaderXMsProperties);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContinuation)
              != response.GetHeaders().end())
          {
            result.XMsContinuation = response.GetHeaders().at(Details::c_HeaderXMsContinuation);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
        {
          // The uploaded data was accepted.
          PathUpdateResponse result;
          if (response.GetHeaders().find(Details::c_HeaderContentMD5)
              != response.GetHeaders().end())
          {
            result.ContentMD5 = response.GetHeaders().at(Details::c_HeaderContentMD5);
          }
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request LeaseCreateRequest(
          std::string url,
          const LeaseOptions& leaseOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, url);
        if (!leaseOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, leaseOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(leaseOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, leaseOptions.ApiVersionParameter);
        request.AddHeader(
            Details::c_HeaderPathLeaseAction, PathLeaseActionToString(leaseOptions.XMsLeaseAction));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddHeader(
            Details::c_HeaderXMsLeaseDuration, std::to_string(leaseOptions.XMsLeaseDuration));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddHeader(
            Details::c_HeaderXMsLeaseBreakPeriod, std::to_string(leaseOptions.XMsLeaseBreakPeriod));
        if (!leaseOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, leaseOptions.LeaseIdOptional);
        }
        if (!leaseOptions.ProposedLeaseIdOptional.empty())
        {
          request.AddHeader(
              Details::c_HeaderProposedLeaseIdOptional, leaseOptions.ProposedLeaseIdOptional);
        }
        if (!leaseOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, leaseOptions.IfMatch);
        }
        if (!leaseOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, leaseOptions.IfNoneMatch);
        }
        if (!leaseOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, leaseOptions.IfModifiedSince);
        }
        if (!leaseOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, leaseOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static PathLeaseResponse LeaseParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // The "renew", "change" or "release" action was successful.
          PathLeaseResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseId)
              != response.GetHeaders().end())
          {
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderXMsLeaseId);
          }
          return result;
        }
        else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Created)
        {
          // A new lease has been created.  The "acquire" action was successful.
          PathLeaseResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseId)
              != response.GetHeaders().end())
          {
            result.LeaseId = response.GetHeaders().at(Details::c_HeaderXMsLeaseId);
          }
          return result;
        }
        else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
        {
          // The "break" lease action was successful.
          PathLeaseResponse result;
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseTime)
              != response.GetHeaders().end())
          {
            result.LeaseTime = response.GetHeaders().at(Details::c_HeaderXMsLeaseTime);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request ReadCreateRequest(
          std::string url,
          const ReadOptions& readOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
        if (!readOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, readOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(readOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, readOptions.ApiVersionParameter);
        if (!readOptions.Range.empty())
        {
          request.AddHeader(Details::c_HeaderRange, readOptions.Range);
        }
        if (!readOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, readOptions.LeaseIdOptional);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddHeader(
            Details::c_HeaderXMsRangeGetContentMd5,
            (readOptions.XMsRangeGetContentMd5 ? "true" : "false"));
        if (!readOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, readOptions.IfMatch);
        }
        if (!readOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, readOptions.IfNoneMatch);
        }
        if (!readOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, readOptions.IfModifiedSince);
        }
        if (!readOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, readOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static PathReadResponse ReadParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Ok
          PathReadResponse result;
          result.BodyStream = response.GetBodyStream();
          if (response.GetHeaders().find(Details::c_HeaderAcceptRanges)
              != response.GetHeaders().end())
          {
            result.AcceptRanges = response.GetHeaders().at(Details::c_HeaderAcceptRanges);
          }
          if (response.GetHeaders().find(Details::c_HeaderCacheControl)
              != response.GetHeaders().end())
          {
            result.CacheControl = response.GetHeaders().at(Details::c_HeaderCacheControl);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
              != response.GetHeaders().end())
          {
            result.ContentDisposition
                = response.GetHeaders().at(Details::c_HeaderContentDisposition);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
              != response.GetHeaders().end())
          {
            result.ContentEncoding = response.GetHeaders().at(Details::c_HeaderContentEncoding);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
              != response.GetHeaders().end())
          {
            result.ContentLanguage = response.GetHeaders().at(Details::c_HeaderContentLanguage);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLength)
              != response.GetHeaders().end())
          {
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
          }
          if (response.GetHeaders().find(Details::c_HeaderContentRange)
              != response.GetHeaders().end())
          {
            result.ContentRange = response.GetHeaders().at(Details::c_HeaderContentRange);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentType)
              != response.GetHeaders().end())
          {
            result.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentMD5)
              != response.GetHeaders().end())
          {
            result.ContentMD5 = response.GetHeaders().at(Details::c_HeaderContentMD5);
          }
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsResourceType)
              != response.GetHeaders().end())
          {
            result.ResourceType = response.GetHeaders().at(Details::c_HeaderXMsResourceType);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsProperties)
              != response.GetHeaders().end())
          {
            result.Properties = response.GetHeaders().at(Details::c_HeaderXMsProperties);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseDuration)
              != response.GetHeaders().end())
          {
            result.LeaseDuration = response.GetHeaders().at(Details::c_HeaderXMsLeaseDuration);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseState)
              != response.GetHeaders().end())
          {
            result.LeaseState = response.GetHeaders().at(Details::c_HeaderXMsLeaseState);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseStatus)
              != response.GetHeaders().end())
          {
            result.LeaseStatus = response.GetHeaders().at(Details::c_HeaderXMsLeaseStatus);
          }
          return result;
        }
        else if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::PartialContent)
        {
          // Partial content
          PathReadResponse result;
          result.BodyStream = response.GetBodyStream();
          if (response.GetHeaders().find(Details::c_HeaderAcceptRanges)
              != response.GetHeaders().end())
          {
            result.AcceptRanges = response.GetHeaders().at(Details::c_HeaderAcceptRanges);
          }
          if (response.GetHeaders().find(Details::c_HeaderCacheControl)
              != response.GetHeaders().end())
          {
            result.CacheControl = response.GetHeaders().at(Details::c_HeaderCacheControl);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
              != response.GetHeaders().end())
          {
            result.ContentDisposition
                = response.GetHeaders().at(Details::c_HeaderContentDisposition);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
              != response.GetHeaders().end())
          {
            result.ContentEncoding = response.GetHeaders().at(Details::c_HeaderContentEncoding);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
              != response.GetHeaders().end())
          {
            result.ContentLanguage = response.GetHeaders().at(Details::c_HeaderContentLanguage);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLength)
              != response.GetHeaders().end())
          {
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
          }
          if (response.GetHeaders().find(Details::c_HeaderContentRange)
              != response.GetHeaders().end())
          {
            result.ContentRange = response.GetHeaders().at(Details::c_HeaderContentRange);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentType)
              != response.GetHeaders().end())
          {
            result.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentMD5)
              != response.GetHeaders().end())
          {
            result.ContentMD5 = response.GetHeaders().at(Details::c_HeaderContentMD5);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContentMd5)
              != response.GetHeaders().end())
          {
            result.XMsContentMd5 = response.GetHeaders().at(Details::c_HeaderXMsContentMd5);
          }
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsResourceType)
              != response.GetHeaders().end())
          {
            result.ResourceType = response.GetHeaders().at(Details::c_HeaderXMsResourceType);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsProperties)
              != response.GetHeaders().end())
          {
            result.Properties = response.GetHeaders().at(Details::c_HeaderXMsProperties);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseDuration)
              != response.GetHeaders().end())
          {
            result.LeaseDuration = response.GetHeaders().at(Details::c_HeaderXMsLeaseDuration);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseState)
              != response.GetHeaders().end())
          {
            result.LeaseState = response.GetHeaders().at(Details::c_HeaderXMsLeaseState);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseStatus)
              != response.GetHeaders().end())
          {
            result.LeaseStatus = response.GetHeaders().at(Details::c_HeaderXMsLeaseStatus);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request GetPropertiesCreateRequest(
          std::string url,
          const GetPropertiesOptions& getPropertiesOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
        if (!getPropertiesOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, getPropertiesOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(getPropertiesOptions.Timeout));
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, getPropertiesOptions.ApiVersionParameter);
        if (getPropertiesOptions.Action != PathGetPropertiesAction::Unknown)
        {
          request.AddQueryParameter(
              Details::c_QueryPathGetPropertiesAction,
              PathGetPropertiesActionToString(getPropertiesOptions.Action));
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryUpn, (getPropertiesOptions.Upn ? "true" : "false"));
        if (!getPropertiesOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, getPropertiesOptions.LeaseIdOptional);
        }
        if (!getPropertiesOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, getPropertiesOptions.IfMatch);
        }
        if (!getPropertiesOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, getPropertiesOptions.IfNoneMatch);
        }
        if (!getPropertiesOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, getPropertiesOptions.IfModifiedSince);
        }
        if (!getPropertiesOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(
              Details::c_HeaderIfUnmodifiedSince, getPropertiesOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static PathGetPropertiesResponse GetPropertiesParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Returns all properties for the file or directory.
          PathGetPropertiesResponse result;
          if (response.GetHeaders().find(Details::c_HeaderAcceptRanges)
              != response.GetHeaders().end())
          {
            result.AcceptRanges = response.GetHeaders().at(Details::c_HeaderAcceptRanges);
          }
          if (response.GetHeaders().find(Details::c_HeaderCacheControl)
              != response.GetHeaders().end())
          {
            result.CacheControl = response.GetHeaders().at(Details::c_HeaderCacheControl);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentDisposition)
              != response.GetHeaders().end())
          {
            result.ContentDisposition
                = response.GetHeaders().at(Details::c_HeaderContentDisposition);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentEncoding)
              != response.GetHeaders().end())
          {
            result.ContentEncoding = response.GetHeaders().at(Details::c_HeaderContentEncoding);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLanguage)
              != response.GetHeaders().end())
          {
            result.ContentLanguage = response.GetHeaders().at(Details::c_HeaderContentLanguage);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLength)
              != response.GetHeaders().end())
          {
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
          }
          if (response.GetHeaders().find(Details::c_HeaderContentRange)
              != response.GetHeaders().end())
          {
            result.ContentRange = response.GetHeaders().at(Details::c_HeaderContentRange);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentType)
              != response.GetHeaders().end())
          {
            result.ContentType = response.GetHeaders().at(Details::c_HeaderContentType);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentMD5)
              != response.GetHeaders().end())
          {
            result.ContentMD5 = response.GetHeaders().at(Details::c_HeaderContentMD5);
          }
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsResourceType)
              != response.GetHeaders().end())
          {
            result.ResourceType = response.GetHeaders().at(Details::c_HeaderXMsResourceType);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsProperties)
              != response.GetHeaders().end())
          {
            result.Properties = response.GetHeaders().at(Details::c_HeaderXMsProperties);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsOwner) != response.GetHeaders().end())
          {
            result.Owner = response.GetHeaders().at(Details::c_HeaderXMsOwner);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsGroup) != response.GetHeaders().end())
          {
            result.Group = response.GetHeaders().at(Details::c_HeaderXMsGroup);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsPermissions)
              != response.GetHeaders().end())
          {
            result.Permissions = response.GetHeaders().at(Details::c_HeaderXMsPermissions);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsAcl) != response.GetHeaders().end())
          {
            result.ACL = response.GetHeaders().at(Details::c_HeaderXMsAcl);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseDuration)
              != response.GetHeaders().end())
          {
            result.LeaseDuration = response.GetHeaders().at(Details::c_HeaderXMsLeaseDuration);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseState)
              != response.GetHeaders().end())
          {
            result.LeaseState = response.GetHeaders().at(Details::c_HeaderXMsLeaseState);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsLeaseStatus)
              != response.GetHeaders().end())
          {
            result.LeaseStatus = response.GetHeaders().at(Details::c_HeaderXMsLeaseStatus);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request DeleteCreateRequest(
          std::string url,
          const DeleteOptions& deleteOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
        if (!deleteOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, deleteOptions.ClientRequestId);
        }
        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(Details::c_QueryTimeout, std::to_string(deleteOptions.Timeout));
        request.AddHeader(Details::c_HeaderApiVersionParameter, deleteOptions.ApiVersionParameter);

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryRecursiveOptional,
            (deleteOptions.RecursiveOptional ? "true" : "false"));
        if (!deleteOptions.Continuation.empty())
        {
          request.AddQueryParameter(Details::c_QueryContinuation, deleteOptions.Continuation);
        }
        if (!deleteOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, deleteOptions.LeaseIdOptional);
        }
        if (!deleteOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, deleteOptions.IfMatch);
        }
        if (!deleteOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, deleteOptions.IfNoneMatch);
        }
        if (!deleteOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, deleteOptions.IfModifiedSince);
        }
        if (!deleteOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, deleteOptions.IfUnmodifiedSince);
        }
        return request;
      }

      static PathDeleteResponse DeleteParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // The file was deleted.
          PathDeleteResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContinuation)
              != response.GetHeaders().end())
          {
            result.Continuation = response.GetHeaders().at(Details::c_HeaderXMsContinuation);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request SetAccessControlCreateRequest(
          std::string url,
          const SetAccessControlOptions& setAccessControlOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
        request.AddQueryParameter(Details::c_QueryAction, "setAccessControl");

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(setAccessControlOptions.Timeout));
        if (!setAccessControlOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(
              Details::c_HeaderLeaseIdOptional, setAccessControlOptions.LeaseIdOptional);
        }
        if (!setAccessControlOptions.Owner.empty())
        {
          request.AddHeader(Details::c_HeaderOwner, setAccessControlOptions.Owner);
        }
        if (!setAccessControlOptions.Group.empty())
        {
          request.AddHeader(Details::c_HeaderGroup, setAccessControlOptions.Group);
        }
        if (!setAccessControlOptions.Permissions.empty())
        {
          request.AddHeader(Details::c_HeaderPermissions, setAccessControlOptions.Permissions);
        }
        if (!setAccessControlOptions.Acl.empty())
        {
          request.AddHeader(Details::c_HeaderAcl, setAccessControlOptions.Acl);
        }
        if (!setAccessControlOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, setAccessControlOptions.IfMatch);
        }
        if (!setAccessControlOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, setAccessControlOptions.IfNoneMatch);
        }
        if (!setAccessControlOptions.IfModifiedSince.empty())
        {
          request.AddHeader(
              Details::c_HeaderIfModifiedSince, setAccessControlOptions.IfModifiedSince);
        }
        if (!setAccessControlOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(
              Details::c_HeaderIfUnmodifiedSince, setAccessControlOptions.IfUnmodifiedSince);
        }
        if (!setAccessControlOptions.ClientRequestId.empty())
        {
          request.AddHeader(
              Details::c_HeaderClientRequestId, setAccessControlOptions.ClientRequestId);
        }
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, setAccessControlOptions.ApiVersionParameter);
        return request;
      }

      static PathSetAccessControlResponse SetAccessControlParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Set directory access control response.
          PathSetAccessControlResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsClientRequestId)
              != response.GetHeaders().end())
          {
            result.ClientRequestId = response.GetHeaders().at(Details::c_HeaderXMsClientRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request SetAccessControlRecursiveCreateRequest(
          std::string url,
          const SetAccessControlRecursiveOptions& setAccessControlRecursiveOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
        request.AddQueryParameter(Details::c_QueryAction, "setAccessControlRecursive");

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(setAccessControlRecursiveOptions.Timeout));
        if (!setAccessControlRecursiveOptions.Continuation.empty())
        {
          request.AddQueryParameter(
              Details::c_QueryContinuation, setAccessControlRecursiveOptions.Continuation);
        }
        request.AddQueryParameter(
            Details::c_QueryPathSetAccessControlRecursiveMode,
            PathSetAccessControlRecursiveModeToString(setAccessControlRecursiveOptions.Mode));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryMaxRecords,
            std::to_string(setAccessControlRecursiveOptions.MaxRecords));
        if (!setAccessControlRecursiveOptions.Acl.empty())
        {
          request.AddHeader(Details::c_HeaderAcl, setAccessControlRecursiveOptions.Acl);
        }
        if (!setAccessControlRecursiveOptions.ClientRequestId.empty())
        {
          request.AddHeader(
              Details::c_HeaderClientRequestId, setAccessControlRecursiveOptions.ClientRequestId);
        }
        request.AddHeader(
            Details::c_HeaderApiVersionParameter,
            setAccessControlRecursiveOptions.ApiVersionParameter);
        return request;
      }

      static PathSetAccessControlRecursiveResponse SetAccessControlRecursiveParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // Set directory access control recursive response.
          PathSetAccessControlRecursiveResponse result = PathSetAccessControlRecursiveResponse::
              PathSetAccessControlRecursiveResponseFromSetAccessControlRecursiveResponse(
                  SetAccessControlRecursiveResponse::CreateFromJson(nlohmann::json::parse(
                      *Azure::Core::Http::Response::ConstructBodyBufferFromStream(
                          response.GetBodyStream().get()))));
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsClientRequestId)
              != response.GetHeaders().end())
          {
            result.ClientRequestId = response.GetHeaders().at(Details::c_HeaderXMsClientRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsContinuation)
              != response.GetHeaders().end())
          {
            result.Continuation = response.GetHeaders().at(Details::c_HeaderXMsContinuation);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request FlushDataCreateRequest(
          std::string url,
          const FlushDataOptions& flushDataOptions)
      {
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
        request.AddQueryParameter(Details::c_QueryAction, "flush");

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(flushDataOptions.Timeout));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryPosition, std::to_string(flushDataOptions.Position));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryRetainUncommittedData,
            (flushDataOptions.RetainUncommittedData ? "true" : "false"));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryClose, (flushDataOptions.Close ? "true" : "false"));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddHeader(
            Details::c_HeaderContentLength, std::to_string(flushDataOptions.ContentLength));
        if (!flushDataOptions.ContentMD5.empty())
        {
          request.AddHeader(Details::c_HeaderContentMD5, flushDataOptions.ContentMD5);
        }
        if (!flushDataOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, flushDataOptions.LeaseIdOptional);
        }
        if (!flushDataOptions.CacheControl.empty())
        {
          request.AddHeader(Details::c_HeaderCacheControl, flushDataOptions.CacheControl);
        }
        if (!flushDataOptions.ContentType.empty())
        {
          request.AddHeader(Details::c_HeaderContentType, flushDataOptions.ContentType);
        }
        if (!flushDataOptions.ContentDisposition.empty())
        {
          request.AddHeader(
              Details::c_HeaderContentDisposition, flushDataOptions.ContentDisposition);
        }
        if (!flushDataOptions.ContentEncoding.empty())
        {
          request.AddHeader(Details::c_HeaderContentEncoding, flushDataOptions.ContentEncoding);
        }
        if (!flushDataOptions.ContentLanguage.empty())
        {
          request.AddHeader(Details::c_HeaderContentLanguage, flushDataOptions.ContentLanguage);
        }
        if (!flushDataOptions.IfMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfMatch, flushDataOptions.IfMatch);
        }
        if (!flushDataOptions.IfNoneMatch.empty())
        {
          request.AddHeader(Details::c_HeaderIfNoneMatch, flushDataOptions.IfNoneMatch);
        }
        if (!flushDataOptions.IfModifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfModifiedSince, flushDataOptions.IfModifiedSince);
        }
        if (!flushDataOptions.IfUnmodifiedSince.empty())
        {
          request.AddHeader(Details::c_HeaderIfUnmodifiedSince, flushDataOptions.IfUnmodifiedSince);
        }
        if (!flushDataOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, flushDataOptions.ClientRequestId);
        }
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, flushDataOptions.ApiVersionParameter);
        return request;
      }

      static PathFlushDataResponse FlushDataParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
        {
          // The data was flushed (written) to the file successfully.
          PathFlushDataResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderETag) != response.GetHeaders().end())
          {
            result.ETag = response.GetHeaders().at(Details::c_HeaderETag);
          }
          if (response.GetHeaders().find(Details::c_HeaderLastModified)
              != response.GetHeaders().end())
          {
            result.LastModified = response.GetHeaders().at(Details::c_HeaderLastModified);
          }
          if (response.GetHeaders().find(Details::c_HeaderContentLength)
              != response.GetHeaders().end())
          {
            result.ContentLength
                = std::stoll(response.GetHeaders().at(Details::c_HeaderContentLength));
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsClientRequestId)
              != response.GetHeaders().end())
          {
            result.ClientRequestId = response.GetHeaders().at(Details::c_HeaderXMsClientRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }

      static Azure::Core::Http::Request AppendDataCreateRequest(
          std::string url,
          std::unique_ptr<Azure::Core::Http::BodyStream> content,
          const AppendDataOptions& appendDataOptions)
      {
        Azure::Core::Http::Request request(
            Azure::Core::Http::HttpMethod::Patch, std::move(url), std::move(content));
        request.AddQueryParameter(Details::c_QueryAction, "append");

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryPosition, std::to_string(appendDataOptions.Position));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddQueryParameter(
            Details::c_QueryTimeout, std::to_string(appendDataOptions.Timeout));

        // TODO: Need to check for Null when Nullable<T> is ready
        request.AddHeader(
            Details::c_HeaderContentLength, std::to_string(appendDataOptions.ContentLength));
        if (!appendDataOptions.TransactionalContentMD5.empty())
        {
          request.AddHeader(
              Details::c_HeaderTransactionalContentMD5, appendDataOptions.TransactionalContentMD5);
        }
        if (!appendDataOptions.LeaseIdOptional.empty())
        {
          request.AddHeader(Details::c_HeaderLeaseIdOptional, appendDataOptions.LeaseIdOptional);
        }
        if (!appendDataOptions.ClientRequestId.empty())
        {
          request.AddHeader(Details::c_HeaderClientRequestId, appendDataOptions.ClientRequestId);
        }
        request.AddHeader(
            Details::c_HeaderApiVersionParameter, appendDataOptions.ApiVersionParameter);
        return request;
      }

      static PathAppendDataResponse AppendDataParseResponse(
          std::unique_ptr<Azure::Core::Http::Response> responsePtr)
      {
        /* const */ auto& response = *responsePtr;
        if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
        {
          // Append data to file control response.
          PathAppendDataResponse result;
          if (response.GetHeaders().find(Details::c_HeaderDate) != response.GetHeaders().end())
          {
            result.Date = response.GetHeaders().at(Details::c_HeaderDate);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsRequestId)
              != response.GetHeaders().end())
          {
            result.RequestId = response.GetHeaders().at(Details::c_HeaderXMsRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsClientRequestId)
              != response.GetHeaders().end())
          {
            result.ClientRequestId = response.GetHeaders().at(Details::c_HeaderXMsClientRequestId);
          }
          if (response.GetHeaders().find(Details::c_HeaderXMsVersion)
              != response.GetHeaders().end())
          {
            result.Version = response.GetHeaders().at(Details::c_HeaderXMsVersion);
          }
          return result;
        }
        else
        {
          throw Azure::Storage::StorageError::CreateFromResponse(std::move(responsePtr));
        }
      }
    };

  }; // class DataLakeRestClient

}}} // namespace Azure::Storage::DataLake
