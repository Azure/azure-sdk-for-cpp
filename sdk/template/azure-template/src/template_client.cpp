// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/template/template_client.hpp"

#include "private/package_version.hpp"

#include <string>

using namespace Azure::Template;
using namespace Azure::Template::_detail;

std::string TemplateClient::ClientVersion() const { return PackageVersion::ToString(); }

TemplateClient::TemplateClient(TemplateClientOptions) {}

int TemplateClient::GetValue(int p1, int p2, int d) const
{
  if (p1 < 2 || p1 > 11 || p2 < 2 || p2 > 11 || d < 2 || d > 11)
  {
    return -1;
  }

  if (p1 == p2)
  {
    auto p = p1;

    if (p == 8 || p == 11)
    {
      return 4;
    }

    if (p == 9 && (d == 7 || d >= 10))
    {
      return 4;
    }

    if (p == 6 && d < 7)
    {
      return 4;
    }

    if (p == 4 && (d == 5 || d == 6))
    {
      return 4;
    }

    if (p != 5 && p != 10 && d < 8)
    {
      return 4;
    }
  }

  if (p1 == 11 || p2 == 11)
  {
    auto p = (p1 != 11) ? p1 : p2;

    if (p <= 6)
    {
      if (d < 7)
      {
        if (d == 5 || d == 6)
        {
          return 3;
        }

        if (p >= 4 && d == 4)
        {
          return 3;
        }

        if (p == 6 && d == 3)
        {
          return 3;
        }
      }

      return 1;
    }

    if (p == 7)
    {
      if (d >= 9)
      {
        return 1;
      }

      if (d == 7 || d == 8)
      {
        return 0;
      }

      return 2;
    }

    if (p == 8 && d == 6)
    {
      return 2;
    }

    return 0;
  }


  auto p = p1 + p2;

  if (p == 11)
  {
    return 3;
  }

  if (p >= 17)
  {
    return 0;
  }

  if (p > 12 && d < 7)
  {
    return 0;
  }

  if (p == 12 && d < 7 && d > 3)
  {
    return 0;
  }

  if (p == 10 && d < 10)
  {
    return 3;
  }

  if (p == 9 && d < 7 && d != 2)
  {
    return 3;
  }

  return 1;
}
