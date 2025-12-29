# Verification: UpdateSocketReuse Function

## ✅ CONFIRMED: Your custom function is present and active

### 1. Function Definition Location
**File**: `sdk/core/azure-core/src/http/curl/curl.cpp`  
**Line**: 1306-1334

```cpp
void CurlConnection::UpdateSocketReuse()
{
  curl_socket_t currentSocket = CURL_SOCKET_BAD;
  CURLcode result = curl_easy_getinfo(m_handle.get(), CURLINFO_ACTIVESOCKET, &currentSocket);
  
  if (result == CURLE_OK && currentSocket != CURL_SOCKET_BAD)
  {
    if (m_previousSocket == CURL_SOCKET_BAD)
    {
      // First request on this handle
      Log::Write(Logger::Level::Informational, 
          "[CURL] First request - socket=" + std::to_string(currentSocket));
    }
    else if (m_previousSocket == currentSocket)
    {
      // Same socket = connection reused!
      Log::Write(Logger::Level::Informational, 
          "[CURL] *** CONNECTION REUSED *** - socket=" + std::to_string(currentSocket));
    }
    else
    {
      // Different socket = new connection
      Log::Write(Logger::Level::Informational, 
          "[CURL] NEW connection created - old_socket=" + std::to_string(m_previousSocket) 
          + ", new_socket=" + std::to_string(currentSocket));
    }
    m_previousSocket = currentSocket;
  }
}
```

### 2. Function Declaration
**File**: `sdk/core/azure-core/src/http/curl/curl_connection_private.hpp`  
**Line**: 243

```cpp
void UpdateSocketReuse();
```

### 3. Where It's Called
**File**: `sdk/core/azure-core/src/http/curl/curl.cpp`  
**Line**: 707  
**Function**: `CurlSession::SendRawHttp()`

```cpp
CURLcode CurlSession::SendRawHttp(Context const& context)
{
  // Check connection reuse before sending request
  m_connection->UpdateSocketReuse();
  
  // ... rest of the function
}
```

### 4. Symbol Verification in Compiled Library

**Command**:
```powershell
dumpbin /SYMBOLS "C:\Users\kraman\azure-sdk-local\lib\azure-core.lib" | Select-String "UpdateSocketReuse"
```

**Result**: ✅ Symbol found in library
```
?UpdateSocketReuse@CurlConnection@Http@Core@Azure@@UEAAXXZ 
(public: virtual void __cdecl Azure::Core::Http::CurlConnection::UpdateSocketReuse(void))
```

### 5. How It Works

1. **Before every HTTP request** is sent, `SendRawHttp()` is called
2. **First action** in `SendRawHttp()` is to call `UpdateSocketReuse()`
3. **The function checks** the current socket and compares it to the previous socket
4. **Logs one of three messages**:
   - `[CURL] First request - socket=X` - First request on this CURL handle
   - `[CURL] *** CONNECTION REUSED *** - socket=X` - Same socket reused
   - `[CURL] NEW connection created - old_socket=X, new_socket=Y` - New connection

### 6. How to See the Logs

When you use the SDK in your application, set the log level to `Informational` or higher:

```cpp
#include <azure/core/diagnostics/logger.hpp>

// In your application startup
Azure::Core::Diagnostics::Logger::SetListener(
    [](auto level, auto message) {
        std::cout << message << std::endl;
    });

Azure::Core::Diagnostics::Logger::SetLevel(
    Azure::Core::Diagnostics::Logger::Level::Informational);
```

Then you'll see output like:
```
[CURL] First request - socket=1234
[CURL] *** CONNECTION REUSED *** - socket=1234
[CURL] NEW connection created - old_socket=1234, new_socket=5678
```

### 7. Quick Verification Commands

```powershell
# Check if function exists in source
grep -r "UpdateSocketReuse" sdk/core/azure-core/src/

# Check if symbol is in compiled library
dumpbin /SYMBOLS "C:\Users\kraman\azure-sdk-local\lib\azure-core.lib" | Select-String "UpdateSocketReuse"

# Verify library was installed
Test-Path "C:\Users\kraman\azure-sdk-local\lib\azure-core.lib"
```

---

## Summary

✅ **Function exists** in source code  
✅ **Function compiled** into library  
✅ **Function called** before every HTTP request  
✅ **Library installed** at `C:\Users\kraman\azure-sdk-local\`

Your custom connection reuse tracking is fully integrated and ready to use!
