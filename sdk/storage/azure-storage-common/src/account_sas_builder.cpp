// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/account_sas_builder.hpp"
#include "azure/core/http/http.hpp"
#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage {

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

  std::string AccountSasBuilder::ToSasQueryParameters(const SharedKeyCredential& credential)
  {
    std::string protocol;
    if (Protocol == SasProtocol::HttpsAndHtttp)
    {
      protocol = "https,http";
    }
    else
    {
      protocol = "https";
    }
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

    std::string stringToSign = credential.AccountName + "\n" + Permissions + "\n" + services + "\n"
        + resourceTypes + "\n" + (StartsOn.HasValue() ? StartsOn.GetValue() : "") + "\n" + ExpiresOn
        + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n" + Version
        + "\n";

    std::string signature
        = Base64Encode(Details::HmacSha256(stringToSign, Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQuery("sv", Version);
    builder.AppendQuery("ss", services);
    builder.AppendQuery("srt", resourceTypes);
    builder.AppendQuery("sp", Permissions);
    if (StartsOn.HasValue())
    {
      builder.AppendQuery("st", StartsOn.GetValue());
    }
    builder.AppendQuery("se", ExpiresOn);
    if (IPRange.HasValue())
    {
      builder.AppendQuery("sip", IPRange.GetValue());
    }
    builder.AppendQuery("spr", protocol);
    builder.AppendQuery("sig", signature);

    return builder.GetAbsoluteUrl();
  }

}} // namespace Azure::Storage
