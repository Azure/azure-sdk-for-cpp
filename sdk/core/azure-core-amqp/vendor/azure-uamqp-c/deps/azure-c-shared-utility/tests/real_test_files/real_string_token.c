// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define StringToken_GetFirst real_StringToken_GetFirst
#define StringToken_GetNext real_StringToken_GetNext
#define StringToken_GetValue real_StringToken_GetValue
#define StringToken_GetLength real_StringToken_GetLength
#define StringToken_GetDelimiter real_StringToken_GetDelimiter
#define StringToken_Split real_StringToken_Split
#define StringToken_Destroy real_StringToken_Destroy

#define GBALLOC_H

#include "string_token.c"
