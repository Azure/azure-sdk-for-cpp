// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/tables/table_sas_builder.hpp"

#include "azure/storage/tables/rest_client.hpp"

#include <azure/core/http/http.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Sas {

  namespace {
    constexpr static const char* SasVersion = Tables::_detail::ApiVersion;
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
    if ((permissions & TableSasPermissions::Process) == TableSasPermissions::Process)
    {
      Permissions += "p";
    }
  }

  std::string TableSasBuilder::GenerateSasToken(const StorageSharedKeyCredential& credential)
  {
    std::string canonicalName = "/table/" + credential.AccountName + "/"; // + QueueName;

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

    std::string signature = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Convert::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Url builder;
    builder.AppendQueryParameter("sv", _internal::UrlEncodeQueryParameter(SasVersion));
    builder.AppendQueryParameter("spr", _internal::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", _internal::UrlEncodeQueryParameter(startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter("se", _internal::UrlEncodeQueryParameter(expiresOnStr));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter("sip", _internal::UrlEncodeQueryParameter(IPRange.Value()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", _internal::UrlEncodeQueryParameter(Identifier));
    }
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", _internal::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter("sig", _internal::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Sas
