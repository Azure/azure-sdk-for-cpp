// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/account_sas_builder.hpp"

#include <azure/core/http/http.hpp>

#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage { namespace Sas {

  void AccountSasBuilder::SetPermissions(AccountSasPermissions permissions)
  {
    Permissions.clear();
    if ((permissions & AccountSasPermissions::Read) == AccountSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & AccountSasPermissions::Write) == AccountSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & AccountSasPermissions::Delete) == AccountSasPermissions::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & AccountSasPermissions::DeleteVersion)
        == AccountSasPermissions::DeleteVersion)
    {
      Permissions += "x";
    }
    if ((permissions & AccountSasPermissions::List) == AccountSasPermissions::List)
    {
      Permissions += "l";
    }
    if ((permissions & AccountSasPermissions::Add) == AccountSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & AccountSasPermissions::Create) == AccountSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & AccountSasPermissions::Update) == AccountSasPermissions::Update)
    {
      Permissions += "u";
    }
    if ((permissions & AccountSasPermissions::Process) == AccountSasPermissions::Process)
    {
      Permissions += "p";
    }
    if ((permissions & AccountSasPermissions::Tags) == AccountSasPermissions::Tags)
    {
      Permissions += "t";
    }
    if ((permissions & AccountSasPermissions::Filter) == AccountSasPermissions::Filter)
    {
      Permissions += "f";
    }
  }

  std::string AccountSasBuilder::GenerateSasToken(const StorageSharedKeyCredential& credential)
  {
    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string services;
    if ((Services & AccountSasServices::Blobs) == AccountSasServices::Blobs)
    {
      services += "b";
    }
    if ((Services & AccountSasServices::Queue) == AccountSasServices::Queue)
    {
      services += "q";
    }
    if ((Services & AccountSasServices::Files) == AccountSasServices::Files)
    {
      services += "f";
    }

    std::string resourceTypes;
    if ((ResourceTypes & AccountSasResource::Service) == AccountSasResource::Service)
    {
      resourceTypes += "s";
    }
    if ((ResourceTypes & AccountSasResource::Container) == AccountSasResource::Container)
    {
      resourceTypes += "c";
    }
    if ((ResourceTypes & AccountSasResource::Object) == AccountSasResource::Object)
    {
      resourceTypes += "o";
    }

    std::string startsOnStr = StartsOn.HasValue() ? StartsOn.GetValue().ToString(
                                  Azure::Core::DateTime::DateFormat::Rfc3339,
                                  Azure::Core::DateTime::TimeFractionFormat::Truncate)
                                                  : "";
    std::string expiresOnStr = ExpiresOn.ToString(
        Azure::Core::DateTime::DateFormat::Rfc3339,
        Azure::Core::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = credential.AccountName + "\n" + Permissions + "\n" + services + "\n"
        + resourceTypes + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n"
        + Storage::_detail::DefaultSasVersion + "\n";

    std::string signature = Azure::Core::Base64Encode(Storage::_detail::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::_detail::UrlEncodeQueryParameter(Storage::_detail::DefaultSasVersion));
    builder.AppendQueryParameter("ss", Storage::_detail::UrlEncodeQueryParameter(services));
    builder.AppendQueryParameter("srt", Storage::_detail::UrlEncodeQueryParameter(resourceTypes));
    builder.AppendQueryParameter("sp", Storage::_detail::UrlEncodeQueryParameter(Permissions));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", startsOnStr);
    }
    builder.AppendQueryParameter("se", expiresOnStr);
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::_detail::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    builder.AppendQueryParameter("spr", Storage::_detail::UrlEncodeQueryParameter(protocol));
    builder.AppendQueryParameter("sig", Storage::_detail::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Sas
