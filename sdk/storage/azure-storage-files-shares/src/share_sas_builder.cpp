// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_sas_builder.hpp"
#include "azure/core/http/http.hpp"
#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

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

  std::string ShareSasPermissionsToString(ShareSasPermissions permissions)
  {
    std::string permissions_str;
    // The order matters
    if ((permissions & ShareSasPermissions::Read) == ShareSasPermissions::Read)
    {
      permissions_str += "r";
    }
    if ((permissions & ShareSasPermissions::Create) == ShareSasPermissions::Create)
    {
      permissions_str += "c";
    }
    if ((permissions & ShareSasPermissions::Write) == ShareSasPermissions::Write)
    {
      permissions_str += "w";
    }
    if ((permissions & ShareSasPermissions::Delete) == ShareSasPermissions::Delete)
    {
      permissions_str += "d";
    }
    if ((permissions & ShareSasPermissions::List) == ShareSasPermissions::List)
    {
      permissions_str += "l";
    }
    return permissions_str;
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

  std::string ShareSasBuilder::ToSasQueryParameters(const SharedKeyCredential& credential)
  {
    std::string canonicalName = "/file/" + credential.AccountName + "/" + ShareName;
    if (Resource == ShareSasResource::File)
    {
      canonicalName += "/" + FilePath;
    }
    std::string protocol = SasProtocolToString(Protocol);
    std::string resource = ShareSasResourceToString(Resource);

    std::string stringToSign = Permissions + "\n" + (StartsOn.HasValue() ? StartsOn.GetValue() : "")
        + "\n" + ExpiresOn + "\n" + canonicalName + "\n" + Identifier + "\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n" + Version + "\n"
        + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage
        + "\n" + ContentType;

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

}}}} // namespace Azure::Storage::Files::Shares
