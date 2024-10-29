// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Declaration of the UserAgentGenerator type.
 */

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  class UserAgentGenerator {

    // This doesn't behave as expected, locally, depending on how the tests are written.
    // TODO: Consider removing these target_compile_definitions.
    static const long CppStandardVersion =
#if defined(_azure_BUILDING_SDK)
        -2L
#elif defined(_azure_BUILDING_TESTS)
        -1L
#elif defined(_azure_BUILDING_SAMPLES)
        0L
#else
        __cplusplus
#endif
        ;

  public:
    static std::string GenerateUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);
  };
}}}} // namespace Azure::Core::Http::_internal
