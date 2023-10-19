// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef REAL_MAP_H
#define REAL_MAP_H

#define REGISTER_MAP_GLOBAL_MOCK_HOOK \
    REGISTER_GLOBAL_MOCK_HOOK(Map_Create, real_Map_Create); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_Destroy, real_Map_Destroy); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_Clone, real_Map_Clone); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_Add, real_Map_Add); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_AddOrUpdate, real_Map_AddOrUpdate); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_Delete, real_Map_Delete); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_ContainsKey, real_Map_ContainsKey); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_ContainsValue, real_Map_ContainsValue); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_GetValueFromKey, real_Map_GetValueFromKey); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_GetInternals, real_Map_GetInternals); \
    REGISTER_GLOBAL_MOCK_HOOK(Map_ToJSON, real_Map_ToJSON);

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif
    extern MAP_HANDLE real_Map_Create(MAP_FILTER_CALLBACK mapFilterFunc);
    extern void real_Map_Destroy(MAP_HANDLE handle);
    extern MAP_HANDLE real_Map_Clone(MAP_HANDLE handle);
    extern MAP_RESULT real_Map_Add(MAP_HANDLE handle, const char* key, const char* value);
    extern MAP_RESULT real_Map_AddOrUpdate(MAP_HANDLE handle, const char* key, const char* value);
    extern MAP_RESULT real_Map_Delete(MAP_HANDLE handle, const char* key);
    extern MAP_RESULT real_Map_ContainsKey(MAP_HANDLE handle, const char* key, bool* keyExists);
    extern MAP_RESULT real_Map_ContainsValue(MAP_HANDLE handle, const char* value, bool* valueExists);
    extern const char* real_Map_GetValueFromKey(MAP_HANDLE handle, const char* key);
    extern MAP_RESULT real_Map_GetInternals(MAP_HANDLE handle, const char*const** keys, const char*const** values, size_t* count);
    extern STRING_HANDLE real_Map_ToJSON(MAP_HANDLE handle);
#ifdef __cplusplus
}
#endif

#endif // !REAL_MAP_H
