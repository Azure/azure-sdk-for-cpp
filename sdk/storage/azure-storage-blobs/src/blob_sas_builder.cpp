// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_sas_builder.hpp"
#include "azure/core/http/http.hpp"
#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  namespace {
    std::string BlobSasResourceToString(BlobSasResource resource)
    {
      if (resource == BlobSasResource::Container)
      {
        return "c";
      }
      else if (resource == BlobSasResource::Blob)
      {
        return "b";
      }
      else if (resource == BlobSasResource::BlobSnapshot)
      {
        return "bs";
      }
      else if (resource == BlobSasResource::BlobVersion)
      {
        return "bv";
      }
      else
      {
        throw std::runtime_error("unknown BlobSasResource value");
      }
    }
  } // namespace

  std::string BlobContainerSasPermissionsToString(BlobContainerSasPermissions permissions)
  {
    std::string permissions_str;
    // The order matters
    if ((permissions & BlobContainerSasPermissions::Read) == BlobContainerSasPermissions::Read)
    {
      permissions_str += "r";
    }
    if ((permissions & BlobContainerSasPermissions::Add) == BlobContainerSasPermissions::Add)
    {
      permissions_str += "a";
    }
    if ((permissions & BlobContainerSasPermissions::Create) == BlobContainerSasPermissions::Create)
    {
      permissions_str += "c";
    }
    if ((permissions & BlobContainerSasPermissions::Write) == BlobContainerSasPermissions::Write)
    {
      permissions_str += "w";
    }
    if ((permissions & BlobContainerSasPermissions::Delete) == BlobContainerSasPermissions::Delete)
    {
      permissions_str += "d";
    }
    if ((permissions & BlobContainerSasPermissions::DeleteVersion)
        == BlobContainerSasPermissions::DeleteVersion)
    {
      permissions_str += "x";
    }
    if ((permissions & BlobContainerSasPermissions::List) == BlobContainerSasPermissions::List)
    {
      permissions_str += "l";
    }
    if ((permissions & BlobContainerSasPermissions::Tags) == BlobContainerSasPermissions::Tags)
    {
      permissions_str += "t";
    }
    return permissions_str;
  }

  void BlobSasBuilder::SetPermissions(BlobSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & BlobSasPermissions::Read) == BlobSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & BlobSasPermissions::Add) == BlobSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & BlobSasPermissions::Create) == BlobSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & BlobSasPermissions::Write) == BlobSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & BlobSasPermissions::Delete) == BlobSasPermissions::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & BlobSasPermissions::DeleteVersion) == BlobSasPermissions::DeleteVersion)
    {
      Permissions += "x";
    }
    if ((permissions & BlobSasPermissions::Tags) == BlobSasPermissions::Tags)
    {
      Permissions += "t";
    }
  }

  std::string BlobSasBuilder::ToSasQueryParameters(const SharedKeyCredential& credential)
  {
    std::string canonicalName = "/blob/" + credential.AccountName + "/" + ContainerName;
    if (Resource == BlobSasResource::Blob || Resource == BlobSasResource::BlobSnapshot
        || Resource == BlobSasResource::BlobVersion)
    {
      canonicalName += "/" + BlobName;
    }
    std::string protocol = SasProtocolToString(Protocol);
    std::string resource = BlobSasResourceToString(Resource);

    std::string snapshotVersion;
    if (Resource == BlobSasResource::BlobSnapshot)
    {
      snapshotVersion = Snapshot;
    }
    else if (Resource == BlobSasResource::BlobVersion)
    {
      snapshotVersion = BlobVersionId;
    }

    std::string stringToSign = Permissions + "\n" + (StartsOn.HasValue() ? StartsOn.GetValue() : "")
        + "\n" + ExpiresOn + "\n" + canonicalName + "\n" + Identifier + "\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n" + Version + "\n"
        + resource + "\n" + snapshotVersion + "\n" + CacheControl + "\n" + ContentDisposition + "\n"
        + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature
        = Base64Encode(Details::HmacSha256(stringToSign, Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQuery("sv", Version);
    builder.AppendQuery("spr", protocol);
    if (StartsOn.HasValue())
    {
      builder.AppendQuery("st", StartsOn.GetValue());
    }
    if (!ExpiresOn.empty())
    {
      builder.AppendQuery("se", ExpiresOn);
    }
    if (IPRange.HasValue())
    {
      builder.AppendQuery("sip", IPRange.GetValue());
    }
    if (!Identifier.empty())
    {
      builder.AppendQuery("si", Identifier);
    }
    builder.AppendQuery("sr", resource);
    if (!Permissions.empty())
    {
      builder.AppendQuery("sp", Permissions);
    }
    builder.AppendQuery("sig", signature);
    if (!CacheControl.empty())
    {
      builder.AppendQuery("rscc", CacheControl);
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQuery("rscd", ContentDisposition);
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQuery("rsce", ContentEncoding);
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQuery("rscl", ContentLanguage);
    }
    if (!ContentType.empty())
    {
      builder.AppendQuery("rsct", ContentType);
    }

    return builder.GetAbsoluteUrl();
  }

  std::string BlobSasBuilder::ToSasQueryParameters(
      const UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    std::string canonicalName = "/blob/" + accountName + "/" + ContainerName;
    if (Resource == BlobSasResource::Blob || Resource == BlobSasResource::BlobSnapshot
        || Resource == BlobSasResource::BlobVersion)
    {
      canonicalName += "/" + BlobName;
    }
    std::string protocol = SasProtocolToString(Protocol);
    std::string resource = BlobSasResourceToString(Resource);

    std::string snapshotVersion;
    if (Resource == BlobSasResource::BlobSnapshot)
    {
      snapshotVersion = Snapshot;
    }
    else if (Resource == BlobSasResource::BlobVersion)
    {
      snapshotVersion = BlobVersionId;
    }

    std::string stringToSign = Permissions + "\n" + (StartsOn.HasValue() ? StartsOn.GetValue() : "")
        + "\n" + ExpiresOn + "\n" + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + userDelegationKey.SignedStartsOn + "\n"
        + userDelegationKey.SignedExpiresOn + "\n" + userDelegationKey.SignedService + "\n"
        + userDelegationKey.SignedVersion + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "")
        + "\n" + protocol + "\n" + Version + "\n" + resource + "\n" + snapshotVersion + "\n"
        + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage
        + "\n" + ContentType;

    std::string signature
        = Base64Encode(Details::HmacSha256(stringToSign, Base64Decode(userDelegationKey.Value)));

    Azure::Core::Http::Url builder;
    builder.AppendQuery("sv", Version);
    builder.AppendQuery("sr", resource);
    if (StartsOn.HasValue())
    {
      builder.AppendQuery("st", StartsOn.GetValue());
    }
    builder.AppendQuery("se", ExpiresOn);
    builder.AppendQuery("sp", Permissions);
    if (IPRange.HasValue())
    {
      builder.AppendQuery("sip", IPRange.GetValue());
    }
    builder.AppendQuery("spr", protocol);
    builder.AppendQuery("skoid", userDelegationKey.SignedObjectId);
    builder.AppendQuery("sktid", userDelegationKey.SignedTenantId);
    builder.AppendQuery("skt", userDelegationKey.SignedStartsOn);
    builder.AppendQuery("ske", userDelegationKey.SignedExpiresOn);
    builder.AppendQuery("sks", userDelegationKey.SignedService);
    builder.AppendQuery("skv", userDelegationKey.SignedVersion);
    if (!CacheControl.empty())
    {
      builder.AppendQuery("rscc", CacheControl);
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQuery("rscd", ContentDisposition);
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQuery("rsce", ContentEncoding);
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQuery("rscl", ContentLanguage);
    }
    if (!ContentType.empty())
    {
      builder.AppendQuery("rsct", ContentType);
    }
    builder.AppendQuery("sig", signature);

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Blobs
