# WinHTTP Build Fix - WIL_ENABLE_EXCEPTIONS

## Problem
WinHTTP transport compilation was failing with:
```
win_http_request.hpp(53): error C2039: 'unique_event': is not a member of 'wil'
```

## Root Cause
The WIL library (Windows Implementation Libraries) defines `wil::unique_event` conditionally:
```cpp
#ifdef WIL_ENABLE_EXCEPTIONS
typedef unique_any_t<event_t<details::unique_storage<details::handle_resource_policy>, err_exception_policy>> unique_event;
#endif
```

The WinHTTP transport code uses `wil::unique_event` at line 53 of `win_http_request.hpp`:
```cpp
wil::unique_event m_actionCompleteEvent;
```

Without `WIL_ENABLE_EXCEPTIONS` defined, the `unique_event` type is not available.

## Solution
Added `WIL_ENABLE_EXCEPTIONS` preprocessor definition to the CMake flags:

**Before:**
```powershell
"-DCMAKE_CXX_FLAGS=/DCURL_STATICLIB"
```

**After:**
```powershell
"-DCMAKE_CXX_FLAGS=/DCURL_STATICLIB /DWIL_ENABLE_EXCEPTIONS"
```

## Verification
Build completed successfully with both transports:
- ✅ CURL transport: `curl.cpp` compiled with UpdateSocketReuse() function
- ✅ WinHTTP transport: `win_http_transport.cpp` compiled successfully

Library symbols verified:
```
# WinHTTP symbols present
dumpbin /SYMBOLS azure-core.lib | Select-String "WinHttp"
-> Found WinHttpTransport constructor and related symbols

# UpdateSocketReuse present
dumpbin /SYMBOLS azure-core.lib | Select-String "UpdateSocketReuse"
-> Found UpdateSocketReuse@CurlConnection symbol

# Static CURL linkage confirmed
dumpbin /SYMBOLS azure-core.lib | Select-String "curl_easy"
-> Shows curl_easy_* (static) not __imp_curl_easy_* (DLL)
```

## Build Configuration Summary
The library now successfully builds with:
- ✅ /MT static runtime (MSVC_USE_STATIC_CRT=ON)
- ✅ Static CURL linkage (CURL_STATICLIB preprocessor define)
- ✅ CURL transport with UpdateSocketReuse() socket tracking
- ✅ WinHTTP transport (WIL_ENABLE_EXCEPTIONS preprocessor define)
- ✅ Both transports available in single library

## Usage
To rebuild:
```powershell
.\build-sdk.ps1 -Config Release -CleanBuild
```

The library will be installed to: `C:\Users\kraman\azure-sdk-local`
