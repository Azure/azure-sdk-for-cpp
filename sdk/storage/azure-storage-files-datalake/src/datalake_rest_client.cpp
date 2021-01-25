
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

  const PublicAccessType PublicAccessType::FileSystem("FileSystem");
  const PublicAccessType PublicAccessType::Path("Path");
  const PublicAccessType PublicAccessType::None("None");

  const PathResourceType PathResourceType::Directory("directory");
  const PathResourceType PathResourceType::File("file");

  const PathRenameMode PathRenameMode::Legacy("legacy");
  const PathRenameMode PathRenameMode::Posix("posix");

  const PathGetPropertiesAction PathGetPropertiesAction::GetAccessControl("getAccessControl");
  const PathGetPropertiesAction PathGetPropertiesAction::GetStatus("getStatus");

  const LeaseDurationType LeaseDurationType::Infinite("infinite");
  const LeaseDurationType LeaseDurationType::Fixed("fixed");

  const LeaseStateType LeaseStateType::Available("available");
  const LeaseStateType LeaseStateType::Leased("leased");
  const LeaseStateType LeaseStateType::Expired("expired");
  const LeaseStateType LeaseStateType::Breaking("breaking");
  const LeaseStateType LeaseStateType::Broken("broken");

  const LeaseStatusType LeaseStatusType::Locked("locked");
  const LeaseStatusType LeaseStatusType::Unlocked("unlocked");

}}}}} // namespace Azure::Storage::Files::DataLake::Models
