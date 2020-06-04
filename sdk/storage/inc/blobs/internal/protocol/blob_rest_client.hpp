
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace libXML2 {
#include "libxml/tree.h"
}

#include "context.hpp"
#include "http/http.hpp"
#include "http/pipeline.hpp"

namespace Azure { namespace Storage { namespace Blobs {
  enum class AccessTier
  {
    Unknown,
    P1,
    P2,
    P3,
    P4,
    P6,
    P10,
    P15,
    P20,
    P30,
    P40,
    P50,
    P60,
    P70,
    P80,
    Hot,
    Cool,
    Archive,
  }; // enum class AccessTier

  inline std::string AccessTierToString(const AccessTier& access_tier)
  {
    switch (access_tier)
    {
      case AccessTier::Unknown:
        return "";
      case AccessTier::P1:
        return "P1";
      case AccessTier::P2:
        return "P2";
      case AccessTier::P3:
        return "P3";
      case AccessTier::P4:
        return "P4";
      case AccessTier::P6:
        return "P6";
      case AccessTier::P10:
        return "P10";
      case AccessTier::P15:
        return "P15";
      case AccessTier::P20:
        return "P20";
      case AccessTier::P30:
        return "P30";
      case AccessTier::P40:
        return "P40";
      case AccessTier::P50:
        return "P50";
      case AccessTier::P60:
        return "P60";
      case AccessTier::P70:
        return "P70";
      case AccessTier::P80:
        return "P80";
      case AccessTier::Hot:
        return "Hot";
      case AccessTier::Cool:
        return "Cool";
      case AccessTier::Archive:
        return "Archive";
      default:
        return std::string();
    }
  }

  inline AccessTier AccessTierFromString(const std::string& access_tier)
  {
    if (access_tier == "")
    {
      return AccessTier::Unknown;
    }
    if (access_tier == "P1")
    {
      return AccessTier::P1;
    }
    if (access_tier == "P2")
    {
      return AccessTier::P2;
    }
    if (access_tier == "P3")
    {
      return AccessTier::P3;
    }
    if (access_tier == "P4")
    {
      return AccessTier::P4;
    }
    if (access_tier == "P6")
    {
      return AccessTier::P6;
    }
    if (access_tier == "P10")
    {
      return AccessTier::P10;
    }
    if (access_tier == "P15")
    {
      return AccessTier::P15;
    }
    if (access_tier == "P20")
    {
      return AccessTier::P20;
    }
    if (access_tier == "P30")
    {
      return AccessTier::P30;
    }
    if (access_tier == "P40")
    {
      return AccessTier::P40;
    }
    if (access_tier == "P50")
    {
      return AccessTier::P50;
    }
    if (access_tier == "P60")
    {
      return AccessTier::P60;
    }
    if (access_tier == "P70")
    {
      return AccessTier::P70;
    }
    if (access_tier == "P80")
    {
      return AccessTier::P80;
    }
    if (access_tier == "Hot")
    {
      return AccessTier::Hot;
    }
    if (access_tier == "Cool")
    {
      return AccessTier::Cool;
    }
    if (access_tier == "Archive")
    {
      return AccessTier::Archive;
    }
    throw std::runtime_error("cannot convert " + access_tier + " to AccessTier");
  }

  struct BasicResponse
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
  }; // struct BasicResponse

  enum class BlobArchiveStatus
  {
    Unknown,
    RehydratePendingToHot,
    RehydratePendingToCool,
  }; // enum class BlobArchiveStatus

  inline std::string BlobArchiveStatusToString(const BlobArchiveStatus& blob_archive_status)
  {
    switch (blob_archive_status)
    {
      case BlobArchiveStatus::Unknown:
        return "";
      case BlobArchiveStatus::RehydratePendingToHot:
        return "rehydrate-pending-to-hot";
      case BlobArchiveStatus::RehydratePendingToCool:
        return "rehydrate-pending-to-cool";
      default:
        return std::string();
    }
  }

  inline BlobArchiveStatus BlobArchiveStatusFromString(const std::string& blob_archive_status)
  {
    if (blob_archive_status == "")
    {
      return BlobArchiveStatus::Unknown;
    }
    if (blob_archive_status == "rehydrate-pending-to-hot")
    {
      return BlobArchiveStatus::RehydratePendingToHot;
    }
    if (blob_archive_status == "rehydrate-pending-to-cool")
    {
      return BlobArchiveStatus::RehydratePendingToCool;
    }
    throw std::runtime_error("cannot convert " + blob_archive_status + " to BlobArchiveStatus");
  }

  struct BlobBlock
  {
    std::string Name;
    uint64_t Size = 0;
  }; // struct BlobBlock

  struct BlobContainerInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
  }; // struct BlobContainerInfo

  struct BlobContentInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string ContentMD5;
    std::string ContentCRC64;
    uint64_t SequenceNumber = 0;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct BlobContentInfo

  struct BlobHttpHeaders
  {
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMD5;
    std::string CacheControl;
    std::string ContentDisposition;
  }; // struct BlobHttpHeaders

