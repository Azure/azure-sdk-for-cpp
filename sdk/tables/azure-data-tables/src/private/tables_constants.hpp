// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Centralize the string constants used by Table Clients.
 *
 */

#pragma once

namespace Azure { namespace Data { namespace Tables { namespace _detail {
  /**
   * The package name of the SDK.
   */
  constexpr static const char* TablesServicePackageName = "data-tables";
  // various strings used in the library
  constexpr static const char* OriginHeader = "Origin";
  constexpr static const char* AccessControlRequestMethodHeader = "Access-Control-Request-Method";
  constexpr static const char* ResrouceTypeService = "service";
  constexpr static const char* ComponentProperties = "properties";
  constexpr static const char* ContentTypeXml = "application/xml";
  constexpr static const char* ContentTypeJson = "application/json";
  constexpr static const char* ResourceTypeHeader = "restype";
  constexpr static const char* CompHeader = "comp";
  constexpr static const char* ContentTypeHeader = "Content-Type";
  constexpr static const char* ContentLengthHeader = "Content-Length";
  constexpr static const char* AcceptHeader = "Accept";
  constexpr static const char* PreferHeader = "Prefer";
  constexpr static const char* PreferNoContent = "return-no-content";
  constexpr static const char* AcceptFullMeta = "application/json;odata=fullmetadata";
  constexpr static const char* IfMatch = "If-Match";
  constexpr static const char* PartitionKeyFragment = "(PartitionKey='";
  constexpr static const char* RowKeyFragment = "',RowKey='";
  constexpr static const char* ClosingFragment = "')";
  constexpr static const char* Value = "value";
  constexpr static const char* TableName = "TableName";
  constexpr static const char* ODataEditLink = "odata.editLink";
  constexpr static const char* ODataId = "odata.id";
  constexpr static const char* ODataType = "odata.type";
  constexpr static const char* ODataMeta = "odata.metadata";
  constexpr static const char* ODataError = "odata.error";
}}}} // namespace Azure::Data::Tables::_detail
