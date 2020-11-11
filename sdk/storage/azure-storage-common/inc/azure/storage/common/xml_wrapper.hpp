// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>

struct _xmlTextReader;
struct _xmlTextWriter;
struct _xmlBuffer;

namespace Azure { namespace Storage { namespace Details {

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
    _xmlTextReader* m_reader = nullptr;
    bool m_readingAttributes = false;
  };

  class XmlWriter {
  public:
    explicit XmlWriter();
    ~XmlWriter();

    void Write(XmlNode node);

    std::string GetDocument();

  private:
    _xmlBuffer* m_buffer = nullptr;
    _xmlTextWriter* m_writer = nullptr;
  };

}}} // namespace Azure::Storage::Details
