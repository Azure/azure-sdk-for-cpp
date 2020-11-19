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

  std::string BlobSasBuilder::GenerateSasToken(const SharedKeyCredential& credential)
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
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n"
        + Storage::Details::defaultSasVersion + "\n" + resource + "\n" + snapshotVersion + "\n"
        + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage
        + "\n" + ContentType;

    std::string signature = Base64Encode(
        Storage::Details::HmacSha256(stringToSign, Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::Details::UrlEncodeQueryParameter(Storage::Details::defaultSasVersion));
    builder.AppendQueryParameter("spr", Storage::Details::UrlEncodeQueryParameter(protocol));
    if (StartsOn.HasValue())
    {
      builder.AppendQueryParameter(
          "st", Storage::Details::UrlEncodeQueryParameter(StartsOn.GetValue()));
    }
    if (!ExpiresOn.empty())
    {
      builder.AppendQueryParameter("se", Storage::Details::UrlEncodeQueryParameter(ExpiresOn));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::Details::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", Storage::Details::UrlEncodeQueryParameter(Identifier));
    }
    builder.AppendQueryParameter("sr", Storage::Details::UrlEncodeQueryParameter(resource));
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", Storage::Details::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter("sig", Storage::Details::UrlEncodeQueryParameter(signature));
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::Details::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::Details::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::Details::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::Details::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::Details::UrlEncodeQueryParameter(ContentType));
    }

    return builder.GetAbsoluteUrl();
  }

  std::string BlobSasBuilder::GenerateSasToken(
      const Models::UserDelegationKey& userDelegationKey,
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
        + userDelegationKey.SignedVersion + "\n\n\n\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n"
        + Storage::Details::defaultSasVersion + "\n" + resource + "\n" + snapshotVersion + "\n"
        + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage
        + "\n" + ContentType;

    std::string signature = Base64Encode(
        Storage::Details::HmacSha256(stringToSign, Base64Decode(userDelegationKey.Value)));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::Details::UrlEncodeQueryParameter(Storage::Details::defaultSasVersion));
    builder.AppendQueryParameter("sr", Storage::Details::UrlEncodeQueryParameter(resource));
    if (StartsOn.HasValue())
    {
      builder.AppendQueryParameter(
          "st", Storage::Details::UrlEncodeQueryParameter(StartsOn.GetValue()));
    }
    builder.AppendQueryParameter("se", Storage::Details::UrlEncodeQueryParameter(ExpiresOn));
    builder.AppendQueryParameter("sp", Storage::Details::UrlEncodeQueryParameter(Permissions));
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::Details::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    builder.AppendQueryParameter("spr", Storage::Details::UrlEncodeQueryParameter(protocol));
    builder.AppendQueryParameter(
        "skoid", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedObjectId));
    builder.AppendQueryParameter(
        "sktid", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedTenantId));
    builder.AppendQueryParameter(
        "skt", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedStartsOn));
    builder.AppendQueryParameter(
        "ske", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedExpiresOn));
    builder.AppendQueryParameter(
        "sks", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedService));
    builder.AppendQueryParameter(
        "skv", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedVersion));
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::Details::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::Details::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::Details::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::Details::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::Details::UrlEncodeQueryParameter(ContentType));
    }
    builder.AppendQueryParameter("sig", Storage::Details::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Blobs
