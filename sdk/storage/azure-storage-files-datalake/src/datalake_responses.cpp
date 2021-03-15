// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_responses.hpp"

#include "azure/storage/files/datalake/datalake_utilities.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Models {

  Acl Acl::FromString(const std::string& aclString)
  {
    std::string::const_iterator cur = aclString.begin();
    std::string str1 = _detail::GetSubstringTillDelimiter(':', aclString, cur);
    std::string str2 = _detail::GetSubstringTillDelimiter(':', aclString, cur);
    std::string str3 = _detail::GetSubstringTillDelimiter(':', aclString, cur);
    std::string str4 = _detail::GetSubstringTillDelimiter(':', aclString, cur);

    Acl acl;
    if (str4.empty())
    {
      // Scope is implicit in this case.
      acl.Type = std::move(str1);
      acl.Id = std::move(str2);
      acl.Permissions = std::move(str3);
    }
    else
    {
      // Scope is not implicit in this case.
      acl.Scope = std::move(str1);
      acl.Type = std::move(str2);
      acl.Id = std::move(str3);
      acl.Permissions = std::move(str4);
    }

    return acl;
  }

  std::string Acl::ToString(const Acl& acl)
  {
    std::string result;
    if (acl.Scope.empty())
    {
      result = acl.Type + ":" + acl.Id + ":" + acl.Permissions;
    }
    else
    {
      result = acl.Scope + ":" + acl.Type + ":" + acl.Id + ":" + acl.Permissions;
    }
    return result;
  }

  std::vector<Acl> Acl::DeserializeAcls(const std::string& dataLakeAclsString)
  {
    std::vector<Acl> result;

    std::string::const_iterator cur = dataLakeAclsString.begin();

    while (cur != dataLakeAclsString.end())
    {
      result.emplace_back(
          FromString(_detail::GetSubstringTillDelimiter(',', dataLakeAclsString, cur)));
    }

    return result;
  }
  std::string Acl::SerializeAcls(const std::vector<Acl>& dataLakeAclArray)
  {
    std::string result;
    for (const auto& acl : dataLakeAclArray)
    {
      result.append(ToString(acl) + ",");
    }
    if (!result.empty())
    {
      result.pop_back();
    }
    return result;
  }

}}}}} // namespace Azure::Storage::Files::DataLake::Models
