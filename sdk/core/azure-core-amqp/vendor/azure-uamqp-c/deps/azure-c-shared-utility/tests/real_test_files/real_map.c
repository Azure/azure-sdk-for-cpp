// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define GBALLOC_H

#define Map_Create          real_Map_Create
#define Map_Destroy         real_Map_Destroy
#define Map_Clone           real_Map_Clone
#define Map_Add             real_Map_Add
#define Map_AddOrUpdate     real_Map_AddOrUpdate
#define Map_Delete          real_Map_Delete
#define Map_ContainsKey     real_Map_ContainsKey
#define Map_ContainsValue   real_Map_ContainsValue
#define Map_GetValueFromKey real_Map_GetValueFromKey
#define Map_GetInternals    real_Map_GetInternals
#define Map_ToJSON          real_Map_ToJSON

#include "map.c"
