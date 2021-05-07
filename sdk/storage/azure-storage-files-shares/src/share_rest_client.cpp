
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  namespace Models {
    const AccessTier AccessTier::TransactionOptimized("TransactionOptimized");
    const AccessTier AccessTier::Hot("Hot");
    const AccessTier AccessTier::Cool("Cool");
    const AccessTier AccessTier::Premium("Premium");

    const PermissionCopyMode PermissionCopyMode::Source("source");
    const PermissionCopyMode PermissionCopyMode::Override("override");

    const DeleteSnapshotsOption DeleteSnapshotsOption::Include("include");

    const LeaseDuration LeaseDuration::Infinite("infinite");
    const LeaseDuration LeaseDuration::Fixed("fixed");

    const LeaseState LeaseState::Available("available");
    const LeaseState LeaseState::Leased("leased");
    const LeaseState LeaseState::Expired("expired");
    const LeaseState LeaseState::Breaking("breaking");
    const LeaseState LeaseState::Broken("broken");

    const LeaseStatus LeaseStatus::Locked("locked");
    const LeaseStatus LeaseStatus::Unlocked("unlocked");

    const CopyStatus CopyStatus::Pending("pending");
    const CopyStatus CopyStatus::Success("success");
    const CopyStatus CopyStatus::Aborted("aborted");
    const CopyStatus CopyStatus::Failed("failed");

  } // namespace Models
  namespace _detail {
    const FileRangeWrite FileRangeWrite::Update("update");
    const FileRangeWrite FileRangeWrite::Clear("clear");

    const FileRangeWriteFromUrl FileRangeWriteFromUrl::Update("update");

    const LeaseAction LeaseAction::Acquire("acquire");
    const LeaseAction LeaseAction::Release("release");
    const LeaseAction LeaseAction::Change("change");
    const LeaseAction LeaseAction::Renew("renew");
    const LeaseAction LeaseAction::Break("break");

  } // namespace _detail
}}}} // namespace Azure::Storage::Files::Shares
