// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/blobs/blob_container_client.hpp"

#include "azure/storage/blobs/append_blob_client.hpp"
#include "azure/storage/blobs/blob_batch.hpp"
#include "azure/storage/blobs/block_blob_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"
#include "private/package_version.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <unordered_map>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 6385)
#pragma warning(disable : 28251)
#endif

#include <nanoarrow/nanoarrow.hpp>
#include <nanoarrow/nanoarrow_ipc.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Azure { namespace Storage { namespace Blobs {

  namespace {
    Models::BlobItem BlobItemConversion(Models::_detail::BlobItem& item)
    {
      Models::BlobItem blobItem;
      if (item.Name.Encoded)
      {
        blobItem.Name = Core::Url::Decode(item.Name.Content);
      }
      else
      {
        blobItem.Name = std::move(item.Name.Content);
      }
      blobItem.IsDeleted = item.IsDeleted;
      blobItem.Snapshot = std::move(item.Snapshot);
      blobItem.VersionId = std::move(item.VersionId);
      blobItem.IsCurrentVersion = item.IsCurrentVersion;
      blobItem.Details = std::move(item.Details);
      blobItem.HasVersionsOnly = item.HasVersionsOnly;
      blobItem.BlobSize = item.BlobSize;
      blobItem.BlobType = std::move(item.BlobType);
      if (blobItem.Details.AccessTier.HasValue()
          && !blobItem.Details.IsAccessTierInferred.HasValue())
      {
        blobItem.Details.IsAccessTierInferred = false;
      }
      if (blobItem.VersionId.HasValue())
      {
        if (!blobItem.HasVersionsOnly.HasValue())
        {
          blobItem.HasVersionsOnly = false;
        }
        if (!blobItem.IsCurrentVersion.HasValue())
        {
          blobItem.IsCurrentVersion = false;
        }
      }
      if (blobItem.BlobType == Models::BlobType::AppendBlob
          && !blobItem.Details.IsSealed.HasValue())
      {
        blobItem.Details.IsSealed = false;
      }
      if (blobItem.Details.CopyStatus.HasValue() && !blobItem.Details.IsIncrementalCopy.HasValue())
      {
        blobItem.Details.IsIncrementalCopy = false;
      }
      {
        /*
         * Object replication metadata is in the following format.
         * <OrMetadata>
         *   <or-{policy_id}_{rule_id}>replication status</>
         *   <...>
         * </OrMetadata>
         *
         * We'll convert the metadata to a vector of policies, each policy being a vector of rules.
         */
        std::map<std::string, std::vector<Models::ObjectReplicationRule>> orPropertiesMap;
        for (auto& policy : blobItem.Details.ObjectReplicationSourceProperties)
        {
          for (auto& rule : policy.Rules)
          {
            auto underscorePos = rule.RuleId.find('_', 3);
            std::string policyId
                = std::string(rule.RuleId.begin() + 3, rule.RuleId.begin() + underscorePos);
            std::string ruleId = rule.RuleId.substr(underscorePos + 1);
            rule.RuleId = ruleId;
            orPropertiesMap[policyId].emplace_back(std::move(rule));
          }
        }
        blobItem.Details.ObjectReplicationSourceProperties.clear();
        for (auto& property : orPropertiesMap)
        {
          Models::ObjectReplicationPolicy policy;
          policy.PolicyId = property.first;
          policy.Rules = std::move(property.second);
          blobItem.Details.ObjectReplicationSourceProperties.emplace_back(std::move(policy));
        }
      }
      return blobItem;
    }

    void ParseListBlobsResultFromArrow(Models::_detail::ListBlobsResult& result)
    {
      const std::vector<uint8_t> responseBody = result.BodyStream->ReadToEnd();

      int ret = NANOARROW_OK;
      auto checkNanoarrowError = [&ret]() {
        if (ret != NANOARROW_OK)
        {
          throw StorageException("Failed to parse Apache Arrow IPC response body");
        }
      };

      nanoarrow::UniqueBuffer buffer;
      ArrowBufferInit(buffer.get());
      ArrowBufferView responseBodyBufferView;
      responseBodyBufferView.data.data = responseBody.data();
      responseBodyBufferView.size_bytes = responseBody.size();
      ret = ArrowBufferAppendBufferView(buffer.get(), responseBodyBufferView);
      checkNanoarrowError();

      nanoarrow::ipc::UniqueInputStream inputStream;
      ret = ArrowIpcInputStreamInitBuffer(inputStream.get(), buffer.get());
      checkNanoarrowError();

      nanoarrow::UniqueArrayStream arrayStream;
      ret = ArrowIpcArrayStreamReaderInit(arrayStream.get(), inputStream.get(), nullptr);
      checkNanoarrowError();

      nanoarrow::UniqueSchema schema;
      ret = arrayStream->get_schema(arrayStream.get(), schema.get());
      checkNanoarrowError();

      if (schema->metadata)
      {
        ArrowMetadataReader reader;
        ret = ArrowMetadataReaderInit(&reader, schema->metadata);
        checkNanoarrowError();

        ArrowStringView keyView;
        ArrowStringView valueView;
        while (ArrowMetadataReaderRead(&reader, &keyView, &valueView) == NANOARROW_OK)
        {
          const std::string key(keyView.data, static_cast<size_t>(keyView.size_bytes));
          std::string value(valueView.data, static_cast<size_t>(valueView.size_bytes));
          if (key == "NumberOfRecords")
          {
            result.Items.reserve(static_cast<size_t>(std::stoull(value)));
          }
          else if (key == "NextMarker")
          {
            result.ContinuationToken = std::move(value);
          }
        }
      }

      nanoarrow::UniqueArrayView arrayView;
      ret = ArrowArrayViewInitFromSchema(arrayView.get(), schema.get(), nullptr);
      checkNanoarrowError();

      while (true)
      {
        nanoarrow::UniqueArray array;
        ret = arrayStream->get_next(arrayStream.get(), array.get());
        checkNanoarrowError();
        if (array->release == nullptr)
        {
          break;
        }

        const size_t batchSize = static_cast<size_t>(array->length);
        const size_t batchRowStartOffset = result.Items.size();
        result.Items.resize(result.Items.size() + batchSize);

        ret = ArrowArrayViewSetArray(arrayView.get(), array.get(), nullptr);
        checkNanoarrowError();

        const int64_t numRows = arrayView->length;
        const int64_t numCols = arrayView->n_children;
        std::string parsedStringValue;
        int64_t parsedIntValue = 0;
        uint64_t parsedUintValue = 0;
        std::map<std::string, std::string> parsedMapValue;
        for (int c = 0; c < numCols; ++c)
        {
          const std::string columnName = schema->children[c]->name;
          const auto columnView = arrayView->children[c];
          const auto valueType = columnView->storage_type;
          for (int r = 0; r < numRows; ++r)
          {
            if (ArrowArrayViewIsNull(columnView, r))
            {
              continue;
            }
            switch (valueType)
            {
              case NANOARROW_TYPE_STRING: {
                ArrowStringView stringView = ArrowArrayViewGetStringUnsafe(columnView, r);
                parsedStringValue.assign(
                    stringView.data, static_cast<size_t>(stringView.size_bytes));
                break;
              }
              case NANOARROW_TYPE_MAP: {
                const int64_t startOffset = ArrowArrayViewListChildOffset(columnView, r);
                const int64_t endOffset = ArrowArrayViewListChildOffset(columnView, r + 1);
                const auto mapView = columnView->children[0];
                const auto keyView = mapView->children[0];
                const auto valueView = mapView->children[1];
                for (int64_t i = startOffset; i < endOffset; ++i)
                {
                  ArrowStringView stringView = ArrowArrayViewGetStringUnsafe(keyView, i);
                  std::string key(stringView.data, static_cast<size_t>(stringView.size_bytes));
                  stringView = ArrowArrayViewGetStringUnsafe(valueView, i);
                  std::string value(stringView.data, static_cast<size_t>(stringView.size_bytes));
                  parsedMapValue.emplace(std::move(key), std::move(value));
                }
                break;
              }
              case NANOARROW_TYPE_INT64:
              case NANOARROW_TYPE_BOOL:
                parsedIntValue = ArrowArrayViewGetIntUnsafe(columnView, r);
                break;
              case NANOARROW_TYPE_UINT64:
                parsedUintValue = ArrowArrayViewGetUIntUnsafe(columnView, r);
                break;
              default:
                ret = -1;
                checkNanoarrowError();
            }
            if (columnName == "Name")
            {
              result.Items[batchRowStartOffset + r].Name.Encoded = false;
              result.Items[batchRowStartOffset + r].Name.Content = std::move(parsedStringValue);
            }
            else if (columnName == "Creation-Time")
            {
              result.Items[batchRowStartOffset + r].Details.CreatedOn
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "Last-Modified")
            {
              result.Items[batchRowStartOffset + r].Details.LastModified
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "BlobType")
            {
              result.Items[batchRowStartOffset + r].BlobType
                  = Models::BlobType(std::move(parsedStringValue));
            }
            else if (columnName == "ResourceType")
            {
              result.Items[batchRowStartOffset + r].ResourceType = std::move(parsedStringValue);
            }
            else if (columnName == "Etag")
            {
              result.Items[batchRowStartOffset + r].Details.ETag
                  = Azure::ETag(std::move(parsedStringValue));
            }
            else if (columnName == "Content-Length")
            {
              result.Items[batchRowStartOffset + r].BlobSize = parsedUintValue;
            }
            else if (columnName == "Content-Type")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentType
                  = std::move(parsedStringValue);
            }
            else if (columnName == "Content-Encoding")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentEncoding
                  = std::move(parsedStringValue);
            }
            else if (columnName == "Content-Language")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentLanguage
                  = std::move(parsedStringValue);
            }
            else if (columnName == "Content-CRC64")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentHash.Value
                  = Core::Convert::Base64Decode(parsedStringValue);
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentHash.Algorithm
                  = Storage::HashAlgorithm::Crc64;
            }
            else if (columnName == "Content-MD5")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentHash.Value
                  = Core::Convert::Base64Decode(parsedStringValue);
            }
            else if (columnName == "Content-Disposition")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.ContentDisposition
                  = std::move(parsedStringValue);
            }
            else if (columnName == "Cache-Control")
            {
              result.Items[batchRowStartOffset + r].Details.HttpHeaders.CacheControl
                  = std::move(parsedStringValue);
            }
            else if (columnName == "x-ms-blob-sequence-number")
            {
              result.Items[batchRowStartOffset + r].Details.SequenceNumber = parsedUintValue;
            }
            else if (columnName == "AccessTier")
            {
              result.Items[batchRowStartOffset + r].Details.AccessTier
                  = Models::AccessTier(std::move(parsedStringValue));
            }
            else if (columnName == "AccessTierInferred")
            {
              result.Items[batchRowStartOffset + r].Details.IsAccessTierInferred
                  = parsedIntValue != 0;
            }
            else if (columnName == "AccessTierChangeTime")
            {
              result.Items[batchRowStartOffset + r].Details.AccessTierChangedOn
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "SmartAccessTier")
            {
              result.Items[batchRowStartOffset + r].Details.SmartAccessTier
                  = Models::AccessTier(std::move(parsedStringValue));
            }
            else if (columnName == "LeaseState")
            {
              result.Items[batchRowStartOffset + r].Details.LeaseState
                  = Models::LeaseState(std::move(parsedStringValue));
            }
            else if (columnName == "LeaseStatus")
            {
              result.Items[batchRowStartOffset + r].Details.LeaseStatus
                  = Models::LeaseStatus(std::move(parsedStringValue));
            }
            else if (columnName == "LeaseDuration")
            {
              result.Items[batchRowStartOffset + r].Details.LeaseDuration
                  = Models::LeaseDurationType(std::move(parsedStringValue));
            }
            else if (columnName == "IncrementalCopy")
            {
              result.Items[batchRowStartOffset + r].Details.IsIncrementalCopy = parsedIntValue != 0;
            }
            else if (columnName == "ServerEncrypted")
            {
              result.Items[batchRowStartOffset + r].Details.IsServerEncrypted = parsedIntValue != 0;
            }
            else if (columnName == "CustomerProvidedKeySha256")
            {
              result.Items[batchRowStartOffset + r].Details.EncryptionKeySha256
                  = Core::Convert::Base64Decode(parsedStringValue);
            }
            else if (columnName == "EncryptionScope")
            {
              result.Items[batchRowStartOffset + r].Details.EncryptionScope
                  = std::move(parsedStringValue);
            }
            else if (columnName == "RehydratePriority")
            {
              result.Items[batchRowStartOffset + r].Details.RehydratePriority
                  = Models::RehydratePriority(std::move(parsedStringValue));
            }
            else if (columnName == "Sealed")
            {
              result.Items[batchRowStartOffset + r].Details.IsSealed = parsedIntValue != 0;
            }
            else if (columnName == "ArchiveStatus")
            {
              result.Items[batchRowStartOffset + r].Details.ArchiveStatus
                  = Models::ArchiveStatus(std::move(parsedStringValue));
            }
            else if (columnName == "CopyId")
            {
              result.Items[batchRowStartOffset + r].Details.CopyId = std::move(parsedStringValue);
            }
            else if (columnName == "CopyStatus")
            {
              result.Items[batchRowStartOffset + r].Details.CopyStatus
                  = Models::CopyStatus(std::move(parsedStringValue));
            }
            else if (columnName == "CopySource")
            {
              result.Items[batchRowStartOffset + r].Details.CopySource
                  = std::move(parsedStringValue);
            }
            else if (columnName == "CopyProgress")
            {
              result.Items[batchRowStartOffset + r].Details.CopyProgress
                  = std::move(parsedStringValue);
            }
            else if (columnName == "CopyCompletionTime")
            {
              result.Items[batchRowStartOffset + r].Details.CopyCompletedOn
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "CopyStatusDescription")
            {
              result.Items[batchRowStartOffset + r].Details.CopyStatusDescription
                  = std::move(parsedStringValue);
            }
            else if (columnName == "CopyDestinationSnapshot")
            {
              result.Items[batchRowStartOffset + r].Details.IncrementalCopyDestinationSnapshot
                  = std::move(parsedStringValue);
            }
            else if (columnName == "ImmutabilityPolicyUntilDate")
            {
              result.Items[batchRowStartOffset + r].Details.ImmutabilityPolicy.Value().ExpiresOn
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "ImmutabilityPolicyMode")
            {
              result.Items[batchRowStartOffset + r].Details.ImmutabilityPolicy.Value().PolicyMode
                  = Models::BlobImmutabilityPolicyMode(std::move(parsedStringValue));
            }
            else if (columnName == "VersionId")
            {
              result.Items[batchRowStartOffset + r].VersionId = std::move(parsedStringValue);
            }
            else if (columnName == "IsCurrentVersion")
            {
              result.Items[batchRowStartOffset + r].IsCurrentVersion = parsedIntValue != 0;
            }
            else if (columnName == "Snapshot")
            {
              result.Items[batchRowStartOffset + r].Snapshot = std::move(parsedStringValue);
            }
            else if (columnName == "LegalHold")
            {
              result.Items[batchRowStartOffset + r].Details.HasLegalHold = parsedIntValue != 0;
            }
            else if (columnName == "Deleted")
            {
              result.Items[batchRowStartOffset + r].IsDeleted = parsedIntValue != 0;
            }
            else if (columnName == "HasVersionsOnly")
            {
              result.Items[batchRowStartOffset + r].HasVersionsOnly = parsedIntValue != 0;
            }
            else if (columnName == "DeletedTime")
            {
              result.Items[batchRowStartOffset + r].Details.DeletedOn
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "RemainingRetentionDays")
            {
              result.Items[batchRowStartOffset + r].Details.RemainingRetentionDays
                  = static_cast<int32_t>(parsedUintValue);
            }
            else if (columnName == "LastAccessTime")
            {
              result.Items[batchRowStartOffset + r].Details.LastAccessedOn
                  = std::chrono::system_clock::time_point(std::chrono::seconds(parsedIntValue));
            }
            else if (columnName == "Tags")
            {
              result.Items[batchRowStartOffset + r].Details.Tags = std::move(parsedMapValue);
            }
            else if (columnName == "OrMetadata")
            {
              for (auto& i : parsedMapValue)
              {
                Models::ObjectReplicationPolicy policy;
                Models::ObjectReplicationRule rule;
                policy.PolicyId = i.first;
                rule.RuleId = i.first;
                rule.ReplicationStatus = Models::ObjectReplicationStatus(std::move(i.second));
                policy.Rules.push_back(std::move(rule));
                result.Items[batchRowStartOffset + r]
                    .Details.ObjectReplicationSourceProperties.push_back(std::move(policy));
              }
            }
            else if (columnName == "OrsPolicySourceBlob")
            {
            }
            else if (columnName == "TagCount")
            {
            }
            else if (columnName == "Metadata")
            {
              for (auto& i : parsedMapValue)
              {
                result.Items[batchRowStartOffset + r].Details.Metadata.emplace(
                    i.first, std::move(i.second));
              }
            }
            else if (columnName == "Encrypted")
            {
            }
            else if (columnName == "AffinityId")
            {
            }
            parsedStringValue.clear();
            parsedMapValue.clear();
          }
        }
      }
      for (const auto& i : result.Items)
      {
        if (i.ResourceType.HasValue() && i.ResourceType.Value() == "blobprefix")
        {
          result.BlobPrefixes.push_back(i.Name);
        }
      }
      result.Items.erase(
          std::remove_if(
              result.Items.begin(),
              result.Items.end(),
              [](const auto& i) {
                return i.ResourceType.HasValue() && i.ResourceType.Value() == "blobprefix";
              }),
          result.Items.end());
    }

    void ParseListBlobsResultFromXml(Models::_detail::ListBlobsResult& result)
    {
      const std::vector<uint8_t> responseBody = result.BodyStream->ReadToEnd();
      _internal::XmlReader reader(
          reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
      enum class XmlTagEnum
      {
        kUnknown,
        kEnumerationResults,
        kPrefix,
        kDelimiter,
        kNextMarker,
        kBlobs,
        kBlob,
        kName,
        kDeleted,
        kSnapshot,
        kVersionId,
        kIsCurrentVersion,
        kProperties,
        kCreationTime,
        kLastModified,
        kEtag,
        kXMsBlobSequenceNumber,
        kLeaseStatus,
        kLeaseState,
        kLeaseDuration,
        kCopyId,
        kCopyStatus,
        kCopySource,
        kCopyProgress,
        kCopyCompletionTime,
        kCopyStatusDescription,
        kServerEncrypted,
        kIncrementalCopy,
        kCopyDestinationSnapshot,
        kDeletedTime,
        kRemainingRetentionDays,
        kAccessTier,
        kAccessTierInferred,
        kArchiveStatus,
        kSmartAccessTier,
        kCustomerProvidedKeySha256,
        kEncryptionScope,
        kAccessTierChangeTime,
        kExpiryTime,
        kSealed,
        kRehydratePriority,
        kLastAccessTime,
        kLegalHold,
        kContentType,
        kContentEncoding,
        kContentLanguage,
        kContentMD5,
        kContentDisposition,
        kCacheControl,
        kMetadata,
        kTags,
        kTagSet,
        kTag,
        kKey,
        kValue,
        kOrMetadata,
        kImmutabilityPolicyUntilDate,
        kImmutabilityPolicyMode,
        kHasVersionsOnly,
        kContentLength,
        kBlobType,
        kDeletionId,
        kBlobPrefix,
      };
      const std::unordered_map<std::string, XmlTagEnum> XmlTagEnumMap{
          {"EnumerationResults", XmlTagEnum::kEnumerationResults},
          {"Prefix", XmlTagEnum::kPrefix},
          {"Delimiter", XmlTagEnum::kDelimiter},
          {"NextMarker", XmlTagEnum::kNextMarker},
          {"Blobs", XmlTagEnum::kBlobs},
          {"Blob", XmlTagEnum::kBlob},
          {"Name", XmlTagEnum::kName},
          {"Deleted", XmlTagEnum::kDeleted},
          {"Snapshot", XmlTagEnum::kSnapshot},
          {"VersionId", XmlTagEnum::kVersionId},
          {"IsCurrentVersion", XmlTagEnum::kIsCurrentVersion},
          {"Properties", XmlTagEnum::kProperties},
          {"Creation-Time", XmlTagEnum::kCreationTime},
          {"Last-Modified", XmlTagEnum::kLastModified},
          {"Etag", XmlTagEnum::kEtag},
          {"x-ms-blob-sequence-number", XmlTagEnum::kXMsBlobSequenceNumber},
          {"LeaseStatus", XmlTagEnum::kLeaseStatus},
          {"LeaseState", XmlTagEnum::kLeaseState},
          {"LeaseDuration", XmlTagEnum::kLeaseDuration},
          {"CopyId", XmlTagEnum::kCopyId},
          {"CopyStatus", XmlTagEnum::kCopyStatus},
          {"CopySource", XmlTagEnum::kCopySource},
          {"CopyProgress", XmlTagEnum::kCopyProgress},
          {"CopyCompletionTime", XmlTagEnum::kCopyCompletionTime},
          {"CopyStatusDescription", XmlTagEnum::kCopyStatusDescription},
          {"ServerEncrypted", XmlTagEnum::kServerEncrypted},
          {"IncrementalCopy", XmlTagEnum::kIncrementalCopy},
          {"CopyDestinationSnapshot", XmlTagEnum::kCopyDestinationSnapshot},
          {"DeletedTime", XmlTagEnum::kDeletedTime},
          {"RemainingRetentionDays", XmlTagEnum::kRemainingRetentionDays},
          {"AccessTier", XmlTagEnum::kAccessTier},
          {"AccessTierInferred", XmlTagEnum::kAccessTierInferred},
          {"ArchiveStatus", XmlTagEnum::kArchiveStatus},
          {"SmartAccessTier", XmlTagEnum::kSmartAccessTier},
          {"CustomerProvidedKeySha256", XmlTagEnum::kCustomerProvidedKeySha256},
          {"EncryptionScope", XmlTagEnum::kEncryptionScope},
          {"AccessTierChangeTime", XmlTagEnum::kAccessTierChangeTime},
          {"Expiry-Time", XmlTagEnum::kExpiryTime},
          {"Sealed", XmlTagEnum::kSealed},
          {"RehydratePriority", XmlTagEnum::kRehydratePriority},
          {"LastAccessTime", XmlTagEnum::kLastAccessTime},
          {"LegalHold", XmlTagEnum::kLegalHold},
          {"Content-Type", XmlTagEnum::kContentType},
          {"Content-Encoding", XmlTagEnum::kContentEncoding},
          {"Content-Language", XmlTagEnum::kContentLanguage},
          {"Content-MD5", XmlTagEnum::kContentMD5},
          {"Content-Disposition", XmlTagEnum::kContentDisposition},
          {"Cache-Control", XmlTagEnum::kCacheControl},
          {"Metadata", XmlTagEnum::kMetadata},
          {"Tags", XmlTagEnum::kTags},
          {"TagSet", XmlTagEnum::kTagSet},
          {"Tag", XmlTagEnum::kTag},
          {"Key", XmlTagEnum::kKey},
          {"Value", XmlTagEnum::kValue},
          {"OrMetadata", XmlTagEnum::kOrMetadata},
          {"ImmutabilityPolicyUntilDate", XmlTagEnum::kImmutabilityPolicyUntilDate},
          {"ImmutabilityPolicyMode", XmlTagEnum::kImmutabilityPolicyMode},
          {"HasVersionsOnly", XmlTagEnum::kHasVersionsOnly},
          {"Content-Length", XmlTagEnum::kContentLength},
          {"BlobType", XmlTagEnum::kBlobType},
          {"DeletionId", XmlTagEnum::kDeletionId},
          {"BlobPrefix", XmlTagEnum::kBlobPrefix},
      };
      std::vector<XmlTagEnum> xmlPath;
      Models::_detail::BlobItem vectorElement1;
      std::string mapKey2;
      std::string mapValue3;
      std::string mapKey4;
      std::string mapValue5;
      Models::ObjectReplicationPolicy vectorElement6;
      Models::ObjectReplicationRule vectorElement7;
      Models::_detail::BlobName vectorElement8;
      while (true)
      {
        auto node = reader.Read();
        if (node.Type == _internal::XmlNodeType::End)
        {
          break;
        }
        else if (node.Type == _internal::XmlNodeType::StartTag)
        {
          auto ite = XmlTagEnumMap.find(node.Name);
          xmlPath.push_back(ite == XmlTagEnumMap.end() ? XmlTagEnum::kUnknown : ite->second);
          if (xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kMetadata)
          {
            mapKey2 = node.Name;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kOrMetadata)
          {
            vectorElement6.PolicyId = node.Name;
            vectorElement7.RuleId = node.Name;
          }
          else if (
              ((xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
                && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
                && xmlPath[3] == XmlTagEnum::kProperties
                && xmlPath[4] == XmlTagEnum::kImmutabilityPolicyUntilDate)
               || (xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
                   && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
                   && xmlPath[3] == XmlTagEnum::kProperties
                   && xmlPath[4] == XmlTagEnum::kImmutabilityPolicyMode))
              && !vectorElement1.Details.ImmutabilityPolicy.HasValue())
          {
            vectorElement1.Details.ImmutabilityPolicy = Models::BlobImmutabilityPolicy();
          }
        }
        else if (node.Type == _internal::XmlNodeType::Text)
        {
          if (xmlPath.size() == 2 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kPrefix)
          {
            result.Prefix = node.Value;
          }
          else if (
              xmlPath.size() == 2 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kDelimiter)
          {
            result.Delimiter = node.Value;
          }
          else if (
              xmlPath.size() == 2 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kNextMarker)
          {
            result.ContinuationToken = node.Value;
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kName)
          {
            vectorElement1.Name.Content = node.Value;
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kDeleted)
          {
            vectorElement1.IsDeleted = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kSnapshot)
          {
            vectorElement1.Snapshot = node.Value;
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kVersionId)
          {
            vectorElement1.VersionId = node.Value;
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kIsCurrentVersion)
          {
            vectorElement1.IsCurrentVersion = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kCreationTime)
          {
            vectorElement1.Details.CreatedOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kLastModified)
          {
            vectorElement1.Details.LastModified
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kEtag)
          {
            vectorElement1.Details.ETag = ETag(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kXMsBlobSequenceNumber)
          {
            vectorElement1.Details.SequenceNumber = std::stoll(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kLeaseStatus)
          {
            vectorElement1.Details.LeaseStatus = Models::LeaseStatus(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kLeaseState)
          {
            vectorElement1.Details.LeaseState = Models::LeaseState(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kLeaseDuration)
          {
            vectorElement1.Details.LeaseDuration = Models::LeaseDurationType(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kCopyId)
          {
            vectorElement1.Details.CopyId = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kCopyStatus)
          {
            vectorElement1.Details.CopyStatus = Models::CopyStatus(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kCopySource)
          {
            vectorElement1.Details.CopySource = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kCopyProgress)
          {
            vectorElement1.Details.CopyProgress = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kCopyCompletionTime)
          {
            vectorElement1.Details.CopyCompletedOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kCopyStatusDescription)
          {
            vectorElement1.Details.CopyStatusDescription = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kServerEncrypted)
          {
            vectorElement1.Details.IsServerEncrypted = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kIncrementalCopy)
          {
            vectorElement1.Details.IsIncrementalCopy = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kCopyDestinationSnapshot)
          {
            vectorElement1.Details.IncrementalCopyDestinationSnapshot = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kDeletedTime)
          {
            vectorElement1.Details.DeletedOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kRemainingRetentionDays)
          {
            vectorElement1.Details.RemainingRetentionDays = std::stoi(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kAccessTier)
          {
            vectorElement1.Details.AccessTier = Models::AccessTier(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kAccessTierInferred)
          {
            vectorElement1.Details.IsAccessTierInferred = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kArchiveStatus)
          {
            vectorElement1.Details.ArchiveStatus = Models::ArchiveStatus(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kSmartAccessTier)
          {
            vectorElement1.Details.SmartAccessTier = Models::AccessTier(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kCustomerProvidedKeySha256)
          {
            vectorElement1.Details.EncryptionKeySha256 = Core::Convert::Base64Decode(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kEncryptionScope)
          {
            vectorElement1.Details.EncryptionScope = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kAccessTierChangeTime)
          {
            vectorElement1.Details.AccessTierChangedOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kExpiryTime)
          {
            vectorElement1.Details.ExpiresOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kSealed)
          {
            vectorElement1.Details.IsSealed = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kRehydratePriority)
          {
            vectorElement1.Details.RehydratePriority = Models::RehydratePriority(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kLastAccessTime)
          {
            vectorElement1.Details.LastAccessedOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kLegalHold)
          {
            vectorElement1.Details.HasLegalHold = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kContentType)
          {
            vectorElement1.Details.HttpHeaders.ContentType = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kContentEncoding)
          {
            vectorElement1.Details.HttpHeaders.ContentEncoding = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kContentLanguage)
          {
            vectorElement1.Details.HttpHeaders.ContentLanguage = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kContentMD5)
          {
            vectorElement1.Details.HttpHeaders.ContentHash.Value
                = Core::Convert::Base64Decode(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kContentDisposition)
          {
            vectorElement1.Details.HttpHeaders.ContentDisposition = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kCacheControl)
          {
            vectorElement1.Details.HttpHeaders.CacheControl = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kMetadata)
          {
            mapValue3 = node.Value;
          }
          else if (
              xmlPath.size() == 7 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kTags && xmlPath[4] == XmlTagEnum::kTagSet
              && xmlPath[5] == XmlTagEnum::kTag && xmlPath[6] == XmlTagEnum::kKey)
          {
            mapKey4 = node.Value;
          }
          else if (
              xmlPath.size() == 7 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kTags && xmlPath[4] == XmlTagEnum::kTagSet
              && xmlPath[5] == XmlTagEnum::kTag && xmlPath[6] == XmlTagEnum::kValue)
          {
            mapValue5 = node.Value;
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kOrMetadata)
          {
            vectorElement7.ReplicationStatus = Models::ObjectReplicationStatus(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kImmutabilityPolicyUntilDate)
          {
            vectorElement1.Details.ImmutabilityPolicy.Value().ExpiresOn
                = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties
              && xmlPath[4] == XmlTagEnum::kImmutabilityPolicyMode)
          {
            vectorElement1.Details.ImmutabilityPolicy.Value().PolicyMode
                = Models::BlobImmutabilityPolicyMode(node.Value);
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kHasVersionsOnly)
          {
            vectorElement1.HasVersionsOnly = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kContentLength)
          {
            vectorElement1.BlobSize = std::stoll(node.Value);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kProperties && xmlPath[4] == XmlTagEnum::kBlobType)
          {
            vectorElement1.BlobType = Models::BlobType(node.Value);
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kDeletionId)
          {
            vectorElement1.DeletionId = node.Value;
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlobPrefix
              && xmlPath[3] == XmlTagEnum::kName)
          {
            vectorElement8.Content = node.Value;
          }
        }
        else if (node.Type == _internal::XmlNodeType::Attribute)
        {
          if (xmlPath.size() == 1 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && node.Name == "ServiceEndpoint")
          {
            result.ServiceEndpoint = node.Value;
          }
          else if (
              xmlPath.size() == 1 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && node.Name == "ContainerName")
          {
            result.BlobContainerName = node.Value;
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kName && node.Name == "Encoded")
          {
            vectorElement1.Name.Encoded = node.Value == std::string("true");
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlobPrefix
              && xmlPath[3] == XmlTagEnum::kName && node.Name == "Encoded")
          {
            vectorElement8.Encoded = node.Value == std::string("true");
          }
        }
        else if (node.Type == _internal::XmlNodeType::EndTag)
        {
          if (xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kMetadata)
          {
            vectorElement1.Details.Metadata[std::move(mapKey2)] = std::move(mapValue3);
          }
          else if (
              xmlPath.size() == 7 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kTags && xmlPath[4] == XmlTagEnum::kTagSet
              && xmlPath[5] == XmlTagEnum::kTag && xmlPath[6] == XmlTagEnum::kValue)
          {
            vectorElement1.Details.Tags[std::move(mapKey4)] = std::move(mapValue5);
          }
          else if (
              xmlPath.size() == 5 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob
              && xmlPath[3] == XmlTagEnum::kOrMetadata)
          {
            vectorElement6.Rules.push_back(std::move(vectorElement7));
            vectorElement7 = Models::ObjectReplicationRule();
            vectorElement1.Details.ObjectReplicationSourceProperties.push_back(
                std::move(vectorElement6));
            vectorElement6 = Models::ObjectReplicationPolicy();
          }
          else if (
              xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlob)
          {
            result.Items.push_back(std::move(vectorElement1));
            vectorElement1 = Models::_detail::BlobItem();
          }
          else if (
              xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kEnumerationResults
              && xmlPath[1] == XmlTagEnum::kBlobs && xmlPath[2] == XmlTagEnum::kBlobPrefix
              && xmlPath[3] == XmlTagEnum::kName)
          {
            result.BlobPrefixes.push_back(std::move(vectorElement8));
            vectorElement8 = Models::_detail::BlobName();
          }
          xmlPath.pop_back();
        }
      }
    }
  } // namespace

  namespace _detail {
    void ParseListBlobsResult(Models::_detail::ListBlobsResult& result)
    {
      if (result.ContentType.find(_internal::ContentTypeXml) != std::string::npos)
      {
        return ParseListBlobsResultFromXml(result);
      }
      else if (
          result.ContentType.find(_internal::ContentTypeApacheArrowStream) != std::string::npos)
      {
        return ParseListBlobsResultFromArrow(result);
      }
      AZURE_UNREACHABLE_CODE();
    }
  } // namespace _detail

  BlobContainerClient BlobContainerClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto blobContainerUrl = std::move(parsedConnectionString.BlobServiceUrl);
    blobContainerUrl.AppendPath(_internal::UrlEncodePath(blobContainerName));

    if (parsedConnectionString.KeyCredential)
    {
      return BlobContainerClient(
          blobContainerUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobContainerClient(blobContainerUrl.GetAbsoluteUrl(), options);
    }
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
  {
    BlobClientOptions newOptions = options;
    auto sharedKeyAuthPolicy = std::make_unique<_internal::SharedKeyPolicy>(credential);
    newOptions.PerRetryPolicies.emplace_back(sharedKeyAuthPolicy->Clone());

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_blobContainerUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));

    m_batchRequestPipeline
        = _detail::ConstructBatchRequestPolicy(perRetryPolicies, perOperationPolicies, newOptions);

    m_batchSubrequestPipeline
        = _detail::ConstructBatchSubrequestPolicy(nullptr, std::move(sharedKeyAuthPolicy), options);

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<const Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_blobContainerUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> tokenAuthPolicy;
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(
          options.Audience.HasValue()
              ? _internal::GetDefaultScopeForAudience(options.Audience.Value().ToString())
              : _internal::StorageScope);
      _internal::SessionOptions sessionOptions;
      if (options.SessionOptions.Mode == SessionMode::Enabled)
      {
        sessionOptions.Enabled = true;
        sessionOptions.AccountName = options.SessionOptions.AccountName;
      }
      tokenAuthPolicy = std::make_unique<_internal::StorageBearerTokenAuthenticationPolicy>(
          credential, tokenContext, options.EnableTenantDiscovery, sessionOptions);
      perRetryPolicies.emplace_back(tokenAuthPolicy->Clone());
    }
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));

    m_batchRequestPipeline
        = _detail::ConstructBatchRequestPolicy(perRetryPolicies, perOperationPolicies, options);

    m_batchSubrequestPipeline
        = _detail::ConstructBatchSubrequestPolicy(std::move(tokenAuthPolicy), nullptr, options);

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      const BlobClientOptions& options)
      : m_blobContainerUrl(blobContainerUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope),
        m_uploadValidationOptions(options.UploadValidationOptions),
        m_downloadValidationOptions(options.DownloadValidationOptions)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_blobContainerUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));

    m_batchRequestPipeline
        = _detail::ConstructBatchRequestPolicy(perRetryPolicies, perOperationPolicies, options);

    m_batchSubrequestPipeline = _detail::ConstructBatchSubrequestPolicy(nullptr, nullptr, options);

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobClient BlobContainerClient::GetBlobClient(const std::string& blobName) const
  {
    auto blobUrl = m_blobContainerUrl;
    blobUrl.AppendPath(_internal::UrlEncodePath(blobName));
    return BlobClient(
        std::move(blobUrl),
        m_pipeline,
        m_customerProvidedKey,
        m_encryptionScope,
        m_uploadValidationOptions,
        m_downloadValidationOptions);
  }

  BlockBlobClient BlobContainerClient::GetBlockBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).AsBlockBlobClient();
  }

  AppendBlobClient BlobContainerClient::GetAppendBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).AsAppendBlobClient();
  }

  PageBlobClient BlobContainerClient::GetPageBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).AsPageBlobClient();
  }

  Azure::Response<Models::CreateBlobContainerResult> BlobContainerClient::Create(
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::CreateBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.Access = options.AccessType;
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    protocolLayerOptions.DefaultEncryptionScope = options.DefaultEncryptionScope;
    protocolLayerOptions.PreventEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    return _detail::BlobContainerClient::Create(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::CreateBlobContainerResult> BlobContainerClient::CreateIfNotExists(
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::Conflict
          && e.ErrorCode == "ContainerAlreadyExists")
      {
        Models::CreateBlobContainerResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreateBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteBlobContainerResult> BlobContainerClient::Delete(
      const DeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::DeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::BlobContainerClient::Delete(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::DeleteBlobContainerResult> BlobContainerClient::DeleteIfExists(
      const DeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::NotFound
          && e.ErrorCode == "ContainerNotFound")
      {
        Models::DeleteBlobContainerResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeleteBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::BlobContainerProperties> BlobContainerClient::GetProperties(
      const GetBlobContainerPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::GetBlobContainerPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::BlobContainerClient::GetProperties(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::SetBlobContainerMetadataResult> BlobContainerClient::SetMetadata(
      Metadata metadata,
      SetBlobContainerMetadataOptions options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::SetBlobContainerMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(metadata.begin(), metadata.end());
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    return _detail::BlobContainerClient::SetMetadata(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
  }

  ListBlobsPagedResponse BlobContainerClient::ListBlobs(
      const ListBlobsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::ListBlobContainerBlobsOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    protocolLayerOptions.StartFrom = options.StartFrom;
    protocolLayerOptions.EndBefore = options.EndBefore;
    if (options.UseApacheArrow)
    {
      protocolLayerOptions.Accept = _internal::ContentTypeApacheArrowStream;
    }
    auto response = _detail::BlobContainerClient::ListBlobs(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
    _detail::ParseListBlobsResult(response.Value);

    ListBlobsPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.BlobContainerName = std::move(response.Value.BlobContainerName);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    for (auto& i : response.Value.Items)
    {
      pagedResponse.Blobs.push_back(BlobItemConversion(i));
    }
    pagedResponse.m_blobContainerClient = std::make_shared<BlobContainerClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  ListBlobsByHierarchyPagedResponse BlobContainerClient::ListBlobsByHierarchy(
      const std::string& delimiter,
      const ListBlobsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::ListBlobContainerBlobsByHierarchyOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = delimiter;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    protocolLayerOptions.StartFrom = options.StartFrom;
    protocolLayerOptions.EndBefore = options.EndBefore;
    if (options.UseApacheArrow)
    {
      protocolLayerOptions.Accept = _internal::ContentTypeApacheArrowStream;
    }
    auto response = _detail::BlobContainerClient::ListBlobsByHierarchy(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
    _detail::ParseListBlobsResult(response.Value);

    ListBlobsByHierarchyPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.BlobContainerName = std::move(response.Value.BlobContainerName);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.Delimiter = std::move(response.Value.Delimiter);
    for (auto& i : response.Value.Items)
    {
      pagedResponse.Blobs.push_back(BlobItemConversion(i));
    }
    for (auto& i : response.Value.BlobPrefixes)
    {
      if (i.Encoded)
      {
        pagedResponse.BlobPrefixes.push_back(Core::Url::Decode(i.Content));
      }
      else
      {
        pagedResponse.BlobPrefixes.push_back(std::move(i.Content));
      }
    }
    pagedResponse.m_blobContainerClient = std::make_shared<BlobContainerClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_delimiter = delimiter;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    if (options.UseApacheArrow)
    {
      pagedResponse.Delimiter = delimiter;
      if (options.Prefix.HasValue())
      {
        pagedResponse.Prefix = options.Prefix.Value();
      }
    }
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::BlobContainerAccessPolicy> BlobContainerClient::GetAccessPolicy(
      const GetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::GetBlobContainerAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::BlobContainerClient::GetAccessPolicy(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::SetBlobContainerAccessPolicyResult> BlobContainerClient::SetAccessPolicy(
      const SetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::SetBlobContainerAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.Access = options.AccessType;
    protocolLayerOptions.ContainerAcl = options.SignedIdentifiers;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::BlobContainerClient::SetAccessPolicy(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::DeleteBlobResult> BlobContainerClient::DeleteBlob(
      const std::string& blobName,
      const DeleteBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blobClient = GetBlobClient(blobName);
    return blobClient.Delete(options, context);
  }

  Azure::Response<BlockBlobClient> BlobContainerClient::UploadBlob(
      const std::string& blobName,
      Azure::Core::IO::BodyStream& content,
      const UploadBlockBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blockBlobClient = GetBlockBlobClient(blobName);
    auto response = blockBlobClient.Upload(content, options, context);
    return Azure::Response<BlockBlobClient>(
        std::move(blockBlobClient), std::move(response.RawResponse));
  }

  FindBlobsByTagsPagedResponse BlobContainerClient::FindBlobsByTags(
      const std::string& tagFilterSqlExpression,
      const FindBlobsByTagsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::FindBlobContainerBlobsByTagsOptions protocolLayerOptions;
    protocolLayerOptions.Where = tagFilterSqlExpression;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto response = _detail::BlobContainerClient::FindBlobsByTags(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));

    FindBlobsByTagsPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.TaggedBlobs = std::move(response.Value.Items);
    pagedResponse.m_blobContainerClient = std::make_shared<BlobContainerClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_tagFilterSqlExpression = tagFilterSqlExpression;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  BlobContainerBatch BlobContainerClient::CreateBatch() const { return BlobContainerBatch(*this); }

  Response<Models::SubmitBlobBatchResult> BlobContainerClient::SubmitBatch(
      const BlobContainerBatch& batch,
      const SubmitBlobBatchOptions& options,
      const Core::Context& context) const
  {
    (void)options;

    _detail::BlobContainerClient::SubmitBlobContainerBatchOptions protocolLayerOptions;
    _detail::StringBodyStream bodyStream(std::string{});
    auto response = _detail::BlobContainerClient::SubmitBatch(
        *m_batchRequestPipeline,
        m_blobContainerUrl,
        bodyStream,
        protocolLayerOptions,
        context.WithValue(_detail::s_containerBatchKey, &batch));
    return Azure::Response<Models::SubmitBlobBatchResult>(
        Models::SubmitBlobBatchResult(), std::move(response.RawResponse));
  }

  Azure::Response<Models::AccountInfo> BlobContainerClient::GetAccountInfo(
      const GetAccountInfoOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobContainerClient::GetBlobContainerAccountInfoOptions protocolLayerOptions;
    return _detail::BlobContainerClient::GetAccountInfo(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
  }
}}} // namespace Azure::Storage::Blobs
