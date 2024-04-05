// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/tables_sas_builder.hpp"

#include "azure/data/tables/internal/cryptography/hmacsha256.hpp"
#include "azure/data/tables/internal/cryptography/url_encode.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/http/http.hpp>

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  namespace {
    constexpr static const char* SasVersion = "2019-07-07";
  }

  void TablesSasBuilder::SetPermissions(TablesSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & TablesSasPermissions::Read) == TablesSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & TablesSasPermissions::Add) == TablesSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & TablesSasPermissions::Update) == TablesSasPermissions::Update)
    {
      Permissions += "u";
    }
    if ((permissions & TablesSasPermissions::Delete) == TablesSasPermissions::Delete)
    {
      Permissions += "d";
    }
  }

  std::string TablesSasBuilder::GenerateSasToken(
      const Azure::Data::Tables::Credentials::NamedKeyCredential& credential)
  {
    std::string canonicalName
        = Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
            GetCanonicalName(credential));

    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = ExpiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    // the order here matters
    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.Value() : "")
        + "\n" + protocol + "\n" + SasVersion + "\n" + PartitionKeyStart + "\n" + RowKeyStart + "\n"
        + PartitionKeyEnd + "\n" + RowKeyEnd;

    std::string signature = Azure::Core::Convert::Base64Encode(
        Azure::Data::Tables::_detail::Cryptography::HmacSha256::Compute(
            std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
            Azure::Core::Convert::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Url builder;

    builder.AppendQueryParameter(
        "sv",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(SasVersion));

    builder.AppendQueryParameter(
        "tn",
        Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(TableName));

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

    if (!PartitionKeyStart.empty())
    {
      builder.AppendQueryParameter(
          "spk",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              PartitionKeyStart));
      if (!PartitionKeyEnd.empty())
      {
        builder.AppendQueryParameter(
            "epk",
            Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
                PartitionKeyEnd));
      }
    }

    if (!RowKeyStart.empty())
    {
      builder.AppendQueryParameter(
          "srk",
          Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
              RowKeyStart));
      if (!RowKeyEnd.empty())
      {
        builder.AppendQueryParameter(
            "erk",
            Azure::Data::Tables::_detail::Cryptography::UrlUtils::UrlEncodeQueryParameter(
                RowKeyEnd));
      }
    }

    return builder.GetAbsoluteUrl();
  }
}}}} // namespace Azure::Data::Tables::Sas
