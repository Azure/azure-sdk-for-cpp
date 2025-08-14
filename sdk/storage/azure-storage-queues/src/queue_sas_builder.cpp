// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/queues/queue_sas_builder.hpp"

#include "azure/storage/queues/rest_client.hpp"

#include <azure/core/http/http.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Sas {

  namespace {
    constexpr static const char* SasVersion = Queues::_detail::ApiVersion;
  }

  void QueueSasBuilder::SetPermissions(QueueSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & QueueSasPermissions::Read) == QueueSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & QueueSasPermissions::Add) == QueueSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & QueueSasPermissions::Update) == QueueSasPermissions::Update)
    {
      Permissions += "u";
    }
    if ((permissions & QueueSasPermissions::Process) == QueueSasPermissions::Process)
    {
      Permissions += "p";
    }
  }

  std::string QueueSasBuilder::GenerateSasToken(const StorageSharedKeyCredential& credential)
  {
    std::string canonicalName = "/queue/" + credential.AccountName + "/" + QueueName;

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

  std::string QueueSasBuilder::GenerateSasToken(
      const Queues::Models::UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    std::string canonicalName = "/queue/" + accountName + "/" + QueueName;

    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string signedStartsOnStr = userDelegationKey.SignedStartsOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    std::string signedExpiresOnStr = userDelegationKey.SignedExpiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + signedStartsOnStr + "\n" + signedExpiresOnStr
        + "\n" + userDelegationKey.SignedService + "\n" + userDelegationKey.SignedVersion + "\n\n"
        + DelegatedUserObjectId + "\n" + (IPRange.HasValue() ? IPRange.Value() : "") + "\n"
        + protocol + "\n" + SasVersion;

    std::string signature = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Convert::Base64Decode(userDelegationKey.Value)));

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
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", _internal::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter(
        "skoid", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedObjectId));
    builder.AppendQueryParameter(
        "sktid", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedTenantId));
    builder.AppendQueryParameter("skt", _internal::UrlEncodeQueryParameter(signedStartsOnStr));
    builder.AppendQueryParameter("ske", _internal::UrlEncodeQueryParameter(signedExpiresOnStr));
    builder.AppendQueryParameter(
        "sks", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedService));
    builder.AppendQueryParameter(
        "skv", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedVersion));
    if (!DelegatedUserObjectId.empty())
    {
      builder.AppendQueryParameter(
          "sduoid", _internal::UrlEncodeQueryParameter(DelegatedUserObjectId));
    }
    builder.AppendQueryParameter("sig", _internal::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

  std::string QueueSasBuilder::GenerateSasStringToSign(const StorageSharedKeyCredential& credential)
  {
    std::string canonicalName = "/queue/" + credential.AccountName + "/" + QueueName;

    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";

    return Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n" + canonicalName + "\n"
        + Identifier + "\n" + (IPRange.HasValue() ? IPRange.Value() : "") + "\n" + protocol + "\n"
        + SasVersion;
  }

  std::string QueueSasBuilder::GenerateSasStringToSign(
      const Queues::Models::UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    std::string canonicalName = "/queue/" + accountName + "/" + QueueName;

    std::string protocol = _detail::SasProtocolToString(Protocol);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string signedStartsOnStr = userDelegationKey.SignedStartsOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    std::string signedExpiresOnStr = userDelegationKey.SignedExpiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);

    return Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n" + canonicalName + "\n"
        + userDelegationKey.SignedObjectId + "\n" + userDelegationKey.SignedTenantId + "\n"
        + signedStartsOnStr + "\n" + signedExpiresOnStr + "\n" + userDelegationKey.SignedService
        + "\n" + userDelegationKey.SignedVersion + "\n\n" + DelegatedUserObjectId + "\n"
        + (IPRange.HasValue() ? IPRange.Value() : "") + "\n" + protocol + "\n" + SasVersion;
  }
}}} // namespace Azure::Storage::Sas
