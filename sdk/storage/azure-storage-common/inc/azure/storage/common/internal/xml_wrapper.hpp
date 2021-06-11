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
    SelfClosingTag,
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
    ~XmlReader();

    XmlNode Read();

  private:
    void* m_reader = nullptr;
    bool m_readingAttributes = false;
  };

  class XmlWriter final {
  public:
    explicit XmlWriter();
    ~XmlWriter();

    void Write(XmlNode node);

    std::string GetDocument();

  private:
    void* m_buffer = nullptr;
    void* m_writer = nullptr;
  };

}}} // namespace Azure::Storage::_internal
