// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "blobs/protocol/blob_rest_client.hpp"
#include "common/account_sas_builder.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  enum class BlobSasResource
  {
    Container,
    Blob,
    BlobSnapshot,
    BlobVersion,
  };

  enum class BlobContainerSasPermissions
  {
    None = 0,
    Read = 1,
    Write = 2,
    Delete = 4,
    List = 8,
    Add = 16,
    Create = 32,
    Tags = 64,
    DeleteVersion = 128,
    All = ~0,
  };

  inline BlobContainerSasPermissions operator|(
      BlobContainerSasPermissions lhs,
      BlobContainerSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobContainerSasPermissions>;
    return static_cast<BlobContainerSasPermissions>(
        static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline BlobContainerSasPermissions operator&(
      BlobContainerSasPermissions lhs,
      BlobContainerSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobContainerSasPermissions>;
    return static_cast<BlobContainerSasPermissions>(
        static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  enum class BlobSasPermissions
  {
    None = 0,
    Read = 1,
    Write = 2,
    Delete = 4,
    Add = 8,
    Create = 16,
    Tags = 32,
    DeleteVersion = 64,
    All = ~0,
  };

  inline BlobSasPermissions operator|(BlobSasPermissions lhs, BlobSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobSasPermissions>;
    return static_cast<BlobSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline BlobSasPermissions operator&(BlobSasPermissions lhs, BlobSasPermissions rhs)
  {
    using type = std::underlying_type_t<BlobSasPermissions>;
    return static_cast<BlobSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  struct BlobSasBuilder
  {
    std::string Version = c_APIVersion;
    SasProtocol Protocol;
    std::string StartsOn;
    std::string ExpiresOn;
    std::string IPRange;
    std::string Identifier;
    std::string ContainerName;
    std::string BlobName;
    std::string Snapshot;
    BlobSasResource Resource;

    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentType;

    void SetPermissions(BlobContainerSasPermissions permissions);
    void SetPermissions(BlobSasPermissions permissions);

    std::string ToSasQueryParameters(const SharedKeyCredential& credential);
    std::string ToSasQueryParameters(
        const UserDelegationKey& userDelegationKey,
        const std::string& accountName);

  private:
    std::string Permissions;
  };

}}} // namespace Azure::Storage::Blobs
