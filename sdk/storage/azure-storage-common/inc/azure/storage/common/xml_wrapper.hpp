// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Storage { namespace _detail {

  enum class XmlNodeType
  {
    StartTag,
    EndTag,
    SelfClosingTag,
    Text,
    Attribute,
    End,
  };

  struct XmlNode
  {
    explicit XmlNode(XmlNodeType type, const char* name = nullptr, const char* value = nullptr)
        : Type(type), Name(name), Value(value)
    {
    }
    XmlNodeType Type;
    const char* Name;
    const char* Value;
  };

  class XmlReader {
  public:
    explicit XmlReader(const char* data, std::size_t length);
    ~XmlReader();

    XmlNode Read();

  private:
    void* m_reader = nullptr;
    bool m_readingAttributes = false;
  };

  class XmlWriter {
  public:
    explicit XmlWriter();
    ~XmlWriter();

    void Write(XmlNode node);

    std::string GetDocument();

  private:
    void* m_buffer = nullptr;
    void* m_writer = nullptr;
  };

}}} // namespace Azure::Storage::_detail
