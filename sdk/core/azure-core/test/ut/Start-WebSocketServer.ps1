# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
param(
  [string] $LogFileLocation = "$($env:BUILD_SOURCESDIRECTORY)/WebSocketServer.log" 
)

if ($IsWindows) { 
  Start-Process 'python.exe' `
    -ArgumentList 'websocket_server.py' `
    -NoNewWindow -PassThru -RedirectStandardOutput $LogFileLocation
} else { 
  Start-Process nohup 'python3 websocket_server.py' -RedirectStandardOutput $LogFileLocation
}
