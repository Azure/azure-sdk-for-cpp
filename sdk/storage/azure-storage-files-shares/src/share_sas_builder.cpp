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
        throw std::invalid_argument("unknown ShareSasResource value");
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

    std::string startsOnStr = StartsOn.HasValue() ? StartsOn.GetValue().ToString(
                                  Azure::Core::DateTime::DateFormat::Rfc3339,
                                  Azure::Core::DateTime::TimeFractionFormat::Truncate)
                                                  : "";
    std::string expiresOnStr = Identifier.empty() ? ExpiresOn.ToString(
                                   Azure::Core::DateTime::DateFormat::Rfc3339,
                                   Azure::Core::DateTime::TimeFractionFormat::Truncate)
                                                  : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "")
        + "\n" + protocol + "\n" + Storage::_detail::DefaultSasVersion + "\n" + CacheControl + "\n"
        + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::_detail::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::_detail::UrlEncodeQueryParameter(Storage::_detail::DefaultSasVersion));
    builder.AppendQueryParameter("spr", Storage::_detail::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::_detail::UrlEncodeQueryParameter(startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter("se", Storage::_detail::UrlEncodeQueryParameter(expiresOnStr));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::_detail::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", Storage::_detail::UrlEncodeQueryParameter(Identifier));
    }
    builder.AppendQueryParameter("sr", Storage::_detail::UrlEncodeQueryParameter(resource));
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", Storage::_detail::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter("sig", Storage::_detail::UrlEncodeQueryParameter(signature));
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::_detail::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::_detail::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::_detail::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::_detail::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::_detail::UrlEncodeQueryParameter(ContentType));
    }

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Sas
