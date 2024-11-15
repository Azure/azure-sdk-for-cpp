// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/account_sas_builder.hpp"

#include "private/hmacsha256.hpp"
#include "private/url_encode.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/http/http.hpp>

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  namespace {
    constexpr const char* SasVersion = "2023-08-03";
  }

  void AccountSasBuilder::SetPermissions(AccountSasPermissionsFlags permissions)
  {
    Permissions.clear();
    if ((permissions & AccountSasPermissionsFlags::Read) == AccountSasPermissionsFlags::Read)
    {
      Permissions += "r";
    }
    if ((permissions & AccountSasPermissionsFlags::Write) == AccountSasPermissionsFlags::Write)
    {
      Permissions += "w";
    }
    if ((permissions & AccountSasPermissionsFlags::Delete) == AccountSasPermissionsFlags::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & AccountSasPermissionsFlags::List) == AccountSasPermissionsFlags::List)
    {
      Permissions += "l";
    }
    if ((permissions & AccountSasPermissionsFlags::Add) == AccountSasPermissionsFlags::Add)
    {
      Permissions += "a";
    }
    if ((permissions & AccountSasPermissionsFlags::Update) == AccountSasPermissionsFlags::Update)
    {
      Permissions += "u";
    }
  }

  std::string AccountSasBuilder::GenerateSasToken(
      const Azure::Data::Tables::Credentials::NamedKeyCredential& credential)
  {
    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string services;
    if ((Services & AccountSasServicesFlags::Table) == AccountSasServicesFlags::Table)
    {
      services += "t";
    }

    std::string resourceTypes;
    if ((ResourceTypes & AccountSasResourceTypeFlags::Service)
        == AccountSasResourceTypeFlags::Service)
    {
      resourceTypes += "s";
    }
    if ((ResourceTypes & AccountSasResourceTypeFlags::Container)
        == AccountSasResourceTypeFlags::Container)
    {
      resourceTypes += "c";
    }
    if ((ResourceTypes & AccountSasResourceTypeFlags::Object)
        == AccountSasResourceTypeFlags::Object)
    {
      resourceTypes += "o";
    }

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
              Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = ExpiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = credential.AccountName + "\n" + Permissions + "\n" + services + "\n"
        + resourceTypes + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + (IPRange.HasValue() ? IPRange.Value() : "") + "\n" + protocol + "\n" + SasVersion + "\n"
        + EncryptionScope + "\n";

    std::string signature = Azure::Core::Convert::Base64Encode(
        Azure::Data::Tables::_detail::Cryptography::HmacSha256::Compute(
            std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
            Azure::Core::Convert::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Url builder;
    builder.AppendQueryParameter(
        "sv",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(SasVersion));
    builder.AppendQueryParameter(
        "ss",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(services));
    builder.AppendQueryParameter(
        "srt",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
            resourceTypes));
    builder.AppendQueryParameter(
        "sp",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(Permissions));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", startsOnStr);
    }
    builder.AppendQueryParameter("se", expiresOnStr);
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              IPRange.Value()));
    }
    builder.AppendQueryParameter(
        "spr",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(protocol));
    builder.AppendQueryParameter(
        "sig",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(signature));
    if (!EncryptionScope.empty())
    {
      builder.AppendQueryParameter(
          "ses",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              EncryptionScope));
    }

    return builder.GetAbsoluteUrl();
  }

}}}} // namespace Azure::Data::Tables::Sas
