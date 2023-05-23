// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>
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
    XmlReader(const char* data, size_t length);
    XmlReader(const XmlReader& other) = delete;
    XmlReader& operator=(const XmlReader& other) = delete;
    XmlReader(XmlReader&& other) noexcept;
    XmlReader& operator=(XmlReader&& other) noexcept;
    ~XmlReader();

    XmlNode Read();

  private:
    struct XmlReaderContext;
    std::unique_ptr<XmlReaderContext> m_context;

  };

  class XmlWriter final {
  public:
    XmlWriter();
    XmlWriter(const XmlWriter& other) = delete;
    XmlWriter& operator=(const XmlWriter& other) = delete;
    XmlWriter(XmlWriter&& other) noexcept;
    XmlWriter& operator=(XmlWriter&& other) noexcept;
    ~XmlWriter();

    void Write(XmlNode node);

    std::string GetDocument();

  private:
    struct XmlWriterContext;
    std::unique_ptr<XmlWriterContext> m_context;
  };

  void XmlGlobalInitialize();
  void XmlGlobalDeinitialize();

}}} // namespace Azure::Storage::_internal
