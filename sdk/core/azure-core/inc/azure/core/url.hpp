// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define an Uniform Resource Location (URL).
 */

#pragma once

#include "azure/core/case_insensitive_containers.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

namespace Azure { namespace Core {
  namespace _detail {
    /**
     * @brief Insert a header into \p headers checking that \p headerName does not contain invalid
     * characters.
     *
     * @param headers The headers map where to insert header.
     * @param headerName The header name for the header to be inserted.
     * @param headerValue The header value for the header to be inserted.
     *
     * @throw if \p headerName is invalid.
     */
    void InsertHeaderWithValidation(
        CaseInsensitiveMap& headers,
        std::string const& headerName,
        std::string const& headerValue);

    inline std::string FormatEncodedUrlQueryParameters(
        std::map<std::string, std::string> const& encodedQueryParameters)
    {
      {
        std::string queryStr;
        if (!encodedQueryParameters.empty())
        {
          auto separ = '?';
          for (const auto& q : encodedQueryParameters)
          {
            queryStr += separ + q.first + '=' + q.second;
            separ = '&';
          }
        }

        return queryStr;
      }
    }
  } // namespace _detail

  /**
   * @brief Url represents the location where a request will be performed.
   * It can be parsed and initialized from a string that contains all URL components (scheme, host,
   * path, etc.). Authority is not currently supported.
   */
  class Url {
  private:
    std::string m_scheme;
    std::string m_host;
    uint16_t m_port{0};
    std::string m_encodedPath;
    // query parameters are all encoded
    std::map<std::string, std::string> m_encodedQueryParameters;

    // List of default non-URL-encode chars. While URL encoding a string, do not escape any chars in
    // this set.
    const static std::unordered_set<unsigned char> defaultNonUrlEncodeChars;

    std::string GetUrlWithoutQuery(bool relative) const;

  public:
    /**
     * @brief Decodes \p value by transforming all escaped characters to it's non-encoded value.
     *
     * @param value URL-encoded string.
     * @return std::string with non-URL encoded values.
     */
    static std::string Decode(const std::string& value);

    /**
     * @brief Encodes \p value by escaping characters to the form of %HH where HH are hex digits.
     *
     * @remark \p doNotEncodeSymbols arg can be used to explicitly ask this function to skip
     * characters from encoding. For instance, using this `= -` input would prevent encoding `=`, `
     * ` and `-`.
     *
     * @param value Non URL-encoded string.
     * @param doNotEncodeSymbols A string consisting of characters that do not need to be encoded.
     * @return std::string
     */
    static std::string Encode(const std::string& value, const std::string& doNotEncodeSymbols = "");

    /**
     * @brief Constructs a new, empty URL object.
     *
     */
    Url() {}

    /**
     * @brief Construct a URL from a URL-encoded string.
     *
     * @param encodedUrl URL string that has all its expected parts already URL-encoded.
     */
    explicit Url(const std::string& encodedUrl);

    /************* Builder Url functions ****************/
    /******** API for building Url from scratch. Override state ********/

    /**
     * @brief Set URL scheme.
     *
     * @param scheme URL scheme.
     */
    void SetScheme(const std::string& scheme) { m_scheme = scheme; }

    /**
     * @brief Set URL host.
     *
     * @param host URL host.
     */
    void SetHost(const std::string& encodedHost) { m_host = encodedHost; }

    /**
     * @brief Set URL port.
     *
     * @param port URL port.
     */
    void SetPort(uint16_t port) { m_port = port; }

    /**
     * @brief Set URL path.
     *
     * @param path URL path.
     */
    void SetPath(const std::string& encodedPath) { m_encodedPath = encodedPath; }

    /**
     * @brief Set the query parameters from an existing query parameter map.
     *
     * @remark Keys and values in \p queryParameters are expected to be URL-encoded.
     *
     * @param queryParameters
     */
    void SetQueryParameters(std::map<std::string, std::string> queryParameters)
    {
      // creates a copy and discard previous
      m_encodedQueryParameters = std::move(queryParameters);
    }

    // ===== APIs for mutating URL state: ======

    /**
     * @brief Append an element of URL path.
     *
     * @param path URL path element to append.
     */
    void AppendPath(const std::string& encodedPath)
    {
      if (!m_encodedPath.empty() && m_encodedPath.back() != '/')
      {
        m_encodedPath += '/';
      }
      m_encodedPath += encodedPath;
    }

    /**
     * @brief The value of a query parameter is expected to be non-URL-encoded and, by default, it
     * will be encoded before adding to the URL. Use \p isValueEncoded = true when the
     * value is already encoded.
     *
     * @remark This function overrides the value of existing query parameters.
     *
     * @param encodedKey Name of the query parameter, already encoded.
     * @param encodedValue Value of the query parameter, already encoded.
     */
    void AppendQueryParameter(const std::string& encodedKey, const std::string& encodedValue)
    {
      m_encodedQueryParameters[encodedKey] = encodedValue;
    }

    /**
     * @brief Finds the first '?' symbol and parses everything after it as query parameters.
     * separated by '&'.
     *
     * @param encodedQueryParameters String containing one or more query parameters.
     */
    void AppendQueryParameters(const std::string& encodedQueryParameters);

    /**
     * @brief Removes an existing query parameter.
     *
     * @param encodedKey The name of the query parameter to be removed.
     */
    void RemoveQueryParameter(const std::string& encodedKey)
    {
      m_encodedQueryParameters.erase(encodedKey);
    }

    /************** API to read values from Url ***************/
    /**
     * @brief Get URL host.
     */
    const std::string& GetHost() const { return m_host; }

    /**
     * @brief Gets the URL path.
     *
     * @return const std::string&
     */
    const std::string& GetPath() const { return m_encodedPath; }

    /**
     * @brief Get the port number set for the URL.
     *
     * @remark If the port was not set for the url, the returned port is 0. An HTTP request cannot
     * be performed to port zero, an HTTP client is expected to set the default port depending on
     * the request's schema when the port was not defined in the URL.
     *
     * @return The port number from the URL.
     */
    uint16_t GetPort() const { return m_port; }

    /**
     * @brief Get a copy of the list of query parameters from the URL.
     *
     * @remark The query parameters are URL-encoded.
     *
     * @return A copy of the query parameters map.
     */
    std::map<std::string, std::string> GetQueryParameters() const
    {
      return m_encodedQueryParameters;
    }

    /**
     * @brief Get Scheme, host, and path, without query parameters.
     * @return Absolute URL without query parameters.
     */
    std::string GetUrlWithoutQuery() const { return GetUrlWithoutQuery(false); }

    /**
     * @brief Get the path and query parameters.
     *
     * @return Relative URL with URL-encoded query parameters.
     */
    std::string GetRelativeUrl() const;

    /**
     * @brief Get Scheme, host, path and query parameters.
     *
     * @return Absolute URL with URL-encoded query parameters.
     */
    std::string GetAbsoluteUrl() const;
  };
}} // namespace Azure::Core
