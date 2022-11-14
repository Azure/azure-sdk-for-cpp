//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/internal/strings.hpp"

#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

namespace {
// returns left map plus all items in right
// when duplicates, left items are preferred
static Azure::Core::CaseInsensitiveMap MergeMaps(
    Azure::Core::CaseInsensitiveMap left,
    Azure::Core::CaseInsensitiveMap const& right)
{
  left.insert(right.begin(), right.end());
  return left;
}
} // namespace

Azure::Nullable<std::string> Request::GetHeader(std::string const& name)
{
  std::vector<std::string> returnedHeaders;
  auto headerNameLowerCase = Azure::Core::_internal::StringExtensions::ToLower(name);

  auto retryHeader = this->m_retryHeaders.find(headerNameLowerCase);
  if (retryHeader != this->m_retryHeaders.end())
  {
    return retryHeader->second;
  }
  auto header = this->m_headers.find(headerNameLowerCase);
  if (header != this->m_headers.end())
  {
    return header->second;
  }
  return Azure::Nullable<std::string>{};
}

void Request::SetHeader(std::string const& name, std::string const& value)
{
  auto headerNameLowerCase = Azure::Core::_internal::StringExtensions::ToLower(name);
  return this->m_retryModeEnabled ? _detail::RawResponseHelpers::InsertHeaderWithValidation(
             this->m_retryHeaders, headerNameLowerCase, value)
                                  : _detail::RawResponseHelpers::InsertHeaderWithValidation(
                                      this->m_headers, headerNameLowerCase, value);
}

void Request::RemoveHeader(std::string const& name)
{
  this->m_headers.erase(name);
  this->m_retryHeaders.erase(name);
}

void Request::StartTry()
{
  this->m_retryModeEnabled = true;
  this->m_retryHeaders.clear();

  // Make sure to rewind the body stream before each attempt, including the first.
  // It's possible the request doesn't have a body, so make sure to check if a body stream exists.
  if (auto bodyStream = this->GetBodyStream())
  {
    bodyStream->Rewind();
  }
}

HttpMethod const& Request::GetMethod() const { return this->m_method; }

Azure::Core::CaseInsensitiveMap Request::GetHeaders() const
{
  // create map with retry headers which are the most important and we don't want
  // to override them with any duplicate header
  return MergeMaps(this->m_retryHeaders, this->m_headers);
}
