// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_sas_builder.hpp"

#include <azure/core/http/http.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Sas {

  namespace {
    std::string ShareSasResourceToString(ShareSasResource resource)
    {
      if (resource == ShareSasResource::Share)
      {
        return "s";
      }
      else if (resource == ShareSasResource::File)
      {
        return "f";
      }
      else
      {
        throw std::runtime_error("unknown ShareSasResource value");
      }
    }
  } // namespace

  void ShareSasBuilder::SetPermissions(ShareSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & ShareSasPermissions::Read) == ShareSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & ShareSasPermissions::Create) == ShareSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & ShareSasPermissions::Write) == ShareSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & ShareSasPermissions::Delete) == ShareSasPermissions::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & ShareSasPermissions::List) == ShareSasPermissions::List)
    {
      Permissions += "l";
    }
  }

  void ShareSasBuilder::SetPermissions(ShareFileSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & ShareFileSasPermissions::Read) == ShareFileSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & ShareFileSasPermissions::Create) == ShareFileSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & ShareFileSasPermissions::Write) == ShareFileSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & ShareFileSasPermissions::Delete) == ShareFileSasPermissions::Delete)
    {
      Permissions += "d";
    }
  }

  std::string ShareSasBuilder::GenerateSasToken(const StorageSharedKeyCredential& credential)
  {
    std::string canonicalName = "/file/" + credential.AccountName + "/" + ShareName;
    if (Resource == ShareSasResource::File)
    {
      canonicalName += "/" + FilePath;
    }
    std::string protocol = Details::SasProtocolToString(Protocol);
    std::string resource = ShareSasResourceToString(Resource);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.GetValue().GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "")
        + "\n" + protocol + "\n" + Storage::Details::DefaultSasVersion + "\n" + CacheControl + "\n"
        + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

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

}}} // namespace Azure::Storage::Sas