  struct BlobInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    uint64_t SequenceNumber = 0;
  }; // struct BlobInfo

  enum class BlobLeaseState
  {
    Available,
    Leased,
    Expired,
    Breaking,
    Broken,
  }; // enum class BlobLeaseState

  inline std::string BlobLeaseStateToString(const BlobLeaseState& blob_lease_state)
  {
    switch (blob_lease_state)
    {
      case BlobLeaseState::Available:
        return "available";
      case BlobLeaseState::Leased:
        return "leased";
      case BlobLeaseState::Expired:
        return "expired";
      case BlobLeaseState::Breaking:
        return "breaking";
      case BlobLeaseState::Broken:
        return "broken";
      default:
        return std::string();
    }
  }

  inline BlobLeaseState BlobLeaseStateFromString(const std::string& blob_lease_state)
  {
    if (blob_lease_state == "available")
    {
      return BlobLeaseState::Available;
    }
    if (blob_lease_state == "leased")
    {
      return BlobLeaseState::Leased;
    }
    if (blob_lease_state == "expired")
    {
      return BlobLeaseState::Expired;
    }
    if (blob_lease_state == "breaking")
    {
      return BlobLeaseState::Breaking;
    }
    if (blob_lease_state == "broken")
    {
      return BlobLeaseState::Broken;
    }
    throw std::runtime_error("cannot convert " + blob_lease_state + " to BlobLeaseState");
  }

  enum class BlobLeaseStatus
  {
    Locked,
    Unlocked,
  }; // enum class BlobLeaseStatus

  inline std::string BlobLeaseStatusToString(const BlobLeaseStatus& blob_lease_status)
  {
    switch (blob_lease_status)
    {
      case BlobLeaseStatus::Locked:
        return "locked";
      case BlobLeaseStatus::Unlocked:
        return "unlocked";
      default:
        return std::string();
    }
  }

  inline BlobLeaseStatus BlobLeaseStatusFromString(const std::string& blob_lease_status)
  {
    if (blob_lease_status == "locked")
    {
      return BlobLeaseStatus::Locked;
    }
    if (blob_lease_status == "unlocked")
    {
      return BlobLeaseStatus::Unlocked;
    }
    throw std::runtime_error("cannot convert " + blob_lease_status + " to BlobLeaseStatus");
  }

  enum class BlobType
  {
    Unknown,
    BlockBlob,
    PageBlob,
    AppendBlob,
  }; // enum class BlobType

  inline std::string BlobTypeToString(const BlobType& blob_type)
  {
    switch (blob_type)
    {
      case BlobType::Unknown:
        return "";
      case BlobType::BlockBlob:
        return "BlockBlob";
      case BlobType::PageBlob:
        return "PageBlob";
      case BlobType::AppendBlob:
        return "AppendBlob";
      default:
        return std::string();
    }
  }

  inline BlobType BlobTypeFromString(const std::string& blob_type)
  {
    if (blob_type == "")
    {
      return BlobType::Unknown;
    }
    if (blob_type == "BlockBlob")
    {
      return BlobType::BlockBlob;
    }
    if (blob_type == "PageBlob")
    {
      return BlobType::PageBlob;
    }
    if (blob_type == "AppendBlob")
    {
      return BlobType::AppendBlob;
    }
    throw std::runtime_error("cannot convert " + blob_type + " to BlobType");
  }

  struct BlockInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ContentMD5;
    std::string ContentCRC64;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct BlockInfo

  enum class BlockListTypeOption
  {
    Committed,
    Uncommitted,
    All,
  }; // enum class BlockListTypeOption

  inline std::string BlockListTypeOptionToString(const BlockListTypeOption& block_list_type_option)
  {
    switch (block_list_type_option)
    {
      case BlockListTypeOption::Committed:
        return "committed";
      case BlockListTypeOption::Uncommitted:
        return "uncommitted";
      case BlockListTypeOption::All:
        return "all";
      default:
        return std::string();
    }
  }

  inline BlockListTypeOption BlockListTypeOptionFromString(
      const std::string& block_list_type_option)
  {
    if (block_list_type_option == "committed")
    {
      return BlockListTypeOption::Committed;
    }
    if (block_list_type_option == "uncommitted")
    {
      return BlockListTypeOption::Uncommitted;
    }
    if (block_list_type_option == "all")
    {
      return BlockListTypeOption::All;
    }
    throw std::runtime_error(
        "cannot convert " + block_list_type_option + " to BlockListTypeOption");
  }

  enum class BlockType
  {
    Committed,
    Uncommitted,
    Latest,
  }; // enum class BlockType

  inline std::string BlockTypeToString(const BlockType& block_type)
  {
    switch (block_type)
    {
      case BlockType::Committed:
        return "Committed";
      case BlockType::Uncommitted:
        return "Uncommitted";
      case BlockType::Latest:
        return "Latest";
      default:
        return std::string();
    }
  }

  inline BlockType BlockTypeFromString(const std::string& block_type)
  {
    if (block_type == "Committed")
    {
      return BlockType::Committed;
    }
    if (block_type == "Uncommitted")
    {
      return BlockType::Uncommitted;
    }
    if (block_type == "Latest")
    {
      return BlockType::Latest;
    }
    throw std::runtime_error("cannot convert " + block_type + " to BlockType");
  }

  enum class DeleteSnapshotsOption
  {
    None,
    IncludeSnapshots,
    Only,
  }; // enum class DeleteSnapshotsOption

  inline std::string DeleteSnapshotsOptionToString(
      const DeleteSnapshotsOption& delete_snapshots_option)
  {
    switch (delete_snapshots_option)
    {
      case DeleteSnapshotsOption::None:
        return "";
      case DeleteSnapshotsOption::IncludeSnapshots:
        return "include";
      case DeleteSnapshotsOption::Only:
        return "only";
      default:
        return std::string();
    }
  }

  inline DeleteSnapshotsOption DeleteSnapshotsOptionFromString(
      const std::string& delete_snapshots_option)
  {
    if (delete_snapshots_option == "")
    {
      return DeleteSnapshotsOption::None;
    }
    if (delete_snapshots_option == "include")
    {
      return DeleteSnapshotsOption::IncludeSnapshots;
    }
    if (delete_snapshots_option == "only")
    {
      return DeleteSnapshotsOption::Only;
    }
    throw std::runtime_error(
        "cannot convert " + delete_snapshots_option + " to DeleteSnapshotsOption");
  }

  enum class ListBlobContainersIncludeOption
  {
    None,
    Metadata,
  }; // enum class ListBlobContainersIncludeOption

  inline std::string ListBlobContainersIncludeOptionToString(
      const ListBlobContainersIncludeOption& list_blob_containers_include_option)
  {
    switch (list_blob_containers_include_option)
    {
      case ListBlobContainersIncludeOption::None:
        return "";
      case ListBlobContainersIncludeOption::Metadata:
        return "metadata";
      default:
        return std::string();
    }
  }

  inline ListBlobContainersIncludeOption ListBlobContainersIncludeOptionFromString(
      const std::string& list_blob_containers_include_option)
  {
    if (list_blob_containers_include_option == "")
    {
      return ListBlobContainersIncludeOption::None;
    }
    if (list_blob_containers_include_option == "metadata")
    {
      return ListBlobContainersIncludeOption::Metadata;
    }
    throw std::runtime_error(
        "cannot convert " + list_blob_containers_include_option
        + " to ListBlobContainersIncludeOption");
  }

  enum class ListBlobsIncludeItem
  {
    Copy,
    Deleted,
    Metadata,
    Snapshots,
    UncomittedBlobs,
  }; // enum class ListBlobsIncludeItem

  inline std::string ListBlobsIncludeItemToString(
      const ListBlobsIncludeItem& list_blobs_include_item)
  {
    switch (list_blobs_include_item)
    {
      case ListBlobsIncludeItem::Copy:
        return "copy";
      case ListBlobsIncludeItem::Deleted:
        return "deleted";
      case ListBlobsIncludeItem::Metadata:
        return "metadata";
      case ListBlobsIncludeItem::Snapshots:
        return "snapshots";
      case ListBlobsIncludeItem::UncomittedBlobs:
        return "uncommittedblobs";
      default:
        return std::string();
    }
  }

  inline ListBlobsIncludeItem ListBlobsIncludeItemFromString(
      const std::string& list_blobs_include_item)
  {
    if (list_blobs_include_item == "copy")
    {
      return ListBlobsIncludeItem::Copy;
    }
    if (list_blobs_include_item == "deleted")
    {
      return ListBlobsIncludeItem::Deleted;
    }
    if (list_blobs_include_item == "metadata")
    {
      return ListBlobsIncludeItem::Metadata;
    }
    if (list_blobs_include_item == "snapshots")
    {
      return ListBlobsIncludeItem::Snapshots;
    }
    if (list_blobs_include_item == "uncommittedblobs")
    {
      return ListBlobsIncludeItem::UncomittedBlobs;
    }
    throw std::runtime_error(
        "cannot convert " + list_blobs_include_item + " to ListBlobsIncludeItem");
  }

  enum class PublicAccessType
  {
    Container,
    Blob,
    Private,
  }; // enum class PublicAccessType

  inline std::string PublicAccessTypeToString(const PublicAccessType& public_access_type)
  {
    switch (public_access_type)
    {
      case PublicAccessType::Container:
        return "container";
      case PublicAccessType::Blob:
        return "blob";
      case PublicAccessType::Private:
        return "";
      default:
        return std::string();
    }
  }

  inline PublicAccessType PublicAccessTypeFromString(const std::string& public_access_type)
  {
    if (public_access_type == "container")
    {
      return PublicAccessType::Container;
    }
    if (public_access_type == "blob")
    {
      return PublicAccessType::Blob;
    }
    if (public_access_type == "")
    {
      return PublicAccessType::Private;
    }
    throw std::runtime_error("cannot convert " + public_access_type + " to PublicAccessType");
  }

  struct BlobBlockListInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string ContentType;
    uint64_t ContentLength = 0;
    std::vector<BlobBlock> CommittedBlocks;
    std::vector<BlobBlock> UncommittedBlocks;
  }; // struct BlobBlockListInfo

  struct BlobContainerItem
  {
    std::string Name;
    std::string ETag;
    std::string LastModified;
    std::map<std::string, std::string> Metadata;
    PublicAccessType AccessType = PublicAccessType::Private;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
  }; // struct BlobContainerItem

  struct BlobContainerProperties
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::map<std::string, std::string> Metadata;
    PublicAccessType AccessType = PublicAccessType::Private;
    bool HasImmutabilityPolicy = false;
    bool HasLegalHold = false;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
  }; // struct BlobContainerProperties

  struct BlobItem
  {
    std::string Name;
    bool Deleted = false;
    std::string Snapshot;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    std::string CreationTime;
    std::string LastModified;
    std::string ETag;
    uint64_t ContentLength = 0;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    AccessTier Tier = AccessTier::Unknown;
    bool AccessTierInferred = true;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    std::string LeaseDuration;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct BlobItem

  struct BlobProperties
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    std::string CreationTime;
    std::map<std::string, std::string> Metadata;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    uint64_t ContentLength = 0;
    std::string ContentType;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentMD5;
    std::string CacheControl;
    std::string ContentDisposition;
    uint64_t SequenceNumber = 0; // only for page blob
    int CommittedBlockCount = 0; // only for append blob
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
    AccessTier Tier = AccessTier::Unknown;
    bool AccessTierInferred = true;
    BlobArchiveStatus ArchiveStatus = BlobArchiveStatus::Unknown;
    std::string AccessTierChangeTime;
  }; // struct BlobProperties

  struct FlattenedDownloadProperties
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::vector<uint8_t> BodyBuffer;
    Azure::Core::Http::BodyStream* BodyStream = nullptr;
    std::string ETag;
    std::string LastModified;
    std::string ContentRange;
    BlobHttpHeaders Properties;
    std::map<std::string, std::string> Metadata;
    uint64_t SequenceNumber = 0; // only for page blob
    uint64_t CommittedBlockCount = 0; // only for append blob
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    std::string ContentMD5; // MD5 for the downloaded range
    std::string ContentCRC64;
    std::string LeaseDuration;
    BlobLeaseState LeaseState = BlobLeaseState::Available;
    BlobLeaseStatus LeaseStatus = BlobLeaseStatus::Unlocked;
    bool ServerEncrypted = true;
    std::string EncryptionKeySHA256;
  }; // struct FlattenedDownloadProperties

  struct BlobsFlatSegment
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ServiceEndpoint;
    std::string Container;
    std::string Prefix;
    std::string Marker;
    std::string NextMarker;
    int MaxResults = 0;
    std::string Delimiter;
    std::vector<BlobItem> BlobItems;
  }; // struct BlobsFlatSegment

  struct ListContainersSegment
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ServiceEndpoint;
    std::string Prefix;
    std::string Marker;
    std::string NextMarker;
    int MaxResults = 0;
    std::vector<BlobContainerItem> BlobContainerItems;
  }; // struct ListContainersSegment

  class BlobRestClient {
  public:
    class Service {
    public:
      struct ListBlobContainersOptions
      {
        std::string Version;
        std::string Date;
        std::string Prefix;
        std::string Marker;
        int MaxResults = 0;
        ListBlobContainersIncludeOption IncludeMetadata = ListBlobContainersIncludeOption::None;
      }; // struct ListBlobContainersOptions

      static Azure::Core::Http::Request ListBlobContainersConstructRequest(
          const std::string& url,
          const ListBlobContainersOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        request.AddQueryParameter("comp", "list");
        if (!options.Prefix.empty())
        {
          request.AddQueryParameter("prefix", options.Prefix);
        }
        if (!options.Marker.empty())
        {
          request.AddQueryParameter("marker", options.Marker);
        }
        if (options.MaxResults != 0)
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults));
        }
        std::string list_blob_containers_include_option
            = ListBlobContainersIncludeOptionToString(options.IncludeMetadata);
        if (!list_blob_containers_include_option.empty())
        {
          request.AddQueryParameter("include", list_blob_containers_include_option);
        }
        return request;
      }

      static ListContainersSegment ListBlobContainersParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        ListContainersSegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        // TODO: Think about how to initialize
        // xmlInitParser();
        // TODO: Think about how to free doc on exception

        // TODO: Think about how to hanlde xml > 2GB
        using namespace libXML2;
        xmlDoc* doc = xmlReadMemory(
            reinterpret_cast<const char*>(http_response.GetBodyBuffer().data()),
            int(http_response.GetBodyBuffer().size()),
            nullptr,
            nullptr,
            0);
        if (doc == nullptr)
          throw std::runtime_error("failed to parse response xml");

        xmlNode* root = xmlDocGetRootElement(doc);
        if (root == nullptr
            || std::string(reinterpret_cast<const char*>(root->name)) != "EnumerationResults")
          throw std::runtime_error("failed to parse response xml");

        enum
        {
          start_tag,
          attribute,
          content,
          end_tag,
        };

        auto parse_xml_callback
            = [&response, blob_container_item = BlobContainerItem(), in_metadata = false](
                  const std::string& name, int type, const std::string& value) mutable {
                if (type == start_tag && name == "Metadata")
                  in_metadata = true;
                else if (type == end_tag && name == "Metadata")
                  in_metadata = false;
                else if (type == content && in_metadata)
                  blob_container_item.Metadata.emplace(name, value);
                else if (type == attribute && name == "ServiceEndpoint")
                  response.ServiceEndpoint = value;
                else if (type == content && name == "Prefix")
                  response.Prefix = value;
                else if (type == content && name == "Marker")
                  response.Marker = value;
                else if (type == content && name == "MaxResults")
                  response.MaxResults = std::stoi(value);
                else if (type == content && name == "NextMarker")
                  response.NextMarker = value;
                else if (type == start_tag && name == "Container")
                  blob_container_item = BlobContainerItem();
                else if (type == end_tag && name == "Container")
                  response.BlobContainerItems.emplace_back(std::move(blob_container_item));
                else if (type == content && name == "Name")
                  blob_container_item.Name = value;
                else if (type == content && name == "Last-Modified")
                  blob_container_item.LastModified = value;
                else if (type == content && name == "Etag")
                  blob_container_item.ETag = value;
                else if (type == content && name == "LeaseStatus")
                  blob_container_item.LeaseStatus = BlobLeaseStatusFromString(value);
                else if (type == content && name == "LeaseState")
                  blob_container_item.LeaseState = BlobLeaseStateFromString(value);
                else if (type == content && name == "LeaseDuration")
                  blob_container_item.LeaseDuration = value;
                else if (type == content && name == "PublicAccess")
                  blob_container_item.AccessType = PublicAccessTypeFromString(value);
                else if (type == content && name == "HasImmutabilityPolicy")
                  blob_container_item.HasImmutabilityPolicy = value == "true";
                else if (type == content && name == "HasLegalHold")
                  blob_container_item.HasLegalHold = value == "true";
              };

        std::function<void(xmlNode*)> parse_xml;
        parse_xml = [&parse_xml, &parse_xml_callback](xmlNode* node) {
          if (!(node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE))
            return;

          std::string node_name(reinterpret_cast<const char*>(node->name));
          parse_xml_callback(node_name, start_tag, "");

          for (xmlAttr* prop = node->properties; prop; prop = prop->next)
          {
            std::string prop_name(reinterpret_cast<const char*>(prop->name));
            std::string prop_value(reinterpret_cast<const char*>(prop->children->content));
            parse_xml_callback(prop_name, attribute, prop_value);
          }

          bool has_child_element = false;
          for (xmlNode* child = node->children; child; child = child->next)
          {
            has_child_element |= child->type == XML_ELEMENT_NODE;
            parse_xml(child);
          }

          if (!has_child_element && node->children)
          {
            std::string node_content(reinterpret_cast<const char*>(node->children->content));
            parse_xml_callback(node_name, content, node_content);
          }

          parse_xml_callback(node_name, end_tag, "");
        };

        parse_xml(root);

        xmlFreeDoc(doc);

        // TODO: Think about how to cleanup
        // xmlCleanupParser();
        return response;
      }

      static ListContainersSegment ListBlobContainers(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobContainersOptions& options)
      {
        auto request = ListBlobContainersConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return ListBlobContainersParseResponse(*response);
      }

    }; // class Service

    class Container {
    public:
      struct CreateOptions
      {
        std::string Version;
        std::string Date;
        PublicAccessType AccessType = PublicAccessType::Private;
        std::map<std::string, std::string> Metadata;
      }; // struct CreateOptions

      static Azure::Core::Http::Request CreateConstructRequest(
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        for (const auto& pair : options.Metadata)
        {
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        auto options_accesstype_str = PublicAccessTypeToString(options.AccessType);
        if (!options_accesstype_str.empty())
        {
          request.AddHeader("x-ms-blob-public-access", options_accesstype_str);
        }
        return request;
      }

      static BlobContainerInfo CreateParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        return response;
      }

      static BlobContainerInfo Create(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CreateOptions& options)
      {
        auto request = CreateConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CreateParseResponse(*response);
      }

      struct DeleteOptions
      {
        std::string Version;
        std::string Date;
      }; // struct DeleteOptions

      static Azure::Core::Http::Request DeleteConstructRequest(
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        return request;
      }

      static BasicResponse DeleteParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse Delete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = DeleteConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return DeleteParseResponse(*response);
      }

      struct GetPropertiesOptions
      {
        std::string Version;
        std::string Date;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct GetPropertiesOptions

      static Azure::Core::Http::Request GetPropertiesConstructRequest(
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static BlobContainerProperties GetPropertiesParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        BlobContainerProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        for (auto i = http_response.GetHeaders().lower_bound("x-ms-meta-");
             i != http_response.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        auto response_accesstype_iterator
            = http_response.GetHeaders().find("x-ms-blob-public-access");
        if (response_accesstype_iterator != http_response.GetHeaders().end())
        {
          response.AccessType = PublicAccessTypeFromString(response_accesstype_iterator->second);
        }
        response.HasImmutabilityPolicy
            = http_response.GetHeaders().at("x-ms-has-immutability-policy") == "true";
        response.HasLegalHold = http_response.GetHeaders().at("x-ms-has-legal-hold") == "true";
        response.LeaseStatus
            = BlobLeaseStatusFromString(http_response.GetHeaders().at("x-ms-lease-status"));
        response.LeaseState
            = BlobLeaseStateFromString(http_response.GetHeaders().at("x-ms-lease-state"));
        auto response_leaseduration_iterator
            = http_response.GetHeaders().find("x-ms-lease-duration");
        if (response_leaseduration_iterator != http_response.GetHeaders().end())
        {
          response.LeaseDuration = response_leaseduration_iterator->second;
        }
        return response;
      }

      static BlobContainerProperties GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = GetPropertiesConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetPropertiesParseResponse(*response);
      }

      struct SetMetadataOptions
      {
        std::string Version;
        std::string Date;
        std::map<std::string, std::string> Metadata;
      }; // struct SetMetadataOptions

      static Azure::Core::Http::Request SetMetadataConstructRequest(
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        for (const auto& pair : options.Metadata)
        {
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        return request;
      }

      static BlobContainerInfo SetMetadataParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContainerInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        return response;
      }

      static BlobContainerInfo SetMetadata(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = SetMetadataConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetMetadataParseResponse(*response);
      }

      struct ListBlobsOptions
      {
        std::string Version;
        std::string Date;
        std::string Prefix;
        std::string Delimiter;
        std::string Marker;
        int MaxResults = 0;
        std::vector<ListBlobsIncludeItem> Include;
      }; // struct ListBlobsOptions

      static Azure::Core::Http::Request ListBlobsConstructRequest(
          const std::string& url,
          const ListBlobsOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        request.AddQueryParameter("restype", "container");
        request.AddQueryParameter("comp", "list");
        if (!options.Prefix.empty())
        {
          request.AddQueryParameter("prefix", options.Prefix);
        }
        if (!options.Delimiter.empty())
        {
          request.AddQueryParameter("delimiter", options.Delimiter);
        }
        if (!options.Marker.empty())
        {
          request.AddQueryParameter("marker", options.Marker);
        }
        if (options.MaxResults != 0)
        {
          request.AddQueryParameter("maxresults", std::to_string(options.MaxResults));
        }
        std::string options_include_str;
        for (auto i : options.Include)
        {
          if (!options_include_str.empty())
          {
            options_include_str += ",";
          }
          options_include_str += ListBlobsIncludeItemToString(i);
        }
        if (!options_include_str.empty())
        {
          request.AddQueryParameter("include", options_include_str);
        }
        return request;
      }

      static BlobsFlatSegment ListBlobsParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobsFlatSegment response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        // TODO: Think about how to initialize
        // xmlInitParser();
        // TODO: Think about how to free doc on exception

        // TODO: Think about how to hanlde xml > 2GB
        using namespace libXML2;
        xmlDoc* doc = xmlReadMemory(
            reinterpret_cast<const char*>(http_response.GetBodyBuffer().data()),
            int(http_response.GetBodyBuffer().size()),
            nullptr,
            nullptr,
            0);
        if (doc == nullptr)
          throw std::runtime_error("failed to parse response xml");

        xmlNode* root = xmlDocGetRootElement(doc);
        if (root == nullptr
            || std::string(reinterpret_cast<const char*>(root->name)) != "EnumerationResults")
          throw std::runtime_error("failed to parse response xml");

        enum
        {
          start_tag,
          attribute,
          content,
          end_tag,
        };

        auto parse_xml_callback
            = [&response, blob_item = BlobItem(), in_metadata = false](
                  const std::string& name, int type, const std::string& value) mutable {
                if (type == start_tag && name == "Metadata")
                  in_metadata = true;
                else if (type == end_tag && name == "Metadata")
                  in_metadata = false;
                else if (type == content && in_metadata)
                  blob_item.Metadata.emplace(name, value);
                else if (type == attribute && name == "ServiceEndpoint")
                  response.ServiceEndpoint = value;
                else if (type == attribute && name == "ContainerName")
                  response.Container = value;
                else if (type == content && name == "Prefix")
                  response.Prefix = value;
                else if (type == content && name == "Marker")
                  response.Marker = value;
                else if (type == content && name == "MaxResults")
                  response.MaxResults = std::stoi(value);
                else if (type == content && name == "Delimiter")
                  response.Delimiter = value;
                else if (type == content && name == "NextMarker")
                  response.NextMarker = value;
                else if (type == start_tag && name == "Blob")
                  blob_item = BlobItem();
                else if (type == end_tag && name == "Blob")
                  response.BlobItems.emplace_back(std::move(blob_item));
                else if (type == content && name == "Name")
                  blob_item.Name = value;
                else if (type == content && name == "Deleted")
                  blob_item.Deleted = value == "true";
                else if (type == content && name == "Snapshot")
                  blob_item.Snapshot = value;
                else if (type == content && name == "Creation-Time")
                  blob_item.CreationTime = value;
                else if (type == content && name == "Last-Modified")
                  blob_item.LastModified = value;
                else if (type == content && name == "Etag")
                  blob_item.ETag = value;
                else if (type == content && name == "Content-Length")
                  blob_item.ContentLength = std::stoull(value);
                else if (type == content && name == "BlobType")
                  blob_item.BlobType = BlobTypeFromString(value);
                else if (type == content && name == "AccessTier")
                  blob_item.Tier = AccessTierFromString(value);
                else if (type == content && name == "AccessTierInferred")
                  blob_item.AccessTierInferred = value == "true";
                else if (type == content && name == "LeaseStatus")
                  blob_item.LeaseStatus = BlobLeaseStatusFromString(value);
                else if (type == content && name == "LeaseState")
                  blob_item.LeaseState = BlobLeaseStateFromString(value);
                else if (type == content && name == "LeaseDuration")
                  blob_item.LeaseDuration = value;
                else if (type == content && name == "ServerEncrypted")
                  blob_item.ServerEncrypted = value == "true";
                else if (type == content && name == "CustomerProvidedKeySha256")
                  blob_item.EncryptionKeySHA256 = value;
                else if (type == content && name == "Content-Type")
                  blob_item.Properties.ContentType = value;
                else if (type == content && name == "Content-Encoding")
                  blob_item.Properties.ContentEncoding = value;
                else if (type == content && name == "Content-Language")
                  blob_item.Properties.ContentLanguage = value;
                else if (type == content && name == "Content-MD5")
                  blob_item.Properties.ContentMD5 = value;
                else if (type == content && name == "Cache-Control")
                  blob_item.Properties.CacheControl = value;
                else if (type == content && name == "Content-Disposition")
                  blob_item.Properties.ContentDisposition = value;
              };

        std::function<void(xmlNode*)> parse_xml;
        parse_xml = [&parse_xml, &parse_xml_callback](xmlNode* node) {
          if (!(node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE))
            return;

          std::string node_name(reinterpret_cast<const char*>(node->name));
          parse_xml_callback(node_name, start_tag, "");

          for (xmlAttr* prop = node->properties; prop; prop = prop->next)
          {
            std::string prop_name(reinterpret_cast<const char*>(prop->name));
            std::string prop_value(reinterpret_cast<const char*>(prop->children->content));
            parse_xml_callback(prop_name, attribute, prop_value);
          }

          bool has_child_element = false;
          for (xmlNode* child = node->children; child; child = child->next)
          {
            has_child_element |= child->type == XML_ELEMENT_NODE;
            parse_xml(child);
          }

          if (!has_child_element && node->children)
          {
            std::string node_content(reinterpret_cast<const char*>(node->children->content));
            parse_xml_callback(node_name, content, node_content);
          }

          parse_xml_callback(node_name, end_tag, "");
        };

        parse_xml(root);

        xmlFreeDoc(doc);

        // TODO: Think about how to cleanup
        // xmlCleanupParser();
        return response;
      }

      static BlobsFlatSegment ListBlobs(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const ListBlobsOptions& options)
      {
        auto request = ListBlobsConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return ListBlobsParseResponse(*response);
      }

    }; // class Container

    class Blob {
    public:
      struct DownloadOptions
      {
        std::string Version;
        std::string Date;
        std::pair<uint64_t, uint64_t> Range;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct DownloadOptions

      static Azure::Core::Http::Request DownloadConstructRequest(
          const std::string& url,
          const DownloadOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        if (options.Range.first <= options.Range.second)
        {
          request.AddHeader(
              "x-ms-range",
              "bytes=" + std::to_string(options.Range.first) + "-"
                  + std::to_string(options.Range.second));
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static FlattenedDownloadProperties DownloadParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        FlattenedDownloadProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200 || http_status_code == 206))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_properties_contenttype_iterator
            = http_response.GetHeaders().find("Content-Type");
        if (response_properties_contenttype_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentType = response_properties_contenttype_iterator->second;
        }
        auto response_properties_contentencoding_iterator
            = http_response.GetHeaders().find("Content-Encoding");
        if (response_properties_contentencoding_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentEncoding
              = response_properties_contentencoding_iterator->second;
        }
        auto response_properties_contentlanguage_iterator
            = http_response.GetHeaders().find("Content-Language");
        if (response_properties_contentlanguage_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentLanguage
              = response_properties_contentlanguage_iterator->second;
        }
        auto response_properties_cachecontrol_iterator
            = http_response.GetHeaders().find("Cache-Control");
        if (response_properties_cachecontrol_iterator != http_response.GetHeaders().end())
        {
          response.Properties.CacheControl = response_properties_cachecontrol_iterator->second;
        }
        auto response_properties_contentmd5_iterator
            = http_response.GetHeaders().find("Content-MD5");
        if (response_properties_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentMD5 = response_properties_contentmd5_iterator->second;
        }
        auto response_properties_contentdisposition_iterator
            = http_response.GetHeaders().find("Content-Disposition");
        if (response_properties_contentdisposition_iterator != http_response.GetHeaders().end())
        {
          response.Properties.ContentDisposition
              = response_properties_contentdisposition_iterator->second;
        }
        for (auto i = http_response.GetHeaders().lower_bound("x-ms-meta-");
             i != http_response.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        response.LeaseStatus
            = BlobLeaseStatusFromString(http_response.GetHeaders().at("x-ms-lease-status"));
        response.LeaseState
            = BlobLeaseStateFromString(http_response.GetHeaders().at("x-ms-lease-state"));
        auto response_leaseduration_iterator
            = http_response.GetHeaders().find("x-ms-lease-duration");
        if (response_leaseduration_iterator != http_response.GetHeaders().end())
        {
          response.LeaseDuration = response_leaseduration_iterator->second;
        }
        auto response_contentrange_iterator = http_response.GetHeaders().find("Content-Range");
        if (response_contentrange_iterator != http_response.GetHeaders().end())
        {
          response.ContentRange = response_contentrange_iterator->second;
        }
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_committedblockcount_iterator
            = http_response.GetHeaders().find("x-ms-blob-committed-block-count");
        if (response_committedblockcount_iterator != http_response.GetHeaders().end())
        {
          response.CommittedBlockCount = std::stoull(response_committedblockcount_iterator->second);
        }
        response.BlobType = BlobTypeFromString(http_response.GetHeaders().at("x-ms-blob-type"));
        response.BodyBuffer = http_response.GetBodyBuffer();
        return response;
      }

      static FlattenedDownloadProperties Download(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DownloadOptions& options)
      {
        auto request = DownloadConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return DownloadParseResponse(*response);
      }

      struct DeleteOptions
      {
        std::string Version;
        std::string Date;
        DeleteSnapshotsOption DeleteSnapshots = DeleteSnapshotsOption::None;
      }; // struct DeleteOptions

      static Azure::Core::Http::Request DeleteConstructRequest(
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        return request;
      }

      static BasicResponse DeleteParseResponse(Azure::Core::Http::Response& http_response)
      {
        BasicResponse response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 202))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        return response;
      }

      static BasicResponse Delete(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const DeleteOptions& options)
      {
        auto request = DeleteConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return DeleteParseResponse(*response);
      }

      struct GetPropertiesOptions
      {
        std::string Version;
        std::string Date;
      }; // struct GetPropertiesOptions

      static Azure::Core::Http::Request GetPropertiesConstructRequest(
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, url);
        request.AddHeader("Content-Length", "0");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        return request;
      }

      static BlobProperties GetPropertiesParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobProperties response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.CreationTime = http_response.GetHeaders().at("x-ms-creation-time");
        for (auto i = http_response.GetHeaders().lower_bound("x-ms-meta-");
             i != http_response.GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
             ++i)
        {
          response.Metadata.emplace(i->first.substr(10), i->second);
        }
        response.BlobType = BlobTypeFromString(http_response.GetHeaders().at("x-ms-blob-type"));
        response.LeaseStatus
            = BlobLeaseStatusFromString(http_response.GetHeaders().at("x-ms-lease-status"));
        response.LeaseState
            = BlobLeaseStateFromString(http_response.GetHeaders().at("x-ms-lease-state"));
        auto response_leaseduration_iterator
            = http_response.GetHeaders().find("x-ms-lease-duration");
        if (response_leaseduration_iterator != http_response.GetHeaders().end())
        {
          response.LeaseDuration = response_leaseduration_iterator->second;
        }
        response.ContentLength = std::stoull(http_response.GetHeaders().at("Content-Length"));
        auto response_contenttype_iterator = http_response.GetHeaders().find("Content-Type");
        if (response_contenttype_iterator != http_response.GetHeaders().end())
        {
          response.ContentType = response_contenttype_iterator->second;
        }
        auto response_contentencoding_iterator
            = http_response.GetHeaders().find("Content-Encoding");
        if (response_contentencoding_iterator != http_response.GetHeaders().end())
        {
          response.ContentEncoding = response_contentencoding_iterator->second;
        }
        auto response_contentlanguage_iterator
            = http_response.GetHeaders().find("Content-Language");
        if (response_contentlanguage_iterator != http_response.GetHeaders().end())
        {
          response.ContentLanguage = response_contentlanguage_iterator->second;
        }
        auto response_cachecontrol_iterator = http_response.GetHeaders().find("Cache-Control");
        if (response_cachecontrol_iterator != http_response.GetHeaders().end())
        {
          response.CacheControl = response_cachecontrol_iterator->second;
        }
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentdisposition_iterator
            = http_response.GetHeaders().find("Content-Disposition");
        if (response_contentdisposition_iterator != http_response.GetHeaders().end())
        {
          response.ContentDisposition = response_contentdisposition_iterator->second;
        }
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        auto response_committedblockcount_iterator
            = http_response.GetHeaders().find("x-ms-blob-committed-block-count");
        if (response_committedblockcount_iterator != http_response.GetHeaders().end())
        {
          response.CommittedBlockCount = std::stoi(response_committedblockcount_iterator->second);
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        auto response_tier_iterator = http_response.GetHeaders().find("x-ms-access-tier");
        if (response_tier_iterator != http_response.GetHeaders().end())
        {
          response.Tier = AccessTierFromString(response_tier_iterator->second);
        }
        auto response_accesstierinferred_iterator
            = http_response.GetHeaders().find("x-ms-access-tier-inferred");
        if (response_accesstierinferred_iterator != http_response.GetHeaders().end())
        {
          response.AccessTierInferred = response_accesstierinferred_iterator->second == "true";
        }
        auto response_archivestatus_iterator
            = http_response.GetHeaders().find("x-ms-archive-status");
        if (response_archivestatus_iterator != http_response.GetHeaders().end())
        {
          response.ArchiveStatus
              = BlobArchiveStatusFromString(response_archivestatus_iterator->second);
        }
        auto response_accesstierchangetime_iterator
            = http_response.GetHeaders().find("x-ms-access-tier-change-time");
        if (response_accesstierchangetime_iterator != http_response.GetHeaders().end())
        {
          response.AccessTierChangeTime = response_accesstierchangetime_iterator->second;
        }
        return response;
      }

      static BlobProperties GetProperties(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetPropertiesOptions& options)
      {
        auto request = GetPropertiesConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetPropertiesParseResponse(*response);
      }

      struct SetHttpHeadersOptions
      {
        std::string Version;
        std::string Date;
        std::string ContentType;
        std::string ContentEncoding;
        std::string ContentLanguage;
        std::string ContentMD5;
        std::string CacheControl;
        std::string ContentDisposition;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct SetHttpHeadersOptions

      static Azure::Core::Http::Request SetHttpHeadersConstructRequest(
          const std::string& url,
          const SetHttpHeadersOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "properties");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        if (!options.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.ContentType);
        }
        if (!options.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.ContentEncoding);
        }
        if (!options.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.ContentLanguage);
        }
        if (!options.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.CacheControl);
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.ContentMD5);
        }
        if (!options.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.ContentDisposition);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static BlobInfo SetHttpHeadersParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_sequencenumber_iterator
            = http_response.GetHeaders().find("x-ms-blob-sequence-number");
        if (response_sequencenumber_iterator != http_response.GetHeaders().end())
        {
          response.SequenceNumber = std::stoull(response_sequencenumber_iterator->second);
        }
        return response;
      }

      static BlobInfo SetHttpHeaders(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetHttpHeadersOptions& options)
      {
        auto request = SetHttpHeadersConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetHttpHeadersParseResponse(*response);
      }

      struct SetMetadataOptions
      {
        std::string Version;
        std::string Date;
        std::map<std::string, std::string> Metadata;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct SetMetadataOptions

      static Azure::Core::Http::Request SetMetadataConstructRequest(
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "metadata");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        for (const auto& pair : options.Metadata)
        {
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static BlobInfo SetMetadataParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        return response;
      }

      static BlobInfo SetMetadata(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const SetMetadataOptions& options)
      {
        auto request = SetMetadataConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return SetMetadataParseResponse(*response);
      }

    }; // class Blob

    class BlockBlob {
    public:
      struct UploadOptions
      {
        std::string Version;
        std::string Date;
        std::vector<uint8_t>* BodyBuffer = nullptr;
        Azure::Core::Http::BodyStream* BodyStream = nullptr;
        std::string ContentMD5;
        std::string ContentCRC64;
        Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
        BlobHttpHeaders Properties;
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        AccessTier Tier = AccessTier::Unknown;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct UploadOptions

      static Azure::Core::Http::Request UploadConstructRequest(
          const std::string& url,
          const UploadOptions& options)
      {
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put, url, *options.BodyBuffer);
        request.AddHeader("Content-Length", std::to_string(options.BodyBuffer->size()));
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("Content-MD5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCRC64);
        }
        if (!options.Properties.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.Properties.ContentType);
        }
        if (!options.Properties.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.Properties.ContentEncoding);
        }
        if (!options.Properties.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.Properties.ContentLanguage);
        }
        if (!options.Properties.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.Properties.CacheControl);
        }
        if (!options.Properties.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.Properties.ContentMD5);
        }
        if (!options.Properties.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.Properties.ContentDisposition);
        }
        for (const auto& pair : options.Metadata)
        {
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        request.AddHeader("x-ms-blob-type", BlobTypeToString(options.BlobType));
        auto options_tier_str = AccessTierToString(options.Tier);
        if (!options_tier_str.empty())
        {
          request.AddHeader("x-ms-access-tier", options_tier_str);
        }
        return request;
      }

      static BlobContentInfo UploadParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobContentInfo Upload(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const UploadOptions& options)
      {
        auto request = UploadConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return UploadParseResponse(*response);
      }

      struct StageBlockOptions
      {
        std::string Version;
        std::string Date;
        std::vector<uint8_t>* BodyBuffer = nullptr;
        Azure::Core::Http::BodyStream* BodyStream = nullptr;
        std::string BlockId;
        std::string ContentMD5;
        std::string ContentCRC64;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
      }; // struct StageBlockOptions

      static Azure::Core::Http::Request StageBlockConstructRequest(
          const std::string& url,
          const StageBlockOptions& options)
      {
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Put, url, *options.BodyBuffer);
        request.AddHeader("Content-Length", std::to_string(options.BodyBuffer->size()));
        request.AddQueryParameter("comp", "block");
        request.AddQueryParameter("blockid", options.BlockId);
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        if (!options.ContentMD5.empty())
        {
          request.AddHeader("Content-MD5", options.ContentMD5);
        }
        if (!options.ContentCRC64.empty())
        {
          request.AddHeader("x-ms-content-crc64", options.ContentCRC64);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        return request;
      }

      static BlockInfo StageBlockParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlockInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        auto response_contentmd5_iterator = http_response.GetHeaders().find("Content-MD5");
        if (response_contentmd5_iterator != http_response.GetHeaders().end())
        {
          response.ContentMD5 = response_contentmd5_iterator->second;
        }
        auto response_contentcrc64_iterator = http_response.GetHeaders().find("x-ms-content-crc64");
        if (response_contentcrc64_iterator != http_response.GetHeaders().end())
        {
          response.ContentCRC64 = response_contentcrc64_iterator->second;
        }
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlockInfo StageBlock(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const StageBlockOptions& options)
      {
        auto request = StageBlockConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return StageBlockParseResponse(*response);
      }

      struct CommitBlockListOptions
      {
        std::string Version;
        std::string Date;
        std::vector<std::pair<BlockType, std::string>> BlockList;
        BlobHttpHeaders Properties;
        std::map<std::string, std::string> Metadata;
        std::string LeaseId;
        std::string EncryptionKey;
        std::string EncryptionKeySHA256;
        std::string EncryptionAlgorithm;
        AccessTier Tier = AccessTier::Unknown;
      }; // struct CommitBlockListOptions

      static Azure::Core::Http::Request CommitBlockListConstructRequest(
          const std::string& url,
          const CommitBlockListOptions& options)
      {
        // TODO: Think about how to initialize
        // xmlInitParser();
        // TODO: Think about how to free doc on exception
        using namespace libXML2;
        xmlDocPtr doc = xmlNewDoc(BAD_CAST("1.0"));
        xmlNodePtr block_list_node = xmlNewNode(nullptr, BAD_CAST("BlockList"));
        xmlDocSetRootElement(doc, block_list_node);

        for (const auto& block : options.BlockList)
        {
          const char* tag_name = nullptr;
          if (block.first == BlockType::Uncommitted)
            tag_name = "Uncommitted";
          else if (block.first == BlockType::Committed)
            tag_name = "Committed";
          else if (block.first == BlockType::Latest)
            tag_name = "Latest";
          else
            throw std::runtime_error("unexpected block type");

          xmlNewChild(block_list_node, nullptr, BAD_CAST(tag_name), BAD_CAST(block.second.data()));
        }

        xmlChar* xml_dump;
        int xml_dump_size;
        xmlDocDumpMemory(doc, &xml_dump, &xml_dump_size);
        xmlFreeDoc(doc);
        if (xml_dump == nullptr)
        {
          throw std::runtime_error("failed to allocate memory when building xml body");
        }

        // TODO: Think about how to free memory
        // xmlFree(xml_dump);

        // TODO: Think about how to cleanup
        // xmlCleanupParser();

        // TODO: Think about how to avoid copy
        std::vector<uint8_t> body_buffer(
            static_cast<const uint8_t*>(xml_dump),
            static_cast<const uint8_t*>(xml_dump) + xml_dump_size);
        // TODO: Set Content-MD5 or x-ms-content-crc64 header
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, body_buffer);
        request.AddHeader("Content-Length", std::to_string(body_buffer.size()));
        request.AddQueryParameter("comp", "blocklist");
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        if (!options.Properties.ContentType.empty())
        {
          request.AddHeader("x-ms-blob-content-type", options.Properties.ContentType);
        }
        if (!options.Properties.ContentEncoding.empty())
        {
          request.AddHeader("x-ms-blob-content-encoding", options.Properties.ContentEncoding);
        }
        if (!options.Properties.ContentLanguage.empty())
        {
          request.AddHeader("x-ms-blob-content-language", options.Properties.ContentLanguage);
        }
        if (!options.Properties.CacheControl.empty())
        {
          request.AddHeader("x-ms-blob-cache-control", options.Properties.CacheControl);
        }
        if (!options.Properties.ContentMD5.empty())
        {
          request.AddHeader("x-ms-blob-content-md5", options.Properties.ContentMD5);
        }
        if (!options.Properties.ContentDisposition.empty())
        {
          request.AddHeader("x-ms-blob-content-disposition", options.Properties.ContentDisposition);
        }
        for (const auto& pair : options.Metadata)
        {
          request.AddHeader("x-ms-meta-" + pair.first, pair.second);
        }
        if (!options.LeaseId.empty())
        {
          request.AddHeader("x-ms-lease-id", options.LeaseId);
        }
        if (!options.EncryptionKey.empty())
        {
          request.AddHeader("x-ms-encryption-key", options.EncryptionKey);
        }
        if (!options.EncryptionKeySHA256.empty())
        {
          request.AddHeader("x-ms-encryption-key-sha256", options.EncryptionKeySHA256);
        }
        if (!options.EncryptionAlgorithm.empty())
        {
          request.AddHeader("x-ms-encryption-algorithm", options.EncryptionAlgorithm);
        }
        auto options_tier_str = AccessTierToString(options.Tier);
        if (!options_tier_str.empty())
        {
          request.AddHeader("x-ms-access-tier", options_tier_str);
        }
        return request;
      }

      static BlobContentInfo CommitBlockListParseResponse(
          Azure::Core::Http::Response& http_response)
      {
        BlobContentInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 201))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        auto response_serverencrypted_iterator
            = http_response.GetHeaders().find("x-ms-server-encrypted");
        if (response_serverencrypted_iterator != http_response.GetHeaders().end())
        {
          response.ServerEncrypted = response_serverencrypted_iterator->second == "true";
        }
        auto response_encryptionkeysha256_iterator
            = http_response.GetHeaders().find("x-ms-encryption-key-sha256");
        if (response_encryptionkeysha256_iterator != http_response.GetHeaders().end())
        {
          response.EncryptionKeySHA256 = response_encryptionkeysha256_iterator->second;
        }
        return response;
      }

      static BlobContentInfo CommitBlockList(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const CommitBlockListOptions& options)
      {
        auto request = CommitBlockListConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return CommitBlockListParseResponse(*response);
      }

      struct GetBlockListOptions
      {
        std::string Version;
        std::string Date;
        BlockListTypeOption ListType = BlockListTypeOption::All;
      }; // struct GetBlockListOptions

      static Azure::Core::Http::Request GetBlockListConstructRequest(
          const std::string& url,
          const GetBlockListOptions& options)
      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        request.AddHeader("Content-Length", "0");
        request.AddQueryParameter("comp", "blocklist");
        std::string block_list_type_option = BlockListTypeOptionToString(options.ListType);
        if (!block_list_type_option.empty())
        {
          request.AddQueryParameter("blocklisttype", block_list_type_option);
        }
        request.AddHeader("x-ms-version", options.Version);
        request.AddHeader("x-ms-date", options.Date);
        return request;
      }

      static BlobBlockListInfo GetBlockListParseResponse(Azure::Core::Http::Response& http_response)
      {
        BlobBlockListInfo response;
        auto http_status_code
            = static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                http_response.GetStatusCode());
        if (!(http_status_code == 200))
        {
          throw std::runtime_error("HTTP status code " + std::to_string(http_status_code));
        }
        response.Version = http_response.GetHeaders().at("x-ms-version");
        response.Date = http_response.GetHeaders().at("Date");
        response.RequestId = http_response.GetHeaders().at("x-ms-request-id");
        auto response_clientrequestid_iterator
            = http_response.GetHeaders().find("x-ms-client-request-id");
        if (response_clientrequestid_iterator != http_response.GetHeaders().end())
        {
          response.ClientRequestId = response_clientrequestid_iterator->second;
        }
        response.ETag = http_response.GetHeaders().at("ETag");
        response.LastModified = http_response.GetHeaders().at("Last-Modified");
        response.ContentType = http_response.GetHeaders().at("Content-Type");
        response.ContentLength
            = std::stoull(http_response.GetHeaders().at("x-ms-blob-content-length"));
        // TODO: Think about how to initialize
        // xmlInitParser();
        // TODO: Think about how to free doc on exception

        // TODO: Think about how to hanlde xml > 2GB
        using namespace libXML2;
        xmlDoc* doc = xmlReadMemory(
            reinterpret_cast<const char*>(http_response.GetBodyBuffer().data()),
            int(http_response.GetBodyBuffer().size()),
            nullptr,
            nullptr,
            0);
        if (doc == nullptr)
          throw std::runtime_error("failed to parse response xml");

        xmlNode* root = xmlDocGetRootElement(doc);
        if (root == nullptr
            || std::string(reinterpret_cast<const char*>(root->name)) != "BlockList")
          throw std::runtime_error("failed to parse response xml");

        enum
        {
          start_tag,
          attribute,
          content,
          end_tag,
        };

        auto parse_xml_callback
            = [&response,
               blob_block = BlobBlock(),
               in_committed_block = false,
               in_uncommitted_block
               = false](const std::string& name, int type, const std::string& value) mutable {
                if (type == start_tag && name == "CommittedBlocks")
                  in_committed_block = true;
                else if (type == end_tag && name == "CommittedBlocks")
                  in_committed_block = false;
                else if (type == start_tag && name == "UncommittedBlocks")
                  in_uncommitted_block = true;
                else if (type == end_tag && name == "UncommittedBlocks")
                  in_uncommitted_block = false;
                else if (type == start_tag && name == "Block")
                  blob_block = BlobBlock();
                else if (type == end_tag && name == "Block" && in_committed_block)
                  response.CommittedBlocks.emplace_back(std::move(blob_block));
                else if (type == end_tag && name == "Block" && in_uncommitted_block)
                  response.UncommittedBlocks.emplace_back(std::move(blob_block));
                else if (type == content && name == "Name")
                  blob_block.Name = value;
                else if (type == content && name == "Size")
                  blob_block.Size = std::stoull(value);
              };

        std::function<void(xmlNode*)> parse_xml;
        parse_xml = [&parse_xml, &parse_xml_callback](xmlNode* node) {
          if (!(node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE))
            return;

          std::string node_name(reinterpret_cast<const char*>(node->name));
          parse_xml_callback(node_name, start_tag, "");

          for (xmlAttr* prop = node->properties; prop; prop = prop->next)
          {
            std::string prop_name(reinterpret_cast<const char*>(prop->name));
            std::string prop_value(reinterpret_cast<const char*>(prop->children->content));
            parse_xml_callback(prop_name, attribute, prop_value);
          }

          bool has_child_element = false;
          for (xmlNode* child = node->children; child; child = child->next)
          {
            has_child_element |= child->type == XML_ELEMENT_NODE;
            parse_xml(child);
          }

          if (!has_child_element && node->children)
          {
            std::string node_content(reinterpret_cast<const char*>(node->children->content));
            parse_xml_callback(node_name, content, node_content);
          }

          parse_xml_callback(node_name, end_tag, "");
        };

        parse_xml(root);

        xmlFreeDoc(doc);

        // TODO: Think about how to cleanup
        // xmlCleanupParser();
        return response;
      }

      static BlobBlockListInfo GetBlockList(
          Azure::Core::Context context,
          Azure::Core::Http::HttpPipeline& pipeline,
          const std::string& url,
          const GetBlockListOptions& options)
      {
        auto request = GetBlockListConstructRequest(url, options);
        auto response = pipeline.Send(context, request);
        return GetBlockListParseResponse(*response);
      }

    }; // class BlockBlob

  }; // class BlobRestClient
}}} // namespace Azure::Storage::Blobs
