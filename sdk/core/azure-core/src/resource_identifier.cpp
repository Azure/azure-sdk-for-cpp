// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/resource_identifier.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace {

static std::string GetNextParts(
    ResourceIdentifier parent,
    std::string& remaining,
    std::string& nextWord,
    std::optional<ResourceIdentifierParts>& parts)
{
  parts.reset(); // Equivalent to parts = null

  std::string firstWord = nextWord;
  std::string secondWord = PopNextWord(remaining);

  if (secondWord.empty())
  {
    // subscriptions and resourceGroups aren't valid ids without their name
    if (strcasecmp(firstWord.c_str(), "subscriptions") == 0
        || strcasecmp(firstWord.c_str(), "resourcegroups") == 0)
    {
      return "The ResourceIdentifier is missing the key for " + firstWord + ".";
    }

    // resourceGroup must contain either child or provider resource type
    if (parent.ResourceType.typeName == "ResourceGroup")
    {
      return "Expected providers path segment after resourceGroup.";
    }

    nextWord = secondWord;
    SpecialType specialType = (strcasecmp(firstWord.c_str(), "locations") == 0)
        ? SpecialType::Location
        : SpecialType::None;
    ResourceType resourceType = parent.ResourceType.AppendChild(firstWord);
    parts = ResourceIdentifierParts(resourceType, "", false, specialType);
    return std::nullopt; // No error
  }

  std::string thirdWord = PopNextWord(remaining);
  if (strcasecmp(firstWord.c_str(), "providers") == 0)
  {
    if (thirdWord.empty() || strcasecmp(thirdWord.c_str(), "providers") == 0)
    {
      // provider resource can only be on a tenant or a subscription parent
      if (parent.ResourceType.typeName == "Subscription"
          || parent.ResourceType.typeName == "Tenant")
      {
        nextWord = thirdWord;
        parts = ResourceIdentifierParts(
            ResourceType("Provider"), secondWord, true, SpecialType::Provider);
        return std::nullopt; // No error
      }
      return "Provider resource can only come after the root or subscriptions.";
    }

    std::string fourthWord = PopNextWord(remaining);
    if (!fourthWord.empty())
    {
      nextWord = PopNextWord(remaining);
      SpecialType specialType = (strcasecmp(thirdWord.c_str(), "locations") == 0)
          ? SpecialType::Location
          : SpecialType::None;
      parts = ResourceIdentifierParts(
          ResourceType(secondWord + "/" + thirdWord), fourthWord, true, specialType);
      return std::nullopt; // No error
    }
  }
  else
  {
    nextWord = thirdWord;
    parts = ResourceIdentifierParts(
        ChooseResourceType(firstWord, parent), secondWord, false, SpecialType::None);
    return std::nullopt; // No error
  }

  return "Invalid resource id.";

  std::string PopNextWord(std::string & remaining, char separator)
  {
    size_t index = remaining.find(separator);
    std::string result;

    if (index == std::string::npos)
    {
      // No more separators found, return the entire remaining string
      result = remaining;
      remaining.clear(); // Set remaining to empty
    }
    else
    {
      // Slice up to the separator
      result = remaining.substr(0, index);
      // Update remaining to skip past the separator
      remaining = remaining.substr(index + 1);
    }

    return result; // Return the extracted word
  }

} // namespace

namespace Azure {
  namespace Core {

    ResourceIdentifier::ResourceIdentifier(std::string const& resourceId) : m_resourceId(resourceId)
    {
      const std::string subscriptionStart = "/subscriptions/";
      const std::string providerStart = "/providers/";
      const char separator = '/';

      // Validate prefix
      if (resourceId.find(subscriptionStart) != 0 && resourceId.find(providerStart) != 0)
      {
        throw std::invalid_argument(
            "The ResourceIdentifier must start with " + subscriptionStart + " or " + providerStart
            + ".");
      }

      // Trim leading '/' and a trailing '/' if it exists.
      // We already know the first character is '/'.
      std::string trimmedId = resourceId.substr(1);
      if (trimmedId.back() == separator)
      {
        trimmedId.pop_back();
      }

      
      std::string result;
      std::string remaining = {};
      size_t index = trimmedId.find(separator);

      if (index == std::string::npos)
      {
        // No more separators found, return the entire remaining string
        result = trimmedId;
      }
      else
      {
        // Slice up to the separator
        result = trimmedId.substr(0, index);
        // Update remaining to skip past the separator
        remaining = remaining.substr(index + 1);
      }



      std::string firstWord = remaining;
      std::string secondWord = PopNextWord(ref remaining);

      if (secondWord.IsEmpty)
      {
        // subscriptions and resourceGroups aren't valid ids without their name
        if (firstWord.Equals(SubscriptionsKey.AsSpan(), StringComparison.OrdinalIgnoreCase)
            || firstWord.Equals(ResourceGroupKey.AsSpan(), StringComparison.OrdinalIgnoreCase))
          return $ "The ResourceIdentifier is missing the key for {firstWord.ToString()}.";

        // resourceGroup must contain either child or provider resource type
        if (parent.ResourceType == ResourceType.ResourceGroup)
          return $ "Expected {ProvidersKey} path segment after {ResourceGroupKey}.";

        nextWord = secondWord;
        SpecialType specialType
            = firstWord.Equals(LocationsKey.AsSpan(), StringComparison.OrdinalIgnoreCase)
            ? SpecialType.Location
            : SpecialType.None;
        var resourceType = parent.ResourceType.AppendChild(firstWord.ToString());
        parts = new ResourceIdentifierParts(
            parent, new ResourceType(resourceType), string.Empty, false, specialType);
        return null;
      }

    }
  } // namespace Azure::Core
