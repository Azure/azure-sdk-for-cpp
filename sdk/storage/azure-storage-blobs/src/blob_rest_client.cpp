// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  const AccountKind AccountKind::Storage("Storage");
  const AccountKind AccountKind::BlobStorage("BlobStorage");
  const AccountKind AccountKind::StorageV2("StorageV2");
  const AccountKind AccountKind::FileStorage("FileStorage");
  const AccountKind AccountKind::BlockBlobStorage("BlockBlobStorage");

  const CopyStatus CopyStatus::Success("success");
  const CopyStatus CopyStatus::Pending("pending");

  const GeoReplicationStatus GeoReplicationStatus::Live("live");
  const GeoReplicationStatus GeoReplicationStatus::Bootstrap("bootstrap");
  const GeoReplicationStatus GeoReplicationStatus::Unavailable("unavailable");

  const LeaseDurationType LeaseDurationType::Infinite("infinite");
  const LeaseDurationType LeaseDurationType::Fixed("fixed");

  const LeaseState LeaseState::Available("available");
  const LeaseState LeaseState::Leased("leased");
  const LeaseState LeaseState::Expired("expired");
  const LeaseState LeaseState::Breaking("breaking");
  const LeaseState LeaseState::Broken("broken");

  const LeaseStatus LeaseStatus::Locked("locked");
  const LeaseStatus LeaseStatus::Unlocked("unlocked");

  const ObjectReplicationStatus ObjectReplicationStatus::Complete("complete");
  const ObjectReplicationStatus ObjectReplicationStatus::Failed("failed");

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

  const AccessTier AccessTier::P1("P1");
  const AccessTier AccessTier::P2("P2");
  const AccessTier AccessTier::P3("P3");
  const AccessTier AccessTier::P4("P4");
  const AccessTier AccessTier::P6("P6");
  const AccessTier AccessTier::P10("P10");
  const AccessTier AccessTier::P15("P15");
  const AccessTier AccessTier::P20("P20");
  const AccessTier AccessTier::P30("P30");
  const AccessTier AccessTier::P40("P40");
  const AccessTier AccessTier::P50("P50");
  const AccessTier AccessTier::P60("P60");
  const AccessTier AccessTier::P70("P70");
  const AccessTier AccessTier::P80("P80");
  const AccessTier AccessTier::Hot("Hot");
  const AccessTier AccessTier::Cool("Cool");
  const AccessTier AccessTier::Archive("Archive");

  const ArchiveStatus ArchiveStatus::RehydratePendingToHot("rehydrate-pending-to-hot");
  const ArchiveStatus ArchiveStatus::RehydratePendingToCool("rehydrate-pending-to-cool");

  const BlobType BlobType::BlockBlob("BlockBlob");
  const BlobType BlobType::PageBlob("PageBlob");
  const BlobType BlobType::AppendBlob("AppendBlob");

  const RehydratePriority RehydratePriority::High("High");
  const RehydratePriority RehydratePriority::Standard("Standard");

  const BlockListType BlockListType::Committed("committed");
  const BlockListType BlockListType::Uncommitted("uncommitted");
  const BlockListType BlockListType::All("all");

  const BlockType BlockType::Committed("Committed");
  const BlockType BlockType::Uncommitted("Uncommitted");
  const BlockType BlockType::Latest("Latest");

  const DeleteSnapshotsOption DeleteSnapshotsOption::IncludeSnapshots("include");
  const DeleteSnapshotsOption DeleteSnapshotsOption::OnlySnapshots("only");

  const EncryptionAlgorithmType EncryptionAlgorithmType::Aes256("AES256");

  const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::NeverExpire("NeverExpire");
  const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::RelativeToCreation(
      "RelativeToCreation");
  const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::RelativeToNow("RelativeToNow");
  const ScheduleBlobExpiryOriginType ScheduleBlobExpiryOriginType::Absolute("Absolute");

  const SequenceNumberAction SequenceNumberAction::Max("max");
  const SequenceNumberAction SequenceNumberAction::Update("update");
  const SequenceNumberAction SequenceNumberAction::Increment("increment");

}}}} // namespace Azure::Storage::Blobs::Models
