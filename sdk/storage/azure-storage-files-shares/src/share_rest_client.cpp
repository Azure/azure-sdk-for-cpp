
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {
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

  const CopyStatusType CopyStatusType::Pending("pending");
  const CopyStatusType CopyStatusType::Success("success");
  const CopyStatusType CopyStatusType::Aborted("aborted");
  const CopyStatusType CopyStatusType::Failed("failed");

  const FileRangeWriteType FileRangeWriteType::Update("update");
  const FileRangeWriteType FileRangeWriteType::Clear("clear");

}}}}} // namespace Azure::Storage::Files::Shares::Models
