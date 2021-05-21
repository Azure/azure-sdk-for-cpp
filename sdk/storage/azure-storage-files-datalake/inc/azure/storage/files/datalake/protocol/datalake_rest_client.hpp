
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
#include <azure/core/internal/json/json.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace Models {

    /**
     * @brief The common HTTP headers.
     */
    struct PathHttpHeaders final
    {

      /**
       * The cache control of the content.
       */
      std::string CacheControl;

      /**
       * The disposition of the content.
       */
      std::string ContentDisposition;

      /**
       * The encoding of the content.
       */
      std::string ContentEncoding;

      /**
       * The language of the content.
       */
      std::string ContentLanguage;

      /**
       * The type of the content.
       */
      std::string ContentType;

      /**
       * The hash of the content.
       */
      Storage::ContentHash ContentHash;
    };

    /**
     * @brief The failed entries when setting the Acl.
     */
    struct AclFailedEntry final
    {
      /**
       * The name of the failed entry.
       */
      std::string Name;

      /**
       * The type of the failure.
       */
      std::string Type;

      /**
       * The error message of the failure.
       */
      std::string ErrorMessage;
    };

    struct PathItem final
    {
      /**
       * The name of the path item.
       */
      std::string Name;

      /**
       * A boolean that indicates if the path is a directory.
       */
      bool IsDirectory = bool();

      /**
       * The data and time the file or directory was last modified.  Write operations on the file or
       * directory update the last modified time.
       */
      DateTime LastModified;

      /**
       * An HTTP entity tag associated with the file or directory.
       */
      std::string ETag;

      /**
       * The size of the file.
       */
      int64_t FileSize = int64_t();

      /**
       * The owner of the file.
       */
      std::string Owner;

      /**
       * The group of the file.
       */
      std::string Group;

      /**
       * The permission of the file.
       */
      std::string Permissions;
    };

    /**
     * @brief The public access type of a file system.
     */
    class PublicAccessType final {
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

    /**
     * @brief Required only for Create File and Create Directory. The value must be "file" or
     * "directory".
     */
    class PathResourceType final {
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

    /**
     * @brief When a resource is leased, specifies whether the lease is of infinite or fixed
     * duration.
     */
    class LeaseDuration final {
    public:
      LeaseDuration() = default;
      explicit LeaseDuration(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseDuration& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseDuration& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseDuration Infinite;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseDuration Fixed;

    private:
      std::string m_value;
    }; // extensible enum LeaseDuration

    /**
     * @brief Lease state of the resource.
     */
    class LeaseState final {
    public:
      LeaseState() = default;
      explicit LeaseState(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseState& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseState& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseState Available;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseState Leased;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseState Expired;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseState Breaking;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseState Broken;

    private:
      std::string m_value;
    }; // extensible enum LeaseState

    /**
     * @brief The lease status of the resource.
     */
    class LeaseStatus final {
    public:
      LeaseStatus() = default;
      explicit LeaseStatus(std::string value) : m_value(std::move(value)) {}
      bool operator==(const LeaseStatus& other) const { return m_value == other.m_value; }
      bool operator!=(const LeaseStatus& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStatus Locked;
      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static LeaseStatus Unlocked;

    private:
      std::string m_value;
    }; // extensible enum LeaseStatus

    /**
     * @brief The serialized return result for operation: SetPathAccessControlList
     */
    struct SetPathAccessControlListResult final
    {

      /**
       * An HTTP entity tag associated with the file or directory.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file or directory was last modified. Write operations on the file or
       * directory update the last modified time.
       */
      DateTime LastModified;
    };

    /**
     * @brief The serialized return result for operation: FlushFile
     */
    struct FlushFileResult final
    {

      /**
       * An HTTP entity tag associated with the file or directory.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file or directory was last modified.  Write operations on the file or
       * directory update the last modified time.
       */
      DateTime LastModified;

      /**
       * The size of the resource in bytes.
       */
      int64_t FileSize = int64_t();
    };

    /**
     * @brief The serialized return result for operation: AppendFile
     */
    struct AppendFileResult final
    {

      /**
       * If the blob has an MD5 hash and this operation is to read the full blob, this response
       * header is returned so that the client can check for message content integrity.
       */
      Azure::Nullable<Storage::ContentHash> TransactionalContentHash;

      /**
       * A boolean that indicates if the server is encrypted.
       */
      bool IsServerEncrypted = bool();
    };

  } // namespace Models
  namespace _detail {
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

    /**
     * @brief The value must be "filesystem" for all filesystem operations.
     */
    class FileSystemResource final {
    public:
      FileSystemResource() = default;
      explicit FileSystemResource(std::string value) : m_value(std::move(value)) {}
      bool operator==(const FileSystemResource& other) const { return m_value == other.m_value; }
      bool operator!=(const FileSystemResource& other) const { return !(*this == other); }
      const std::string& ToString() const { return m_value; }

      AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static FileSystemResource Filesystem;

    private:
      std::string m_value;
    }; // extensible enum FileSystemResource

    /**
     * @brief Mode "set" sets POSIX access control rights on files and directories, "modify"
     * modifies one or more POSIX access control rights  that pre-exist on files and directories,
     * "remove" removes one or more POSIX access control rights  that were present earlier on files
     * and directories
     */
    class PathSetAccessControlRecursiveMode final {
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

    struct SetAccessControlRecursiveResponse final
    {
      int32_t NumberOfSuccessfulDirectories = int32_t();

      int32_t NumberOfSuccessfulFiles = int32_t();

      int32_t NumberOfFailures = int32_t();

      std::vector<AclFailedEntry> FailedEntries;
    };

    /**
     * @brief The items returned when listing paths.
     */
    struct PathList final
    {
      std::vector<PathItem> Items;
    };

    /**
     * @brief Optional. Valid only when namespace is enabled. This parameter determines the behavior
     * of the rename operation. The value must be "legacy" or "posix", and the default value will be
     * "posix".
     */
    class PathRenameMode final {
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

    /**
     * @brief Optional. If the value is "getStatus" only the system defined properties for the path
     * are returned. If the value is "getAccessControl" the access control list is returned in the
     * response headers (Hierarchical Namespace must be enabled for the account), otherwise the
     * properties are returned.
     */
    class PathGetPropertiesAction final {
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

    struct FileSystemListPathsResult final
    {
      std::vector<PathItem> Items;

      /**
       * If the number of paths to be listed exceeds the maxResults limit, a continuation token is
       * returned in this response header.  When a continuation token is returned in the response,
       * it must be specified in a subsequent invocation of the list operation to continue listing
       * the paths.
       */
      Azure::Nullable<std::string> ContinuationToken;
    };

    struct PathCreateResult final
    {

      /**
       * An HTTP entity tag associated with the file or directory.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file or directory was last modified.  Write operations on the file or
       * directory update the last modified time.
       */
      Azure::Nullable<DateTime> LastModified;

      /**
       * The size of the resource in bytes.
       */
      Azure::Nullable<int64_t> ContentLength;
    };

    struct PathGetPropertiesResult final
    {

      /**
       * Indicates that the service supports requests for partial file content.
       */
      Azure::Nullable<std::string> AcceptRanges;

      /**
       * The HTTP headers of the object.
       */
      PathHttpHeaders HttpHeaders;

      /**
       * An HTTP entity tag associated with the file or directory.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file or directory was last modified.  Write operations on the file or
       * directory update the last modified time.
       */
      DateTime LastModified;

      /**
       * The type of the resource.  The value may be "file" or "directory".  If not set, the value
       * is "file".
       */
      Azure::Nullable<std::string> ResourceType;

      /**
       * The user-defined properties associated with the file or directory, in the format of a
       * comma-separated list of name and value pairs "n1=v1, n2=v2, ...", where each value is a
       * Base64 encoded string. Note that the string may only contain ASCII characters in the
       * ISO-8859-1 character set.
       */
      Azure::Nullable<std::string> Properties;

      /**
       * The owner of the file or directory. Included in the response if Hierarchical Namespace is
       * enabled for the account.
       */
      Azure::Nullable<std::string> Owner;

      /**
       * The owning group of the file or directory. Included in the response if Hierarchical
       * Namespace is enabled for the account.
       */
      Azure::Nullable<std::string> Group;

      /**
       * The POSIX access permissions for the file owner, the file owning group, and others.
       * Included in the response if Hierarchical Namespace is enabled for the account.
       */
      Azure::Nullable<std::string> Permissions;

      /**
       * The POSIX access control list for the file or directory.  Included in the response only if
       * the action is "getAccessControl" and Hierarchical Namespace is enabled for the account.
       */
      Azure::Nullable<std::string> Acl;

      /**
       * When a resource is leased, specifies whether the lease is of infinite or fixed duration.
       */
      Azure::Nullable<Models::LeaseDuration> LeaseDuration;

      /**
       * Lease state of the resource.
       */
      Azure::Nullable<Models::LeaseState> LeaseState;

      /**
       * The lease status of the resource.
       */
      Azure::Nullable<Models::LeaseStatus> LeaseStatus;
    };

    struct PathDeleteResult final
    {

      /**
       * When deleting a directory, the number of paths that are deleted with each invocation is
       * limited.  If the number of paths to be deleted exceeds this limit, a continuation token is
       * returned in this response header.  When a continuation token is returned in the response,
       * it must be specified in a subsequent invocation of the delete operation to continue
       * deleting the directory.
       */
      Azure::Nullable<std::string> ContinuationToken;
    };

    struct PathSetAccessControlRecursiveResult final
    {
      int32_t NumberOfSuccessfulDirectories = int32_t();
      int32_t NumberOfSuccessfulFiles = int32_t();
      int32_t NumberOfFailures = int32_t();
      std::vector<AclFailedEntry> FailedEntries;

      /**
       * When performing setAccessControlRecursive on a directory, the number of paths that are
       * processed with each invocation is limited.  If the number of paths to be processed exceeds
       * this limit, a continuation token is returned in this response header.  When a continuation
       * token is returned in the response, it must be specified in a subsequent invocation of the
       * setAccessControlRecursive operation to continue the setAccessControlRecursive operation on
       * the directory.
       */
      Azure::Nullable<std::string> ContinuationToken;
    };

    class DataLakeRestClient final {
    public:
      class FileSystem final {
      public:
        struct ListPathsOptions final
        {
          FileSystemResource Resource;
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<std::string> Directory;
          bool RecursiveRequired = bool();
          Azure::Nullable<int32_t> MaxResults;
          Azure::Nullable<bool> Upn;
        };

        static Azure::Response<FileSystemListPathsResult> ListPaths(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const ListPathsOptions& listPathsOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
          request.GetUrl().AppendQueryParameter(
              _detail::QueryFileSystemResource,
              _internal::UrlEncodeQueryParameter(listPathsOptions.Resource.ToString()));
          if (listPathsOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listPathsOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, listPathsOptions.ApiVersionParameter);
          if (listPathsOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(listPathsOptions.ContinuationToken.Value()));
          }
          if (listPathsOptions.Directory.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPath,
                _internal::UrlEncodeQueryParameter(listPathsOptions.Directory.Value()));
          }
          request.GetUrl().AppendQueryParameter(
              _detail::QueryRecursive,
              _internal::UrlEncodeQueryParameter(
                  (listPathsOptions.RecursiveRequired ? "true" : "false")));
          if (listPathsOptions.MaxResults.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPageSizeHint,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(listPathsOptions.MaxResults.Value())));
          }
          if (listPathsOptions.Upn.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryUpn,
                _internal::UrlEncodeQueryParameter(
                    (listPathsOptions.Upn.Value() ? "true" : "false")));
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
                    PathListFromJson(Azure::Core::Json::_internal::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(_detail::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(_detail::HeaderContinuationToken);
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

        static PathItem PathItemFromJson(const Azure::Core::Json::_internal::json& node)
        {
          PathItem result;
          result.Name = node["name"].get<std::string>();
          if (node.contains("isDirectory"))
          {
            result.IsDirectory = (node["isDirectory"].get<std::string>() == "true");
          }
          result.LastModified = DateTime::Parse(
              node["lastModified"].get<std::string>(), DateTime::DateFormat::Rfc1123);
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

        static PathList PathListFromJson(const Azure::Core::Json::_internal::json& node)
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

      class Path final {
      public:
        struct CreateOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<PathResourceType> Resource;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<PathRenameMode> Mode;
          Azure::Nullable<std::string> CacheControl;
          Azure::Nullable<std::string> ContentEncoding;
          Azure::Nullable<std::string> ContentLanguage;
          Azure::Nullable<std::string> ContentDisposition;
          Azure::Nullable<std::string> ContentType;
          Azure::Nullable<std::string> RenameSource;
          Azure::Nullable<std::string> LeaseIdOptional;
          Azure::Nullable<std::string> SourceLeaseId;
          Azure::Nullable<std::string> Properties;
          Azure::Nullable<std::string> Permissions;
          Azure::Nullable<std::string> Umask;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<DateTime> IfModifiedSince;
          Azure::Nullable<DateTime> IfUnmodifiedSince;
          Azure::ETag SourceIfMatch;
          Azure::ETag SourceIfNoneMatch;
          Azure::Nullable<DateTime> SourceIfModifiedSince;
          Azure::Nullable<DateTime> SourceIfUnmodifiedSince;
        };

        static Azure::Response<PathCreateResult> Create(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const CreateOptions& createOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url);
          request.SetHeader(_detail::HeaderContentLength, "0");
          if (createOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(createOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, createOptions.ApiVersionParameter);
          if (createOptions.Resource.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPathResourceType,
                _internal::UrlEncodeQueryParameter(createOptions.Resource.Value().ToString()));
          }
          if (createOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(createOptions.ContinuationToken.Value()));
          }
          if (createOptions.Mode.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPathRenameMode,
                _internal::UrlEncodeQueryParameter(createOptions.Mode.Value().ToString()));
          }
          if (createOptions.CacheControl.HasValue())
          {
            request.SetHeader(_detail::HeaderCacheControl, createOptions.CacheControl.Value());
          }
          if (createOptions.ContentEncoding.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentEncoding, createOptions.ContentEncoding.Value());
          }
          if (createOptions.ContentLanguage.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentLanguage, createOptions.ContentLanguage.Value());
          }
          if (createOptions.ContentDisposition.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentDisposition, createOptions.ContentDisposition.Value());
          }
          if (createOptions.ContentType.HasValue())
          {
            request.SetHeader(_detail::HeaderContentType, createOptions.ContentType.Value());
          }
          if (createOptions.RenameSource.HasValue())
          {
            request.SetHeader(_detail::HeaderRenameSource, createOptions.RenameSource.Value());
          }
          if (createOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, createOptions.LeaseIdOptional.Value());
          }
          if (createOptions.SourceLeaseId.HasValue())
          {
            request.SetHeader(_detail::HeaderSourceLeaseId, createOptions.SourceLeaseId.Value());
          }
          if (createOptions.Properties.HasValue())
          {
            request.SetHeader(_detail::HeaderProperties, createOptions.Properties.Value());
          }
          if (createOptions.Permissions.HasValue())
          {
            request.SetHeader(_detail::HeaderPermissions, createOptions.Permissions.Value());
          }
          if (createOptions.Umask.HasValue())
          {
            request.SetHeader(_detail::HeaderUmask, createOptions.Umask.Value());
          }
          if (createOptions.IfMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfMatch, createOptions.IfMatch.ToString());
          }
          if (createOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfNoneMatch, createOptions.IfNoneMatch.ToString());
          }
          if (createOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfModifiedSince,
                createOptions.IfModifiedSince.Value().ToString(DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfUnmodifiedSince,
                createOptions.IfUnmodifiedSince.Value().ToString(DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.SourceIfMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderSourceIfMatch, createOptions.SourceIfMatch.ToString());
          }
          if (createOptions.SourceIfNoneMatch.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceIfNoneMatch, createOptions.SourceIfNoneMatch.ToString());
          }
          if (createOptions.SourceIfModifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceIfModifiedSince,
                createOptions.SourceIfModifiedSince.Value().ToString(
                    DateTime::DateFormat::Rfc1123));
          }
          if (createOptions.SourceIfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderSourceIfUnmodifiedSince,
                createOptions.SourceIfUnmodifiedSince.Value().ToString(
                    DateTime::DateFormat::Rfc1123));
          }
          return CreateParseResult(context, pipeline.Send(request, context));
        }

        struct GetPropertiesOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<PathGetPropertiesAction> Action;
          Azure::Nullable<bool> Upn;
          Azure::Nullable<std::string> LeaseIdOptional;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<DateTime> IfModifiedSince;
          Azure::Nullable<DateTime> IfUnmodifiedSince;
        };

        static Azure::Response<PathGetPropertiesResult> GetProperties(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const GetPropertiesOptions& getPropertiesOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Head, url);
          if (getPropertiesOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(getPropertiesOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, getPropertiesOptions.ApiVersionParameter);
          if (getPropertiesOptions.Action.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPathGetPropertiesAction,
                _internal::UrlEncodeQueryParameter(getPropertiesOptions.Action.Value().ToString()));
          }
          if (getPropertiesOptions.Upn.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryUpn,
                _internal::UrlEncodeQueryParameter(
                    (getPropertiesOptions.Upn.Value() ? "true" : "false")));
          }
          if (getPropertiesOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, getPropertiesOptions.LeaseIdOptional.Value());
          }
          if (getPropertiesOptions.IfMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfMatch, getPropertiesOptions.IfMatch.ToString());
          }
          if (getPropertiesOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfNoneMatch, getPropertiesOptions.IfNoneMatch.ToString());
          }
          if (getPropertiesOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfModifiedSince,
                getPropertiesOptions.IfModifiedSince.Value().ToString(
                    DateTime::DateFormat::Rfc1123));
          }
          if (getPropertiesOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfUnmodifiedSince,
                getPropertiesOptions.IfUnmodifiedSince.Value().ToString(
                    DateTime::DateFormat::Rfc1123));
          }
          return GetPropertiesParseResult(context, pipeline.Send(request, context));
        }

        struct DeleteOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
          Azure::Nullable<bool> RecursiveOptional;
          Azure::Nullable<std::string> ContinuationToken;
          Azure::Nullable<std::string> LeaseIdOptional;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<DateTime> IfModifiedSince;
          Azure::Nullable<DateTime> IfUnmodifiedSince;
        };

        static Azure::Response<PathDeleteResult> Delete(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const DeleteOptions& deleteOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, url);
          if (deleteOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(std::to_string(deleteOptions.Timeout.Value())));
          }
          request.SetHeader(_detail::HeaderVersion, deleteOptions.ApiVersionParameter);
          if (deleteOptions.RecursiveOptional.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryRecursive,
                _internal::UrlEncodeQueryParameter(
                    (deleteOptions.RecursiveOptional.Value() ? "true" : "false")));
          }
          if (deleteOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(deleteOptions.ContinuationToken.Value()));
          }
          if (deleteOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, deleteOptions.LeaseIdOptional.Value());
          }
          if (deleteOptions.IfMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfMatch, deleteOptions.IfMatch.ToString());
          }
          if (deleteOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfNoneMatch, deleteOptions.IfNoneMatch.ToString());
          }
          if (deleteOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfModifiedSince,
                deleteOptions.IfModifiedSince.Value().ToString(DateTime::DateFormat::Rfc1123));
          }
          if (deleteOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfUnmodifiedSince,
                deleteOptions.IfUnmodifiedSince.Value().ToString(DateTime::DateFormat::Rfc1123));
          }
          return DeleteParseResult(context, pipeline.Send(request, context));
        }

        struct SetAccessControlOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> LeaseIdOptional;
          Azure::Nullable<std::string> Owner;
          Azure::Nullable<std::string> Group;
          Azure::Nullable<std::string> Permissions;
          Azure::Nullable<std::string> Acl;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<DateTime> IfModifiedSince;
          Azure::Nullable<DateTime> IfUnmodifiedSince;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<Models::SetPathAccessControlListResult> SetAccessControl(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetAccessControlOptions& setAccessControlOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryAction, "setAccessControl");
          if (setAccessControlOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setAccessControlOptions.Timeout.Value())));
          }
          if (setAccessControlOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(
                _detail::HeaderLeaseId, setAccessControlOptions.LeaseIdOptional.Value());
          }
          if (setAccessControlOptions.Owner.HasValue())
          {
            request.SetHeader(_detail::HeaderOwner, setAccessControlOptions.Owner.Value());
          }
          if (setAccessControlOptions.Group.HasValue())
          {
            request.SetHeader(_detail::HeaderGroup, setAccessControlOptions.Group.Value());
          }
          if (setAccessControlOptions.Permissions.HasValue())
          {
            request.SetHeader(
                _detail::HeaderPermissions, setAccessControlOptions.Permissions.Value());
          }
          if (setAccessControlOptions.Acl.HasValue())
          {
            request.SetHeader(_detail::HeaderAcl, setAccessControlOptions.Acl.Value());
          }
          if (setAccessControlOptions.IfMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfMatch, setAccessControlOptions.IfMatch.ToString());
          }
          if (setAccessControlOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfNoneMatch, setAccessControlOptions.IfNoneMatch.ToString());
          }
          if (setAccessControlOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfModifiedSince,
                setAccessControlOptions.IfModifiedSince.Value().ToString(
                    DateTime::DateFormat::Rfc1123));
          }
          if (setAccessControlOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfUnmodifiedSince,
                setAccessControlOptions.IfUnmodifiedSince.Value().ToString(
                    DateTime::DateFormat::Rfc1123));
          }
          request.SetHeader(_detail::HeaderVersion, setAccessControlOptions.ApiVersionParameter);
          return SetAccessControlParseResult(context, pipeline.Send(request, context));
        }

        struct SetAccessControlRecursiveOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<std::string> ContinuationToken;
          PathSetAccessControlRecursiveMode Mode;
          Azure::Nullable<bool> ForceFlag;
          Azure::Nullable<int32_t> MaxRecords;
          Azure::Nullable<std::string> Acl;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<PathSetAccessControlRecursiveResult> SetAccessControlRecursive(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const SetAccessControlRecursiveOptions& setAccessControlRecursiveOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryAction, "setAccessControlRecursive");
          if (setAccessControlRecursiveOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setAccessControlRecursiveOptions.Timeout.Value())));
          }
          if (setAccessControlRecursiveOptions.ContinuationToken.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryContinuationToken,
                _internal::UrlEncodeQueryParameter(
                    setAccessControlRecursiveOptions.ContinuationToken.Value()));
          }
          request.GetUrl().AppendQueryParameter(
              _detail::QueryPathSetAccessControlRecursiveMode,
              _internal::UrlEncodeQueryParameter(setAccessControlRecursiveOptions.Mode.ToString()));
          if (setAccessControlRecursiveOptions.ForceFlag.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryForceFlag,
                _internal::UrlEncodeQueryParameter(
                    (setAccessControlRecursiveOptions.ForceFlag.Value() ? "true" : "false")));
          }
          if (setAccessControlRecursiveOptions.MaxRecords.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryMaxRecords,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(setAccessControlRecursiveOptions.MaxRecords.Value())));
          }
          if (setAccessControlRecursiveOptions.Acl.HasValue())
          {
            request.SetHeader(_detail::HeaderAcl, setAccessControlRecursiveOptions.Acl.Value());
          }
          request.SetHeader(
              _detail::HeaderVersion, setAccessControlRecursiveOptions.ApiVersionParameter);
          return SetAccessControlRecursiveParseResult(context, pipeline.Send(request, context));
        }

        struct FlushDataOptions final
        {
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<int64_t> Position;
          Azure::Nullable<bool> RetainUncommittedData;
          Azure::Nullable<bool> Close;
          Azure::Nullable<int64_t> ContentLength;
          Azure::Nullable<Storage::ContentHash> ContentMd5;
          Azure::Nullable<std::string> LeaseIdOptional;
          Azure::Nullable<std::string> CacheControl;
          Azure::Nullable<std::string> ContentType;
          Azure::Nullable<std::string> ContentDisposition;
          Azure::Nullable<std::string> ContentEncoding;
          Azure::Nullable<std::string> ContentLanguage;
          Azure::ETag IfMatch;
          Azure::ETag IfNoneMatch;
          Azure::Nullable<DateTime> IfModifiedSince;
          Azure::Nullable<DateTime> IfUnmodifiedSince;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<Models::FlushFileResult> FlushData(
            const Azure::Core::Url& url,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const FlushDataOptions& flushDataOptions)
        {
          Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Patch, url);
          request.GetUrl().AppendQueryParameter(_detail::QueryAction, "flush");
          if (flushDataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(flushDataOptions.Timeout.Value())));
          }
          if (flushDataOptions.Position.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPosition,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(flushDataOptions.Position.Value())));
          }
          if (flushDataOptions.RetainUncommittedData.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryRetainUncommittedData,
                _internal::UrlEncodeQueryParameter(
                    (flushDataOptions.RetainUncommittedData.Value() ? "true" : "false")));
          }
          if (flushDataOptions.Close.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryClose,
                _internal::UrlEncodeQueryParameter(
                    (flushDataOptions.Close.Value() ? "true" : "false")));
          }
          if (flushDataOptions.ContentLength.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentLength,
                std::to_string(flushDataOptions.ContentLength.Value()));
          }
          if (flushDataOptions.ContentMd5.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentHashMd5,
                _internal::ToBase64String(flushDataOptions.ContentMd5.Value()));
          }
          if (flushDataOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, flushDataOptions.LeaseIdOptional.Value());
          }
          if (flushDataOptions.CacheControl.HasValue())
          {
            request.SetHeader(_detail::HeaderCacheControl, flushDataOptions.CacheControl.Value());
          }
          if (flushDataOptions.ContentType.HasValue())
          {
            request.SetHeader(_detail::HeaderContentType, flushDataOptions.ContentType.Value());
          }
          if (flushDataOptions.ContentDisposition.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentDisposition, flushDataOptions.ContentDisposition.Value());
          }
          if (flushDataOptions.ContentEncoding.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentEncoding, flushDataOptions.ContentEncoding.Value());
          }
          if (flushDataOptions.ContentLanguage.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentLanguage, flushDataOptions.ContentLanguage.Value());
          }
          if (flushDataOptions.IfMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfMatch, flushDataOptions.IfMatch.ToString());
          }
          if (flushDataOptions.IfNoneMatch.HasValue())
          {
            request.SetHeader(_detail::HeaderIfNoneMatch, flushDataOptions.IfNoneMatch.ToString());
          }
          if (flushDataOptions.IfModifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfModifiedSince,
                flushDataOptions.IfModifiedSince.Value().ToString(DateTime::DateFormat::Rfc1123));
          }
          if (flushDataOptions.IfUnmodifiedSince.HasValue())
          {
            request.SetHeader(
                _detail::HeaderIfUnmodifiedSince,
                flushDataOptions.IfUnmodifiedSince.Value().ToString(DateTime::DateFormat::Rfc1123));
          }
          request.SetHeader(_detail::HeaderVersion, flushDataOptions.ApiVersionParameter);
          return FlushDataParseResult(context, pipeline.Send(request, context));
        }

        struct AppendDataOptions final
        {
          Azure::Nullable<int64_t> Position;
          Azure::Nullable<int32_t> Timeout;
          Azure::Nullable<int64_t> ContentLength;
          Azure::Nullable<Storage::ContentHash> TransactionalContentMd5;
          Azure::Nullable<Storage::ContentHash> TransactionalContentCrc64;
          Azure::Nullable<std::string> LeaseIdOptional;
          std::string ApiVersionParameter = _detail::DefaultServiceApiVersion;
        };

        static Azure::Response<Models::AppendFileResult> AppendData(
            const Azure::Core::Url& url,
            Azure::Core::IO::BodyStream& bodyStream,
            Azure::Core::Http::_internal::HttpPipeline& pipeline,
            Azure::Core::Context context,
            const AppendDataOptions& appendDataOptions)
        {
          Azure::Core::Http::Request request(
              Azure::Core::Http::HttpMethod::Patch, url, &bodyStream);
          request.GetUrl().AppendQueryParameter(_detail::QueryAction, "append");
          if (appendDataOptions.Position.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryPosition,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(appendDataOptions.Position.Value())));
          }
          if (appendDataOptions.Timeout.HasValue())
          {
            request.GetUrl().AppendQueryParameter(
                _detail::QueryTimeout,
                _internal::UrlEncodeQueryParameter(
                    std::to_string(appendDataOptions.Timeout.Value())));
          }
          if (appendDataOptions.ContentLength.HasValue())
          {
            request.SetHeader(
                _detail::HeaderContentLength,
                std::to_string(appendDataOptions.ContentLength.Value()));
          }
          if (appendDataOptions.TransactionalContentMd5.HasValue())
          {
            request.SetHeader(
                _detail::HeaderTransactionalContentHashMd5,
                _internal::ToBase64String(appendDataOptions.TransactionalContentMd5.Value()));
          }
          if (appendDataOptions.TransactionalContentCrc64.HasValue())
          {
            request.SetHeader(
                _detail::HeaderTransactionalContentHashCrc64,
                _internal::ToBase64String(appendDataOptions.TransactionalContentCrc64.Value()));
          }
          if (appendDataOptions.LeaseIdOptional.HasValue())
          {
            request.SetHeader(_detail::HeaderLeaseId, appendDataOptions.LeaseIdOptional.Value());
          }
          request.SetHeader(_detail::HeaderVersion, appendDataOptions.ApiVersionParameter);
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
            if (response.GetHeaders().find(_detail::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            }
            if (response.GetHeaders().find(_detail::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderLastModified),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.ContentLength
                  = std::stoll(response.GetHeaders().at(_detail::HeaderContentLength));
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
            if (response.GetHeaders().find(_detail::HeaderAcceptRanges)
                != response.GetHeaders().end())
            {
              result.AcceptRanges = response.GetHeaders().at(_detail::HeaderAcceptRanges);
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
            if (response.GetHeaders().find(_detail::HeaderContentHashMd5)
                != response.GetHeaders().end())
            {
              result.HttpHeaders.ContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderContentHashMd5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            }
            if (response.GetHeaders().find(_detail::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderLastModified),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderResourceType)
                != response.GetHeaders().end())
            {
              result.ResourceType = response.GetHeaders().at(_detail::HeaderResourceType);
            }
            if (response.GetHeaders().find(_detail::HeaderProperties)
                != response.GetHeaders().end())
            {
              result.Properties = response.GetHeaders().at(_detail::HeaderProperties);
            }
            if (response.GetHeaders().find(_detail::HeaderOwner) != response.GetHeaders().end())
            {
              result.Owner = response.GetHeaders().at(_detail::HeaderOwner);
            }
            if (response.GetHeaders().find(_detail::HeaderGroup) != response.GetHeaders().end())
            {
              result.Group = response.GetHeaders().at(_detail::HeaderGroup);
            }
            if (response.GetHeaders().find(_detail::HeaderPermissions)
                != response.GetHeaders().end())
            {
              result.Permissions = response.GetHeaders().at(_detail::HeaderPermissions);
            }
            if (response.GetHeaders().find(_detail::HeaderAcl) != response.GetHeaders().end())
            {
              result.Acl = response.GetHeaders().at(_detail::HeaderAcl);
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseDuration)
                != response.GetHeaders().end())
            {
              result.LeaseDuration
                  = LeaseDuration(response.GetHeaders().at(_detail::HeaderLeaseDuration));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseState)
                != response.GetHeaders().end())
            {
              result.LeaseState = LeaseState(response.GetHeaders().at(_detail::HeaderLeaseState));
            }
            if (response.GetHeaders().find(_detail::HeaderLeaseStatus)
                != response.GetHeaders().end())
            {
              result.LeaseStatus
                  = LeaseStatus(response.GetHeaders().at(_detail::HeaderLeaseStatus));
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
            if (response.GetHeaders().find(_detail::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(_detail::HeaderContinuationToken);
            }
            return Azure::Response<PathDeleteResult>(std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<Models::SetPathAccessControlListResult> SetAccessControlParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // Set directory access control response.
            Models::SetPathAccessControlListResult result;
            if (response.GetHeaders().find(_detail::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            }
            if (response.GetHeaders().find(_detail::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderLastModified),
                  DateTime::DateFormat::Rfc1123);
            }
            return Azure::Response<Models::SetPathAccessControlListResult>(
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
                        Azure::Core::Json::_internal::json::parse(bodyBuffer)));
            if (response.GetHeaders().find(_detail::HeaderContinuationToken)
                != response.GetHeaders().end())
            {
              result.ContinuationToken = response.GetHeaders().at(_detail::HeaderContinuationToken);
            }
            return Azure::Response<PathSetAccessControlRecursiveResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static AclFailedEntry AclFailedEntryFromJson(const Azure::Core::Json::_internal::json& node)
        {
          AclFailedEntry result;
          result.Name = node["name"].get<std::string>();
          result.Type = node["type"].get<std::string>();
          result.ErrorMessage = node["errorMessage"].get<std::string>();
          return result;
        }

        static SetAccessControlRecursiveResponse SetAccessControlRecursiveResponseFromJson(
            const Azure::Core::Json::_internal::json& node)
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
        static Azure::Response<Models::FlushFileResult> FlushDataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok)
          {
            // The data was flushed (written) to the file successfully.
            Models::FlushFileResult result;
            if (response.GetHeaders().find(_detail::HeaderETag) != response.GetHeaders().end())
            {
              result.ETag = Azure::ETag(response.GetHeaders().at(_detail::HeaderETag));
            }
            if (response.GetHeaders().find(_detail::HeaderLastModified)
                != response.GetHeaders().end())
            {
              result.LastModified = DateTime::Parse(
                  response.GetHeaders().at(_detail::HeaderLastModified),
                  DateTime::DateFormat::Rfc1123);
            }
            if (response.GetHeaders().find(_detail::HeaderContentLength)
                != response.GetHeaders().end())
            {
              result.FileSize = std::stoll(response.GetHeaders().at(_detail::HeaderContentLength));
            }
            return Azure::Response<Models::FlushFileResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }

        static Azure::Response<Models::AppendFileResult> AppendDataParseResult(
            Azure::Core::Context context,
            std::unique_ptr<Azure::Core::Http::RawResponse> responsePtr)
        {
          auto& response = *responsePtr;
          if (response.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Accepted)
          {
            // Append data to file control response.
            Models::AppendFileResult result;
            if (response.GetHeaders().find(_detail::HeaderContentHashMd5)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderContentHashMd5), HashAlgorithm::Md5);
            }
            if (response.GetHeaders().find(_detail::HeaderTransactionalContentHashCrc64)
                != response.GetHeaders().end())
            {
              result.TransactionalContentHash = _internal::FromBase64String(
                  response.GetHeaders().at(_detail::HeaderTransactionalContentHashCrc64),
                  HashAlgorithm::Crc64);
            }
            result.IsServerEncrypted
                = response.GetHeaders().at(_detail::HeaderRequestIsServerEncrypted) == "true";
            return Azure::Response<Models::AppendFileResult>(
                std::move(result), std::move(responsePtr));
          }
          else
          {
            (void)context;
            throw Storage::StorageException::CreateFromResponse(std::move(responsePtr));
          }
        }
      };

    }; // class DataLakeRestClient

  } // namespace _detail

}}}} // namespace Azure::Storage::Files::DataLake
