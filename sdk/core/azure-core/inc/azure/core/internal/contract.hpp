// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if defined(NO_CONTRACTS_CHECKING)
#define AZURE_CONTRACT(condition, error)
#define AZURE_CONTRACT_ARG_NOT_NULL(arg)
#else
#define AZURE_CONTRACT(condition, error) \
  do \
  { \
    if (!(condition)) \
    { \
      return error; \
    } \
  } while (0)

#define AZURE_CONTRACT_ARG_NOT_NULL(arg) AZURE_CONTRACT((arg) != NULL, 1)

#endif
