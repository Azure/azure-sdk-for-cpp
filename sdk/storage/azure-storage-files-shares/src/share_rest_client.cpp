
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  namespace Models {
    const AccessTier AccessTier::TransactionOptimized("TransactionOptimized");
    const AccessTier AccessTier::Hot("Hot");
    const AccessTier AccessTier::Cool("Cool");
    const AccessTier AccessTier::Premium("Premium");

    const PermissionCopyModeType PermissionCopyModeType::Source("source");
    const PermissionCopyModeType PermissionCopyModeType::Override("override");

    const DeleteSnapshotsOptionType DeleteSnapshotsOptionType::Include("include");

    const FileRangeWriteFromUrlType FileRangeWriteFromUrlType::Update("update");

    const LeaseDurationType LeaseDurationType::Infinite("infinite");
    const LeaseDurationType LeaseDurationType::Fixed("fixed");

    const LeaseStateType LeaseStateType::Available("available");
    const LeaseStateType LeaseStateType::Leased("leased");
    const LeaseStateType LeaseStateType::Expired("expired");
    const LeaseStateType LeaseStateType::Breaking("breaking");
    const LeaseStateType LeaseStateType::Broken("broken");

    const LeaseStatusType LeaseStatusType::Locked("locked");
    const LeaseStatusType LeaseStatusType::Unlocked("unlocked");

    const LeaseAction LeaseAction::Acquire("acquire");
    const LeaseAction LeaseAction::Release("release");
    const LeaseAction LeaseAction::Change("change");
    const LeaseAction LeaseAction::Renew("renew");
    const LeaseAction LeaseAction::Break("break");

    const CopyStatus CopyStatus::Pending("pending");
    const CopyStatus CopyStatus::Success("success");
    const CopyStatus CopyStatus::Aborted("aborted");
    const CopyStatus CopyStatus::Failed("failed");

  } // namespace Models
  namespace _detail {
    const FileRangeWriteType FileRangeWriteType::Update("update");
    const FileRangeWriteType FileRangeWriteType::Clear("clear");
  } // namespace _detail
}}}} // namespace Azure::Storage::Files::Shares
