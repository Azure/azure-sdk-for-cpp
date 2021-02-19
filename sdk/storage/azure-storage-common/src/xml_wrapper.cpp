// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/xml_wrapper.hpp"

#include <limits>
#include <stdexcept>

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace Azure { namespace Storage { namespace Details {

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

  XmlReader::~XmlReader() { xmlFreeTextReader(static_cast<xmlTextReaderPtr>(m_reader)); }

  XmlNode XmlReader::Read()
  {
    xmlTextReaderPtr reader = static_cast<xmlTextReaderPtr>(m_reader);
    if (m_readingAttributes)
    {
      int ret = xmlTextReaderMoveToNextAttribute(reader);
      if (ret == 1)
      {
        const char* name = reinterpret_cast<const char*>(xmlTextReaderName(reader));
        const char* value = reinterpret_cast<const char*>(xmlTextReaderValue(reader));
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

    int ret = xmlTextReaderRead(reader);
    if (ret == 0)
    {
      return XmlNode{XmlNodeType::End};
    }
    if (ret != 1)
    {
      throw std::runtime_error("failed to parse xml");
    }

    int type = xmlTextReaderNodeType(reader);
    bool is_empty = xmlTextReaderIsEmptyElement(reader) == 1;
    bool has_value = xmlTextReaderHasValue(reader) == 1;
    bool has_attributes = xmlTextReaderHasAttributes(reader) == 1;

    const char* name = reinterpret_cast<const char*>(xmlTextReaderName(reader));
    const char* value = reinterpret_cast<const char*>(xmlTextReaderValue(reader));

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
    m_writer = xmlNewTextWriterMemory(static_cast<xmlBufferPtr>(m_buffer), 0);
    xmlTextWriterStartDocument(static_cast<xmlTextWriterPtr>(m_writer), nullptr, nullptr, nullptr);
  }

  XmlWriter::~XmlWriter()
  {
    xmlFreeTextWriter(static_cast<xmlTextWriterPtr>(m_writer));
    xmlBufferFree(static_cast<xmlBufferPtr>(m_buffer));
  }

  namespace {
    inline xmlChar* BadCast(const char* x)
    {
      return const_cast<xmlChar*>(reinterpret_cast<const xmlChar*>(x));
    }
  } // namespace

  void XmlWriter::Write(XmlNode node)
  {
    xmlTextWriterPtr writer = static_cast<xmlTextWriterPtr>(m_writer);
    if (node.Type == XmlNodeType::StartTag)
    {
      if (!node.Value)
      {
        xmlTextWriterStartElement(writer, BadCast(node.Name));
      }
      else
      {
        xmlTextWriterWriteElement(writer, BadCast(node.Name), BadCast(node.Value));
      }
    }
    else if (node.Type == XmlNodeType::EndTag)
    {
      xmlTextWriterEndElement(writer);
    }
    else if (node.Type == XmlNodeType::SelfClosingTag)
    {
      xmlTextWriterStartElement(writer, BadCast(node.Name));
      xmlTextWriterEndElement(writer);
    }
    else if (node.Type == XmlNodeType::Text)
    {
      xmlTextWriterWriteString(writer, BadCast(node.Value));
    }
    else if (node.Type == XmlNodeType::Attribute)
    {
      xmlTextWriterWriteAttribute(writer, BadCast(node.Name), BadCast(node.Value));
    }
    else if (node.Type == XmlNodeType::End)
    {
      xmlTextWriterEndDocument(writer);
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
    xmlTextWriterPtr writer = static_cast<xmlTextWriterPtr>(m_writer);
    xmlBufferPtr buffer = static_cast<xmlBufferPtr>(m_buffer);
    xmlTextWriterFlush(writer);
    return std::string(reinterpret_cast<const char*>(buffer->content), buffer->use);
  }

}}} // namespace Azure::Storage::Details
