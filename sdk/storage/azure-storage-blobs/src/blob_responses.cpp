// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/blobs/blob_responses.hpp"

#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"
#include "azure/storage/blobs/blob_service_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  namespace Models {
    const EncryptionAlgorithmType EncryptionAlgorithmType::Aes256("AES256");
    const BlockType BlockType::Committed("Committed");
    const BlockType BlockType::Uncommitted("Uncommitted");
    const BlockType BlockType::Latest("Latest");
    const GeoReplicationStatus GeoReplicationStatus::Live("live");
    const GeoReplicationStatus GeoReplicationStatus::Bootstrap("bootstrap");
    const GeoReplicationStatus GeoReplicationStatus::Unavailable("unavailable");
    const LeaseStatus LeaseStatus::Locked("locked");
    const LeaseStatus LeaseStatus::Unlocked("unlocked");
    const LeaseState LeaseState::Available("available");
    const LeaseState LeaseState::Leased("leased");
    const LeaseState LeaseState::Expired("expired");
    const LeaseState LeaseState::Breaking("breaking");
    const LeaseState LeaseState::Broken("broken");
    const LeaseDurationType LeaseDurationType::Infinite("infinite");
    const LeaseDurationType LeaseDurationType::Fixed("fixed");
    const PublicAccessType PublicAccessType::BlobContainer("container");
    const PublicAccessType PublicAccessType::Blob("blob");
    const PublicAccessType PublicAccessType::None("");
    const SkuName SkuName::StandardLrs("Standard_LRS");
    const SkuName SkuName::StandardGrs("Standard_GRS");
    const SkuName SkuName::StandardRagrs("Standard_RAGRS");
    const SkuName SkuName::StandardZrs("Standard_ZRS");
    const SkuName SkuName::PremiumLrs("Premium_LRS");
    const SkuName SkuName::PremiumZrs("Premium_ZRS");
    const SkuName SkuName::StandardGzrs("Standard_GZRS");
    const SkuName SkuName::StandardRagzrs("Standard_RAGZRS");
    const AccountKind AccountKind::Storage("Storage");
    const AccountKind AccountKind::BlobStorage("BlobStorage");
    const AccountKind AccountKind::StorageV2("StorageV2");
    const AccountKind AccountKind::FileStorage("FileStorage");
    const AccountKind AccountKind::BlockBlobStorage("BlockBlobStorage");
    const CopyStatus CopyStatus::Pending("pending");
    const CopyStatus CopyStatus::Success("success");
    const CopyStatus CopyStatus::Aborted("aborted");
    const CopyStatus CopyStatus::Failed("failed");
    const AccessTier AccessTier::P1("p1");
    const AccessTier AccessTier::P2("p2");
    const AccessTier AccessTier::P3("p3");
    const AccessTier AccessTier::P4("p4");
    const AccessTier AccessTier::P6("p6");
    const AccessTier AccessTier::P10("p10");
    const AccessTier AccessTier::P15("p15");
    const AccessTier AccessTier::P20("p20");
    const AccessTier AccessTier::P30("p30");
    const AccessTier AccessTier::P40("p40");
    const AccessTier AccessTier::P50("p50");
    const AccessTier AccessTier::P60("p60");
    const AccessTier AccessTier::P70("p70");
    const AccessTier AccessTier::P80("p80");
    const AccessTier AccessTier::Hot("Hot");
    const AccessTier AccessTier::Cool("Cool");
    const AccessTier AccessTier::Archive("Archive");
    const AccessTier AccessTier::Premium("Premium");
    const AccessTier AccessTier::Cold("Cold");
    const AccessTier AccessTier::Smart("Smart");
    const ArchiveStatus ArchiveStatus::RehydratePendingToHot("rehydrate-pending-to-hot");
    const ArchiveStatus ArchiveStatus::RehydratePendingToCool("rehydrate-pending-to-cool");
    const ArchiveStatus ArchiveStatus::RehydratePendingToCold("rehydrate-pending-to-cold");
    const ArchiveStatus ArchiveStatus::RehydratePendingToSmart("rehydrate-pending-to-smart");
    const RehydratePriority RehydratePriority::High("High");
    const RehydratePriority RehydratePriority::Standard("Standard");
    const ObjectReplicationStatus ObjectReplicationStatus::Complete("complete");
    const ObjectReplicationStatus ObjectReplicationStatus::Failed("failed");
    const BlobImmutabilityPolicyMode BlobImmutabilityPolicyMode::Unlocked("unlocked");
    const BlobImmutabilityPolicyMode BlobImmutabilityPolicyMode::Locked("locked");
    const BlobType BlobType::BlockBlob("BlockBlob");
    const BlobType BlobType::PageBlob("PageBlob");
    const BlobType BlobType::AppendBlob("AppendBlob");
    const DeleteSnapshotsOption DeleteSnapshotsOption::IncludeSnapshots("include");
    const DeleteSnapshotsOption DeleteSnapshotsOption::OnlySnapshots("only");
    const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::NeverExpire("NeverExpire");
    const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::RelativeToCreation(
        "RelativeToCreation");
    const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::RelativeToNow("RelativeToNow");
    const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::Absolute("Absolute");
    const BlobCopySourceTagsMode BlobCopySourceTagsMode::Replace("REPLACE");
    const BlobCopySourceTagsMode BlobCopySourceTagsMode::Copy("COPY");
    const FileShareTokenIntent FileShareTokenIntent::Backup("backup");
    namespace _detail {
      const QueryRequestQueryType QueryRequestQueryType::SQL("SQL");
      const QueryFormatType QueryFormatType::Delimited("delimited");
      const QueryFormatType QueryFormatType::Json("json");
      const QueryFormatType QueryFormatType::Arrow("arrow");
      const QueryFormatType QueryFormatType::Parquet("parquet");
    } // namespace _detail
    const BlobQueryArrowFieldType BlobQueryArrowFieldType::Int64("int64");
    const BlobQueryArrowFieldType BlobQueryArrowFieldType::Bool("bool");
    const BlobQueryArrowFieldType BlobQueryArrowFieldType::Timestamp("timestamp[ms]");
    const BlobQueryArrowFieldType BlobQueryArrowFieldType::String("string");
    const BlobQueryArrowFieldType BlobQueryArrowFieldType::Double("double");
    const BlobQueryArrowFieldType BlobQueryArrowFieldType::Decimal("decimal");
    const SequenceNumberAction SequenceNumberAction::Max("max");
    const SequenceNumberAction SequenceNumberAction::Update("update");
    const SequenceNumberAction SequenceNumberAction::Increment("increment");
    const BlockListType BlockListType::Committed("committed");
    const BlockListType BlockListType::Uncommitted("uncommitted");
    const BlockListType BlockListType::All("all");
  } // namespace Models
  std::unique_ptr<Azure::Core::Http::RawResponse> StartBlobCopyOperation::PollInternal(
      const Azure::Core::Context&)
  {

    auto response = m_blobClient->GetProperties();
    if (!response.Value.CopyStatus.HasValue())
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else if (response.Value.CopyStatus.Value() == Models::CopyStatus::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response.Value.CopyStatus.Value() == Models::CopyStatus::Success)
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    else
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    m_pollResult = response.Value;
    return std::move(response.RawResponse);
  }

  Azure::Response<Models::BlobProperties> StartBlobCopyOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      auto rawResponse = Poll(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Response<Models::BlobProperties>(
            m_pollResult, std::make_unique<Azure::Core::Http::RawResponse>(rawResponse));
      }
      else if (m_status == Azure::Core::OperationStatus::Failed)
      {
        throw Azure::Core::RequestFailedException("Operation failed.");
      }
      else if (m_status == Azure::Core::OperationStatus::Cancelled)
      {
        throw Azure::Core::RequestFailedException("Operation was cancelled.");
      }

      std::this_thread::sleep_for(period);
    }
  }

  void ListBlobContainersPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_blobServiceClient->ListBlobContainers(m_operationOptions, context);
  }

  void FindBlobsByTagsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    if (m_blobServiceClient)
    {
      *this = m_blobServiceClient->FindBlobsByTags(
          m_tagFilterSqlExpression, m_operationOptions, context);
    }
    else if (m_blobContainerClient)
    {
      *this = m_blobContainerClient->FindBlobsByTags(
          m_tagFilterSqlExpression, m_operationOptions, context);
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  void ListBlobsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_blobContainerClient->ListBlobs(m_operationOptions, context);
  }

  void ListBlobsByHierarchyPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_blobContainerClient->ListBlobsByHierarchy(m_delimiter, m_operationOptions, context);
  }

  void GetPageRangesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_pageBlobClient->GetPageRanges(m_operationOptions, context);
  }

  void GetPageRangesDiffPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    if (m_previousSnapshot.HasValue())
    {
      *this = m_pageBlobClient->GetPageRangesDiff(
          m_previousSnapshot.Value(), m_operationOptions, context);
    }
    else if (m_previousSnapshotUrl.HasValue())
    {
      *this = m_pageBlobClient->GetManagedDiskPageRangesDiff(
          m_previousSnapshotUrl.Value(), m_operationOptions, context);
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

}}} // namespace Azure::Storage::Blobs
