
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace Models {
    const PublicAccessType PublicAccessType::FileSystem("FileSystem");
    const PublicAccessType PublicAccessType::Path("Path");
    const PublicAccessType PublicAccessType::None("None");

    const PathResourceType PathResourceType::Directory("directory");
    const PathResourceType PathResourceType::File("file");

    const LeaseDuration LeaseDuration::Infinite("infinite");
    const LeaseDuration LeaseDuration::Fixed("fixed");

    const LeaseState LeaseState::Available("available");
    const LeaseState LeaseState::Leased("leased");
    const LeaseState LeaseState::Expired("expired");
    const LeaseState LeaseState::Breaking("breaking");
    const LeaseState LeaseState::Broken("broken");

    const LeaseStatus LeaseStatus::Locked("locked");
    const LeaseStatus LeaseStatus::Unlocked("unlocked");

  } // namespace Models
  namespace _detail {
    const PathRenameMode PathRenameMode::Legacy("legacy");
    const PathRenameMode PathRenameMode::Posix("posix");

    const PathGetPropertiesAction PathGetPropertiesAction::GetAccessControl("getAccessControl");
    const PathGetPropertiesAction PathGetPropertiesAction::GetStatus("getStatus");

    const FileSystemResource FileSystemResource::Filesystem("filesystem");

    const PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveMode::Set("set");
    const PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveMode::Modify("modify");
    const PathSetAccessControlRecursiveMode PathSetAccessControlRecursiveMode::Remove("remove");
  } // namespace _detail
}}}} // namespace Azure::Storage::Files::DataLake
