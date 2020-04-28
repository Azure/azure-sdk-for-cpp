// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/xml_wrapper.hpp"
#include <stdexcept>
#include <iostream>

namespace Azure
{
namespace Storage
{
namespace Common
{
namespace XML
{

std::string xml_char_to_string(const xmlChar* xml_char)
{
  return std::string(reinterpret_cast<const char*>(xml_char));
}

xml_text_reader_wrapper::xml_text_reader_wrapper(const unsigned char* buffer, unsigned int size)
{
  m_reader = xmlReaderForMemory((const char*)buffer, size, NULL, 0, 0);
}

xml_text_reader_wrapper::~xml_text_reader_wrapper()
{
  if (m_reader != nullptr)
  {
    xmlFreeTextReader(m_reader);
    m_reader = nullptr;
  }
}

bool xml_text_reader_wrapper::read() { return xmlTextReaderRead(m_reader) == 1; }

unsigned xml_text_reader_wrapper::get_node_type() { return xmlTextReaderNodeType(m_reader); }

bool xml_text_reader_wrapper::is_empty_element()
{
  return xmlTextReaderIsEmptyElement(m_reader) == 1;
}

std::string xml_text_reader_wrapper::get_local_name()
{
  auto xml_char = xmlTextReaderLocalName(m_reader);
  std::string result;

  if (xml_char != nullptr)
  {
    result = xml_char_to_string(xml_char);
    xmlFree(xml_char);
  }

  return result;
}

std::string xml_text_reader_wrapper::get_value()
{
  auto xml_char = xmlTextReaderValue(m_reader);
  std::string result;

  if (xml_char != nullptr)
  {
    result = xml_char_to_string(xml_char);
    xmlFree(xml_char);
  }
  return result;
}

bool xml_text_reader_wrapper::move_to_first_attribute()
{
  return xmlTextReaderMoveToFirstAttribute(m_reader) == 1;
}

bool xml_text_reader_wrapper::move_to_next_attribute()
{
  return xmlTextReaderMoveToNextAttribute(m_reader) == 1;
}

xml_element_wrapper::~xml_element_wrapper() {}

xml_element_wrapper::xml_element_wrapper(xmlNode* node)
{
  m_ele = node;
  m_ele->_private = this;
}

xml_element_wrapper* xml_element_wrapper::add_child(
    const std::string& name,
    const std::string& prefix)
{
  xmlNs* ns = nullptr;
  xmlNode* child = nullptr;

  if (m_ele->type != XML_ELEMENT_NODE)
  {
    return nullptr;
  }

  if (!prefix.empty())
  {
    ns = xmlSearchNs(m_ele->doc, m_ele, (const xmlChar*)prefix.c_str());
    if (!ns)
    {
      return nullptr;
    }
  }

  child = xmlNewNode(ns, (const xmlChar*)name.c_str()); // mem leak?

  if (!child)
    return nullptr;

  xmlNode* node = xmlAddChild(m_ele, child);
  if (!node)
    return nullptr;

  node->_private = new xml_element_wrapper(node);
  return reinterpret_cast<xml_element_wrapper*>(node->_private);
}

void xml_element_wrapper::set_namespace_declaration(
    const std::string& uri,
    const std::string& prefix)
{
  xmlNewNs(
      m_ele,
      (const xmlChar*)(uri.empty() ? nullptr : uri.c_str()),
      (const xmlChar*)(prefix.empty() ? nullptr : prefix.c_str()));
}

void xml_element_wrapper::set_namespace(const std::string& prefix)
{
  xmlNs* ns
      = xmlSearchNs(m_ele->doc, m_ele, (const xmlChar*)(prefix.empty() ? nullptr : prefix.c_str()));
  if (ns)
  {
    xmlSetNs(m_ele, ns);
  }
}

void xml_element_wrapper::set_attribute(
    const std::string& name,
    const std::string& value,
    const std::string& prefix)
{
  xmlAttr* attr = 0;

  if (prefix.empty())
  {
    attr = xmlSetProp(m_ele, (const xmlChar*)name.c_str(), (const xmlChar*)value.c_str());
  }
  else
  {
    xmlNs* ns = xmlSearchNs(m_ele->doc, m_ele, (const xmlChar*)prefix.c_str());
    if (ns)
    {
      attr = xmlSetNsProp(m_ele, ns, (const xmlChar*)name.c_str(), (const xmlChar*)value.c_str());
    }
    else
    {
      return;
    }
  }

  if (attr)
  {
    attr->_private = new xml_element_wrapper(reinterpret_cast<xmlNode*>(attr));
  }
}

void xml_element_wrapper::set_child_text(const std::string& text)
{
  xml_element_wrapper* node = nullptr;

  for (xmlNode* child = m_ele->children; child; child = child->next)
    if (child->type == xmlElementType::XML_TEXT_NODE)
    {
      child->_private = new xml_element_wrapper(child);
      node = reinterpret_cast<xml_element_wrapper*>(child->_private);
    }

  if (node)
  {
    if (node->m_ele->type != xmlElementType::XML_ELEMENT_NODE)
    {
      xmlNodeSetContent(node->m_ele, (const xmlChar*)text.c_str());
    }
  }
  else
  {
    if (m_ele->type == XML_ELEMENT_NODE)
    {
      xmlNode* childNode = xmlNewText((const xmlChar*)text.c_str());

      childNode = xmlAddChild(m_ele, childNode);

      childNode->_private = new xml_element_wrapper(childNode);
    }
  }
}

void xml_element_wrapper::free_wrappers(xmlNode* node)
{
  if (!node)
    return;

  for (xmlNode* child = node->children; child; child = child->next)
    free_wrappers(child);

  switch (node->type)
  {
    case XML_DTD_NODE:
    case XML_ELEMENT_DECL:
    case XML_ATTRIBUTE_NODE:
    case XML_ATTRIBUTE_DECL:
    case XML_ENTITY_DECL:
      if (node->_private)
      {
        delete reinterpret_cast<xml_element_wrapper*>(node->_private);
        node->_private = nullptr;
      }
      break;
    case XML_DOCUMENT_NODE:
      break;
    default:
      if (node->_private)
      {
        delete reinterpret_cast<xml_element_wrapper*>(node->_private);
        node->_private = nullptr;
      }
      break;
  }
}

xml_document_wrapper::xml_document_wrapper()
{
  m_doc = xmlNewDoc(reinterpret_cast<const xmlChar*>("1.0"));
}

xml_document_wrapper::~xml_document_wrapper()
{
  xml_element_wrapper::free_wrappers(reinterpret_cast<xmlNode*>(m_doc));
  xmlFreeDoc(m_doc);
  m_doc = nullptr;
}

std::string xml_document_wrapper::write_to_string()
{
  xmlIndentTreeOutput = 0;
  xmlChar* buffer = 0;
  int size = 0;

  xmlDocDumpFormatMemoryEnc(m_doc, &buffer, &size, 0, 0);

  std::string result;

  if (buffer)
  {
    result = std::string(
        reinterpret_cast<const char*>(buffer), reinterpret_cast<const char*>(buffer + size));

    xmlFree(buffer);
  }
  return result;
}

xml_element_wrapper* xml_document_wrapper::create_root_node(
    const std::string& name,
    const std::string& namespace_name,
    const std::string& prefix)
{
  xmlNode* node = xmlNewDocNode(m_doc, 0, (const xmlChar*)name.c_str(), 0);
  xmlDocSetRootElement(m_doc, node);

  xml_element_wrapper* element = get_root_node();

  if (!namespace_name.empty())
  {
    element->set_namespace_declaration(namespace_name, prefix);
    element->set_namespace(prefix);
  }

  return element;
}

xml_element_wrapper* xml_document_wrapper::get_root_node() const
{
  xmlNode* root = xmlDocGetRootElement(m_doc);
  if (root == NULL)
    return NULL;
  else
  {
    root->_private = new xml_element_wrapper(root);
    return reinterpret_cast<xml_element_wrapper*>(root->_private);
  }

  return nullptr;
}

void xml_writer::initialize(std::ostream& stream)
{
  m_document.reset(new xml_document_wrapper());
  m_elementStack = std::stack<xml_element_wrapper*>();
  m_stream = &stream;
}

void xml_writer::finalize()
{
  auto result = m_document->write_to_string();
  if (m_stream != nullptr)
    *m_stream << reinterpret_cast<const char*>(result.c_str());
}

void xml_writer::write_start_element_with_prefix(
    const std::string& elementPrefix,
    const std::string& elementName,
    const std::string& namespaceName)
{
  if (m_elementStack.empty())
  {
    m_elementStack.push(m_document->create_root_node(elementName, namespaceName, elementPrefix));
  }
  else
  {
    m_elementStack.push(m_elementStack.top()->add_child(elementName, elementPrefix));
    if (!namespaceName.empty())
    {
      m_elementStack.top()->set_namespace_declaration(namespaceName, elementPrefix);
    }
  }
}

void xml_writer::write_start_element(
    const std::string& elementName,
    const std::string& namespaceName)
{
  write_start_element_with_prefix(std::string(), elementName, namespaceName);
}

void xml_writer::write_end_element()
{
  m_elementStack.pop();
}

void xml_writer::write_full_end_element()
{
  throw std::runtime_error("Not implemented");
}

void xml_writer::write_string(const std::string& str)
{
  throw std::runtime_error("Not implemented");
}

void xml_writer::write_attribute_string(
    const std::string& prefix,
    const std::string& name,
    const std::string& namespaceUri,
    const std::string& value)
{
  if (prefix == "xmlns")
  {
    m_elementStack.top()->set_namespace_declaration(value, name);
  }
  else
  {
    m_elementStack.top()->set_attribute(name, value, prefix);
  }
}

void xml_writer::write_element(const std::string& elementName, const std::string& value)
{
  write_element_with_prefix(std::string(), elementName, value);
}

void xml_writer::write_element_with_prefix(
    const std::string& prefix,
    const std::string& elementName,
    const std::string& value)
{
  write_start_element_with_prefix(prefix, elementName);
  m_elementStack.top()->set_child_text(value);
  write_end_element();
}
} // namespace XML
} // namespace Common
} // namespace Storage
} // namespace Azure
