// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/xml_wrapper.hpp"

#include "libxml/xmlreader.h"
#include "libxml/xmlwriter.h"

#include <limits>
#include <stdexcept>

namespace Azure { namespace Storage {

  struct XmlGlobalInitializer
  {
    XmlGlobalInitializer() { xmlInitParser(); }
    ~XmlGlobalInitializer() { xmlCleanupParser(); }
  };

  static void XmlGlobalInitialize() { static XmlGlobalInitializer globalInitializer; }

  XmlReader::XmlReader(const char* data, std::size_t length)
  {
    XmlGlobalInitialize();

    if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
      throw std::runtime_error("xml data too big");
    }

    m_reader = xmlReaderForMemory(data, static_cast<int>(length), nullptr, nullptr, 0);
    if (!m_reader)
    {
      throw std::runtime_error("failed to parse xml");
    }
  }

  XmlReader::~XmlReader() { xmlFreeTextReader(m_reader); }

  XmlNode XmlReader::Read()
  {
    if (m_readingAttributes)
    {
      int ret = xmlTextReaderMoveToNextAttribute(m_reader);
      if (ret == 1)
      {
        const char* name = reinterpret_cast<const char*>(xmlTextReaderName(m_reader));
        const char* value = reinterpret_cast<const char*>(xmlTextReaderValue(m_reader));
        return XmlNode{XmlNodeType::Attribute, name, value};
      }
      else if (ret == 0)
      {
        m_readingAttributes = false;
      }
      else
      {
        throw std::runtime_error("failed to parse xml");
      }
    }

    int ret = xmlTextReaderRead(m_reader);
    if (ret == 0)
    {
      return XmlNode{XmlNodeType::End};
    }
    if (ret != 1)
    {
      throw std::runtime_error("failed to parse xml");
    }

    int type = xmlTextReaderNodeType(m_reader);
    bool is_empty = xmlTextReaderIsEmptyElement(m_reader) == 1;
    bool has_value = xmlTextReaderHasValue(m_reader) == 1;
    bool has_attributes = xmlTextReaderHasAttributes(m_reader) == 1;

    const char* name = reinterpret_cast<const char*>(xmlTextReaderName(m_reader));
    const char* value = reinterpret_cast<const char*>(xmlTextReaderValue(m_reader));

    if (has_attributes)
    {
      m_readingAttributes = true;
    }

    if (type == XML_READER_TYPE_ELEMENT && is_empty)
    {
      return XmlNode{XmlNodeType::SelfClosingTag, name};
    }
    else if (type == XML_READER_TYPE_ELEMENT)
    {
      return XmlNode{XmlNodeType::StartTag, name};
    }
    else if (type == XML_READER_TYPE_END_ELEMENT)
    {
      return XmlNode{XmlNodeType::EndTag, name};
    }
    else if (type == XML_READER_TYPE_TEXT)
    {
      if (has_value)
      {
        return XmlNode{XmlNodeType::Text, nullptr, value};
      }
    }
    else if (type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
    {
      // silently ignore
    }
    else
    {
      throw std::runtime_error("unknown type " + std::to_string(type) + " while parsing xml");
    }

    return Read();
  }

  XmlWriter::XmlWriter()
  {
    XmlGlobalInitialize();
    m_buffer = xmlBufferCreate();
    m_writer = xmlNewTextWriterMemory(m_buffer, 0);
    xmlTextWriterStartDocument(m_writer, nullptr, nullptr, nullptr);
  }

  XmlWriter::~XmlWriter()
  {
    xmlFreeTextWriter(m_writer);
    xmlBufferFree(m_buffer);
  }

  void XmlWriter::Write(XmlNode node)
  {
    if (node.Type == XmlNodeType::StartTag)
    {
      if (!node.Value)
      {
        xmlTextWriterStartElement(m_writer, BAD_CAST(node.Name));
      }
      else
      {
        xmlTextWriterWriteElement(m_writer, BAD_CAST(node.Name), BAD_CAST(node.Value));
      }
    }
    else if (node.Type == XmlNodeType::EndTag)
    {
      xmlTextWriterEndElement(m_writer);
    }
    else if (node.Type == XmlNodeType::SelfClosingTag)
    {
      xmlTextWriterStartElement(m_writer, BAD_CAST(node.Name));
      xmlTextWriterEndElement(m_writer);
    }
    else if (node.Type == XmlNodeType::Text)
    {
      xmlTextWriterWriteString(m_writer, BAD_CAST(node.Value));
    }
    else if (node.Type == XmlNodeType::Attribute)
    {
      xmlTextWriterWriteAttribute(m_writer, BAD_CAST(node.Name), BAD_CAST(node.Value));
    }
    else if (node.Type == XmlNodeType::End)
    {
      xmlTextWriterEndDocument(m_writer);
    }
    else
    {
      throw std::runtime_error(
          "unsupported XmlNode type "
          + std::to_string(static_cast<std::underlying_type<XmlNodeType>::type>(node.Type)));
    }
  }

  std::string XmlWriter::GetDocument()
  {
    xmlTextWriterFlush(m_writer);
    return std::string(reinterpret_cast<const char*>(m_buffer->content), m_buffer->use);
  }

}} // namespace Azure::Storage
