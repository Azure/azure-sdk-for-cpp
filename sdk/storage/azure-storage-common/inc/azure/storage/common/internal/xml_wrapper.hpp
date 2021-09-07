// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Storage { namespace _internal {

  enum class XmlNodeType
  {
    StartTag,
    EndTag,
    Text,
    Attribute,
    End,
  };

  struct XmlNode final
  {
    explicit XmlNode(
        XmlNodeType type,
        std::string name = std::string(),
        std::string value = std::string())
        : Type(type), Name(std::move(name)), Value(std::move(value))
    {
    }

    XmlNodeType Type;
    std::string Name;
    std::string Value;
  };

  class XmlReader final {
  public:
    explicit XmlReader(const char* data, size_t length);
    XmlReader(const XmlReader& other) = delete;
    XmlReader& operator=(const XmlReader& other) = delete;
    XmlReader(XmlReader&& other) noexcept { *this = std::move(other); }
    XmlReader& operator=(XmlReader&& other) noexcept
    {
      m_context = other.m_context;
      other.m_context = nullptr;
      return *this;
    }
    ~XmlReader();

    XmlNode Read();

  private:
    void* m_context = nullptr;
  };

  class XmlWriter final {
  public:
    explicit XmlWriter();
    XmlWriter(const XmlWriter& other) = delete;
    XmlWriter& operator=(const XmlWriter& other) = delete;
    XmlWriter(XmlWriter&& other) noexcept { *this = std::move(other); }
    XmlWriter& operator=(XmlWriter&& other) noexcept
    {
      m_context = other.m_context;
      other.m_context = nullptr;
      return *this;
    }
    ~XmlWriter();

    void Write(XmlNode node);

    std::string GetDocument();

  private:
    void* m_context = nullptr;
  };

}}} // namespace Azure::Storage::_internal
