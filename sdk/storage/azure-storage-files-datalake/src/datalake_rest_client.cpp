
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Models {
  const FileSystemResourceType FileSystemResourceType::Filesystem("filesystem");

  const PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveMode::Set("set");
  const PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveMode::Modify("modify");
  const PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveMode::Remove("remove");

  const PathExpiryOptions PathExpiryOptions::NeverExpire("NeverExpire");
  const PathExpiryOptions PathExpiryOptions::RelativeToCreation("RelativeToCreation");
  const PathExpiryOptions PathExpiryOptions::RelativeToNow("RelativeToNow");
  const PathExpiryOptions PathExpiryOptions::Absolute("Absolute");

  const AccountResourceType AccountResourceType::Account("account");

  const PathResourceType PathResourceType::Directory("directory");
  const PathResourceType PathResourceType::File("file");

  const PathRenameMode PathRenameMode::Legacy("legacy");
  const PathRenameMode PathRenameMode::Posix("posix");

  const PathLeaseAction PathLeaseAction::Acquire("acquire");
  const PathLeaseAction PathLeaseAction::Break("break");
  const PathLeaseAction PathLeaseAction::Change("change");
  const PathLeaseAction PathLeaseAction::Renew("renew");
  const PathLeaseAction PathLeaseAction::Release("release");

  const PathGetPropertiesAction PathGetPropertiesAction::GetAccessControl("getAccessControl");
  const PathGetPropertiesAction PathGetPropertiesAction::GetStatus("getStatus");

  const LeaseStateType LeaseStateType::Available("available");
  const LeaseStateType LeaseStateType::Leased("leased");
  const LeaseStateType LeaseStateType::Expired("expired");
  const LeaseStateType LeaseStateType::Breaking("breaking");
  const LeaseStateType LeaseStateType::Broken("broken");

  const LeaseStatusType LeaseStatusType::Locked("locked");
  const LeaseStatusType LeaseStatusType::Unlocked("unlocked");

}}}}} // namespace Azure::Storage::Files::DataLake::Models
