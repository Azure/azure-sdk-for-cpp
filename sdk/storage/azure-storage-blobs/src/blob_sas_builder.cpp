// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_sas_builder.hpp"

#include <azure/core/http/http.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Sas {

  namespace {
    std::string BlobSasResourceToString(BlobSasResource resource)
    {
      if (resource == BlobSasResource::BlobContainer)
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

  void BlobSasBuilder::SetPermissions(BlobContainerSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & BlobContainerSasPermissions::Read) == BlobContainerSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & BlobContainerSasPermissions::Add) == BlobContainerSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & BlobContainerSasPermissions::Create) == BlobContainerSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & BlobContainerSasPermissions::Write) == BlobContainerSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & BlobContainerSasPermissions::Delete) == BlobContainerSasPermissions::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & BlobContainerSasPermissions::DeleteVersion)
        == BlobContainerSasPermissions::DeleteVersion)
    {
      Permissions += "x";
    }
    if ((permissions & BlobContainerSasPermissions::List) == BlobContainerSasPermissions::List)
    {
      Permissions += "l";
    }
    if ((permissions & BlobContainerSasPermissions::Tags) == BlobContainerSasPermissions::Tags)
    {
      Permissions += "t";
    }
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

  std::string BlobSasBuilder::GenerateSasToken(const StorageSharedKeyCredential& credential)
  {
    std::string canonicalName = "/blob/" + credential.AccountName + "/" + BlobContainerName;
    if (Resource == BlobSasResource::Blob || Resource == BlobSasResource::BlobSnapshot
        || Resource == BlobSasResource::BlobVersion)
    {
      canonicalName += "/" + BlobName;
    }
    std::string protocol = Details::SasProtocolToString(Protocol);
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

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.GetValue().GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "")
        + "\n" + protocol + "\n" + Storage::Details::DefaultSasVersion + "\n" + resource + "\n"
        + snapshotVersion + "\n" + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding
        + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::Details::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::Details::UrlEncodeQueryParameter(Storage::Details::DefaultSasVersion));
    builder.AppendQueryParameter("spr", Storage::Details::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::Details::UrlEncodeQueryParameter(startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter("se", Storage::Details::UrlEncodeQueryParameter(expiresOnStr));
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
      const Blobs::Models::UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    std::string canonicalName = "/blob/" + accountName + "/" + BlobContainerName;
    if (Resource == BlobSasResource::Blob || Resource == BlobSasResource::BlobSnapshot
        || Resource == BlobSasResource::BlobVersion)
    {
      canonicalName += "/" + BlobName;
    }
    std::string protocol = Details::SasProtocolToString(Protocol);
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

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.GetValue().GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr
        = ExpiresOn.GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate);
    std::string signedStartsOnStr = userDelegationKey.SignedStartsOn.GetRfc3339String(
        Azure::Core::DateTime::TimeFractionFormat::Truncate);
    std::string signedExpiresOnStr = userDelegationKey.SignedExpiresOn.GetRfc3339String(
        Azure::Core::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + signedStartsOnStr + "\n" + signedExpiresOnStr
        + "\n" + userDelegationKey.SignedService + "\n" + userDelegationKey.SignedVersion
        + "\n\n\n\n" + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n"
        + Storage::Details::DefaultSasVersion + "\n" + resource + "\n" + snapshotVersion + "\n"
        + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage
        + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::Details::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(userDelegationKey.Value)));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::Details::UrlEncodeQueryParameter(Storage::Details::DefaultSasVersion));
    builder.AppendQueryParameter("sr", Storage::Details::UrlEncodeQueryParameter(resource));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::Details::UrlEncodeQueryParameter(startsOnStr));
    }
    builder.AppendQueryParameter("se", Storage::Details::UrlEncodeQueryParameter(expiresOnStr));
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
        "skt", Storage::Details::UrlEncodeQueryParameter(signedStartsOnStr));
    builder.AppendQueryParameter(
        "ske", Storage::Details::UrlEncodeQueryParameter(signedExpiresOnStr));
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

}}} // namespace Azure::Storage::Sas
