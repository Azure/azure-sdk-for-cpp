// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/account_sas_builder.hpp"
#include "common/crypt.hpp"
#include "common/storage_uri_builder.hpp"

namespace Azure { namespace Storage {

  void AccountSasBuilder::SetPermissions(AccountSasPermissions permissions)
  {
    Permissions.clear();
    if ((permissions & AccountSasPermissions::Read) != AccountSasPermissions::None)
    {
      Permissions += "r";
    }
    if ((permissions & AccountSasPermissions::Write) != AccountSasPermissions::None)
    {
      Permissions += "w";
    }
    if ((permissions & AccountSasPermissions::Delete) != AccountSasPermissions::None)
    {
      Permissions += "d";
    }
    if ((permissions & AccountSasPermissions::DeleteVersion) != AccountSasPermissions::None)
    {
      Permissions += "x";
    }
    if ((permissions & AccountSasPermissions::List) != AccountSasPermissions::None)
    {
      Permissions += "l";
    }
    if ((permissions & AccountSasPermissions::Add) != AccountSasPermissions::None)
    {
      Permissions += "a";
    }
    if ((permissions & AccountSasPermissions::Create) != AccountSasPermissions::None)
    {
      Permissions += "c";
    }
    if ((permissions & AccountSasPermissions::Update) != AccountSasPermissions::None)
    {
      Permissions += "u";
    }
    if ((permissions & AccountSasPermissions::Process) != AccountSasPermissions::None)
    {
      Permissions += "p";
    }
    if ((permissions & AccountSasPermissions::Tags) != AccountSasPermissions::None)
    {
      Permissions += "t";
    }
    if ((permissions & AccountSasPermissions::Filter) != AccountSasPermissions::None)
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
    if ((Services & AccountSasServices::Blobs) != AccountSasServices::None)
    {
      services += "b";
    }
    if ((Services & AccountSasServices::Queue) != AccountSasServices::None)
    {
      services += "q";
    }
    if ((Services & AccountSasServices::Files) != AccountSasServices::None)
    {
      services += "f";
    }

    std::string resourceTypes;
    if ((ResourceTypes & AccountSasResource::Service) != AccountSasResource::None)
    {
      resourceTypes += "s";
    }
    if ((ResourceTypes & AccountSasResource::Container) != AccountSasResource::None)
    {
      resourceTypes += "c";
    }
    if ((ResourceTypes & AccountSasResource::Object) != AccountSasResource::None)
    {
      resourceTypes += "o";
    }

    std::string stringToSign = credential.AccountName + "\n" + Permissions + "\n" + services + "\n"
        + resourceTypes + "\n" + StartsOn + "\n" + ExpiresOn + "\n" + IPRange + "\n" + protocol
        + "\n" + Version + "\n";

    std::string signature
        = Base64Encode(HMAC_SHA256(stringToSign, Base64Decode(credential.GetAccountKey())));

    UriBuilder builder;
    builder.AppendQuery("sv", Version);
    builder.AppendQuery("ss", services);
    builder.AppendQuery("srt", resourceTypes);
    builder.AppendQuery("sp", Permissions);
    builder.AppendQuery("st", StartsOn);
    builder.AppendQuery("se", ExpiresOn);
    if (!IPRange.empty())
    {
      builder.AppendQuery("sip", IPRange);
    }
    builder.AppendQuery("spr", protocol);
    builder.AppendQuery("sig", signature, true);

    return builder.ToString();
  }

}} // namespace Azure::Storage
