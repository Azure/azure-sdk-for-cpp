// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/table_sas_builder.hpp"

#include "azure/data/tables/internal/cryptography/hmacsha256.hpp"
#include "azure/data/tables/internal/cryptography/url_encode.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/http/http.hpp>

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  namespace {
    constexpr static const char* SasVersion = "2023-08-03";
  }

  void TableSasBuilder::SetPermissions(TableSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & TableSasPermissions::Read) == TableSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & TableSasPermissions::Add) == TableSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & TableSasPermissions::Update) == TableSasPermissions::Update)
    {
      Permissions += "u";
    }
    if ((permissions & TableSasPermissions::Delete) == TableSasPermissions::Delete)
    {
      Permissions += "d";
    }
  }

  std::string TableSasBuilder::GenerateSasToken(
      const Azure::Data::Tables::Credentials::SharedKeyCredential& credential)
  {
    std::string canonicalName = "/table/" + credential.AccountName + "/" + TableName;

    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.Value() : "")
        + "\n" + protocol + "\n" + SasVersion;

    std::string signature = Azure::Core::Convert::Base64Encode(
        Azure::Data::Tables::_detail::Cryptography::HmacSha256::Compute(
            std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
            Azure::Core::Convert::Base64Decode(credential.GetAccountKey())));
    Azure::Core::Url builder;
    builder.AppendQueryParameter(
        "sv",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
            SasVersion));
    builder.AppendQueryParameter(
        "spr",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter(
          "st",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter(
          "se",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              expiresOnStr));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              IPRange.Value()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter(
          "si",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              Identifier));
    }
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter(
          "sp",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              Permissions));
    }
    builder.AppendQueryParameter(
        "sig",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }
}}}} // namespace Azure::Data::Tables::Sas