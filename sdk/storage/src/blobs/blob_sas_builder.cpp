// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_sas_builder.hpp"
#include "common/crypt.hpp"
#include "common/storage_uri_builder.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  void BlobSasBuilder::SetPermissions(BlobContainerSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & BlobContainerSasPermissions::Read) != BlobContainerSasPermissions::None)
    {
      Permissions += "r";
    }
    if ((permissions & BlobContainerSasPermissions::Add) != BlobContainerSasPermissions::None)
    {
      Permissions += "a";
    }
    if ((permissions & BlobContainerSasPermissions::Create) != BlobContainerSasPermissions::None)
    {
      Permissions += "c";
    }
    if ((permissions & BlobContainerSasPermissions::Write) != BlobContainerSasPermissions::None)
    {
      Permissions += "w";
    }
    if ((permissions & BlobContainerSasPermissions::Delete) != BlobContainerSasPermissions::None)
    {
      Permissions += "d";
    }
    if ((permissions & BlobContainerSasPermissions::DeleteVersion)
        != BlobContainerSasPermissions::None)
    {
      Permissions += "x";
    }
    if ((permissions & BlobContainerSasPermissions::List) != BlobContainerSasPermissions::None)
    {
      Permissions += "l";
    }
    if ((permissions & BlobContainerSasPermissions::Tags) != BlobContainerSasPermissions::None)
    {
      Permissions += "t";
    }
  }

  void BlobSasBuilder::SetPermissions(BlobSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & BlobSasPermissions::Read) != BlobSasPermissions::None)
    {
      Permissions += "r";
    }
    if ((permissions & BlobSasPermissions::Add) != BlobSasPermissions::None)
    {
      Permissions += "a";
    }
    if ((permissions & BlobSasPermissions::Create) != BlobSasPermissions::None)
    {
      Permissions += "c";
    }
    if ((permissions & BlobSasPermissions::Write) != BlobSasPermissions::None)
    {
      Permissions += "w";
    }
    if ((permissions & BlobSasPermissions::Delete) != BlobSasPermissions::None)
    {
      Permissions += "d";
    }
    if ((permissions & BlobSasPermissions::DeleteVersion) != BlobSasPermissions::None)
    {
      Permissions += "x";
    }
    if ((permissions & BlobSasPermissions::Tags) != BlobSasPermissions::None)
    {
      Permissions += "t";
    }
  }

  std::string BlobSasBuilder::ToSasQueryParameters(const SharedKeyCredential& credential)
  {
    std::string canonicalName = "/blob/" + credential.AccountName + "/" + ContainerName;
    if (Resource == BlobSasResource::Blob || Resource == BlobSasResource::BlobSnapshot)
    {
      canonicalName += "/" + BlobName;
      // TODO: what if BlobName contains slash or backslash
    }
    std::string protocol;
    if (Protocol == SasProtocol::HttpsAndHtttp)
    {
      protocol = "https,http";
    }
    else
    {
      protocol = "https";
    }
    std::string resource;
    if (Resource == BlobSasResource::Container)
    {
      resource = "c";
    }
    else if (Resource == BlobSasResource::Blob)
    {
      resource = "b";
    }
    else if (Resource == BlobSasResource::BlobSnapshot)
    {
      resource = "bs";
    }
    else if (Resource == BlobSasResource::BlobVersion)
    {
      resource = "bv";
    }
    std::string stringToSign = Permissions + "\n" + StartsOn + "\n" + ExpiresOn + "\n"
        + canonicalName + "\n" + Identifier + "\n" + IPRange + "\n" + protocol + "\n" + Version
        + "\n" + resource + "\n" + Snapshot + "\n" + CacheControl + "\n" + ContentDisposition + "\n"
        + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature
        = Base64Encode(HMAC_SHA256(stringToSign, Base64Decode(credential.GetAccountKey())));

    UriBuilder builder;
    builder.AppendQuery("sv", Version);
    builder.AppendQuery("spr", protocol);
    builder.AppendQuery("st", StartsOn);
    builder.AppendQuery("se", ExpiresOn);
    if (!IPRange.empty())
    {
      builder.AppendQuery("sip", IPRange);
    }
    if (!Identifier.empty())
    {
      builder.AppendQuery("si", Identifier);
    }
    builder.AppendQuery("sr", resource);
    builder.AppendQuery("sp", Permissions);
    builder.AppendQuery("sig", signature, true);
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

    return builder.ToString();
  }

  std::string BlobSasBuilder::ToSasQueryParameters(
      const UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    unused(userDelegationKey, accountName);
    return std::string();
  }

}}} // namespace Azure::Storage::Blobs
