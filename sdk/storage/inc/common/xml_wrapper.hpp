// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

// TODO: remove this
// TODO: Fix the naming convention in this file and corresponding .cpp file.
#pragma warning(disable : 4239)

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string>
#include <stack>
#include <memory>

namespace Azure
{
namespace Storage
{
namespace Common
{
namespace XML
{

std::string xml_char_to_string(const xmlChar* xml_char);

/// <summary>
/// A class to wrap xmlTextReader of c library libxml2. This class provides abilities to read from
/// xml format texts.
/// </summary>
class xml_text_reader_wrapper
{
public:
  xml_text_reader_wrapper(const unsigned char* buffer, unsigned int size);

  ~xml_text_reader_wrapper();

  /// <summary>
  /// Moves to the next node in the stream.
  /// </summary>
  /// <returns>true if the node was read successfully and false if there are no more nodes to
  /// read</returns>
  bool read();

  /// <summary>
  /// Gets the type of the current node.
  /// </summary>
  /// <returns>A integer that represent the type of the node</returns>
  unsigned get_node_type();

  /// <summary>
  /// Checks if current node is empty
  /// </summary>
  /// <returns>True if current node is empty and false if otherwise</returns>
  bool is_empty_element();

  /// <summary>
  /// Gets the local name of the node
  /// </summary>
  /// <returns>A string indicates the local name of this node</returns>
  std::string get_local_name();

  /// <summary>
  /// Gets the value of the node
  /// </summary>
  /// <returns>A string value of the node</returns>
  std::string get_value();

  /// <summary>
  /// Moves to the first attribute of the node.
  /// </summary>
  /// <returns>True if the move is successful, false if empty</returns>
  bool move_to_first_attribute();

  /// <summary>
  /// Moves to the next attribute of the node.
  /// </summary>
  /// <returns>True if the move is successful, false if empty</returns>
  bool move_to_next_attribute();

private:
  xmlTextReaderPtr m_reader;
};

/// <summary>
/// A class to wrap xmlNode of c library libxml2. This class provides abilities to create xml nodes.
/// </summary>
class xml_element_wrapper
{
public:
  xml_element_wrapper();

  ~xml_element_wrapper();

  xml_element_wrapper(xmlNode* node);

  /// <summary>
  /// Adds a child element to this node.
  /// </summary>
  /// <param name="name">The name of the child node.</param>
  /// <param name="prefix">The namespace prefix of the child node.</param>
  /// <returns>The created child node.</returns>
  xml_element_wrapper* add_child(const std::string& name, const std::string& prefix);

  /// <summary>
  /// Adds a namespace declaration to the node.
  /// </summary>
  /// <param name="uri">The namespace to associate with the prefix.</param>
  /// <param name="prefix">The namespace prefix</param>
  void set_namespace_declaration(const std::string& uri, const std::string& prefix);

  /// <summary>
  /// Set the namespace prefix
  /// </summary>
  /// <param name="prefix">name space prefix to be set</param>
  void set_namespace(const std::string& prefix);

  /// <summary>
  /// Sets the value of the attribute with this name (and prefix).
  /// </summary>
  /// <param name="name">The name of the attribute</param>
  /// <param name="value">The value of the attribute</param>
  /// <param name="prefix">The prefix of the attribute, this is optional.</param>
  void set_attribute(const std::string& name, const std::string& value, const std::string& prefix);

  /// <summary>
  /// Sets the text of the first text node. If there isn't a text node, add one and set it.
  /// </summary>
  /// <param name="text">The text to be set to the child node.</param>
  void set_child_text(const std::string& text);

  /// <summary>
  /// Frees the wrappers set in nod->_private
  /// </summary>
  /// <param name="node">The node to be freed.</param>
  /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
  static void free_wrappers(xmlNode* node);

private:
  xmlNode* m_ele;
};

/// <summary>
/// A class to wrap xmlDoc of c library libxml2. This class provides abilities to create xml format
/// texts from nodes.
/// </summary>
class xml_document_wrapper
{
public:
  xml_document_wrapper();

  ~xml_document_wrapper();

  /// <summary>
  /// Converts object to a string object.
  /// </summary>
  /// <returns>A std::string that contains the result</returns>
  std::string write_to_string();

  /// <summary>
  /// Creates the root node of the document.
  /// </summary>
  /// <param name="name">The name of the root node.</param>
  /// <param name="namespace_name">The namespace of the root node.</param>
  /// <param name="prefix">The namespace prefix of the root node.</param>
  /// <returns>A wrapper that contains the root node.</returns>
  xml_element_wrapper* create_root_node(
      const std::string& name,
      const std::string& namespace_name,
      const std::string& prefix);

  /// <summary>
  /// Gets the root node of the document.
  /// </summary>
  /// <returns>The root node of the document.</returns>
  xml_element_wrapper* get_root_node() const;

private:
  xmlDocPtr m_doc;
};

/// <summary>
/// XML writer based on libxml2
/// </summary>
class xml_writer
{
public:
  virtual ~xml_writer() {}

protected:
  xml_writer() : m_stream(nullptr)
  {
  }

  /// <summary>
  /// Initialize the writer
  /// </summary>
  void initialize(std::ostream& stream);

  /// <summary>
  /// Finalize the writer
  /// </summary>
  void finalize();

  /// <summary>
  /// Write the start element tag
  /// </summary>
  void write_start_element(
      const std::string& elementName,
      const std::string& namespaceName = std::string());

  /// <summary>
  /// Writes the start element tag with a prefix
  /// </summary>
  void write_start_element_with_prefix(
      const std::string& elementPrefix,
      const std::string& elementName,
      const std::string& namespaceName = std::string());

  /// <summary>
  /// Write the end element tag for the current element
  /// </summary>
  void write_end_element();

  /// <summary>
  /// Write the full end element tag for the current element
  /// </summary>
  void write_full_end_element();

  /// <summary>
  /// Write an element including the name and text.
  /// </summary>
  template <class T> void write_element(const std::string& elementName, T value)
  {
    write_element(elementName, convert_to_string(value));
  }

  /// <summary>
  /// Write an element including the name and text.
  /// </summary>
  void write_element(const std::string& elementName, const std::string& value);

  /// <summary>
  /// Write an element including the prefix, name and text.
  /// </summary>
  void write_element_with_prefix(
      const std::string& prefix,
      const std::string& elementName,
      const std::string& value);

  /// <summary>
  /// Write raw data
  /// </summary>
  void write_raw(const std::string& data);

  /// <summary>
  /// Write a string
  /// </summary>
  void write_string(const std::string& string);

  /// <summary>
  /// Write an attribute string with a prefix
  /// </summary>
  void write_attribute_string(
      const std::string& prefix,
      const std::string& name,
      const std::string& namespaceUri,
      const std::string& value);

  /// <summary>
  /// Logs an error from processing XML
  /// </summary>
  virtual void log_error_message(const std::string& message, unsigned long error = 0)
  {
  }

private:
  std::shared_ptr<xml_document_wrapper> m_document;
  std::stack<xml_element_wrapper*> m_elementStack;
  std::ostream* m_stream;
};

} // namespace XML
} // namespace Common
} // namespace Storage
} // namespace Azure