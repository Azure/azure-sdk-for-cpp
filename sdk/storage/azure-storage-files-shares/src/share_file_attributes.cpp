// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_file_attributes.hpp"

#include <algorithm>
#include <iterator>

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  FileAttributes::FileAttributes(const std::string& value)
  {
    std::string::const_iterator cur = value.begin();
    while (cur != value.end())
    {
      auto delimiter_pos
          = std::find_if(cur, value.end(), [](char c) { return c == ' ' || c == '|'; });
      m_value.emplace_back(std::string(cur, delimiter_pos));
      while (delimiter_pos != value.end() && (*delimiter_pos == ' ' || *delimiter_pos == '|'))
      {
        ++delimiter_pos;
      }
      cur = delimiter_pos;
    }
    std::sort(m_value.begin(), m_value.end());
  }

  FileAttributes FileAttributes::operator|(const FileAttributes& other) const
  {
    FileAttributes ret;
    std::set_union(
        m_value.begin(),
        m_value.end(),
        other.m_value.begin(),
        other.m_value.end(),
        std::back_inserter(ret.m_value));
    return ret;
  }

  FileAttributes FileAttributes::operator&(const FileAttributes& other) const
  {
    FileAttributes ret;
    std::set_intersection(
        m_value.begin(),
        m_value.end(),
        other.m_value.begin(),
        other.m_value.end(),
        std::back_inserter(ret.m_value));
    return ret;
  }

  FileAttributes FileAttributes::operator^(const FileAttributes& other) const
  {
    FileAttributes ret;
    std::set_symmetric_difference(
        m_value.begin(),
        m_value.end(),
        other.m_value.begin(),
        other.m_value.end(),
        std::back_inserter(ret.m_value));
    return ret;
  }

  std::string FileAttributes::ToString() const
  {
    std::string ret;
    for (const auto& v : m_value)
    {
      if (!ret.empty())
      {
        ret += " | ";
      }
      ret += v;
    }
    return ret;
  }

  const FileAttributes FileAttributes::ReadOnly("ReadOnly");
  const FileAttributes FileAttributes::Hidden("Hidden");
  const FileAttributes FileAttributes::System("System");
  const FileAttributes FileAttributes::None("None");
  const FileAttributes FileAttributes::Directory("Directory");
  const FileAttributes FileAttributes::Archive("Archive");
  const FileAttributes FileAttributes::Temporary("Temporary");
  const FileAttributes FileAttributes::Offline("Offline");
  const FileAttributes FileAttributes::NotContentIndexed("NotContentIndexed");
  const FileAttributes FileAttributes::NoScrubData("NoScrubData");

}}}}} // namespace Azure::Storage::Files::Shares::Models
