# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

Write-Host 'EnvVar: ' $env:ANONYMOUSCONTAINERIPV4ADDRESS
Write-Host 'EnvVar: ' $env:AUTHENTICATEDCONTAINERIPV4ADDRESS
if ($env:ANONYMOUSCONTAINERIPV4ADDRESS -ne $null) {
  $proxyServer='http://'+$env:ANONYMOUSCONTAINERIPV4ADDRESS+':3128'
}
else
{
  $proxyServer='http://127.0.0.1:3128'
}
if ($env:AUTHENTICATEDCONTAINERIPV4ADDRESS -ne $null) {
  $proxyAuthServer='http://'+$env:AUTHENTICATEDCONTAINERIPV4ADDRESS+':3129'
}
else
{
  $proxyAuthServer='http://127.0.0.1:3129'
}

Write-Host 'Proxy server: ' $proxyServer
Write-Host 'Authenticated Proxy server: ' $proxyAuthServer

curl -v https://azuresdkforcpp.azurewebsites.net/get -x $proxyServer
if ($? -eq $false) {
	Write-Error "Failed to connect to unauthenticated proxy for HTTPS"
}
curl -v http://azuresdkforcpp.azurewebsites.net/get -x $proxyServer
if ($? -eq $false) {
	Write-Error "Failed to connect to unauthenticated proxy for HTTP"
}

curl -v https://azuresdkforcpp.azurewebsites.net/get -x $proxyAuthServer -U user:password
if ($? -eq $false) {
	Write-Error "Failed to connect to authenticated proxy for HTTPS"
}
curl -v http://azuresdkforcpp.azurewebsites.net/get -x $proxyAuthServer -U user:password
if ($? -eq $false) {
	Write-Error "Failed to connect to authenticated proxy for HTTP"
}

Invoke-WebRequest -Uri https://azuresdkforcpp.azurewebsites.net/get -Proxy $proxyServer
Invoke-WebRequest -Uri http://azuresdkforcpp.azurewebsites.net/get -Proxy $proxyServer

[string]$proxyUser="user"
[string]$proxyPassword="password"
[secureString]$securePassword= ConvertTo-SecureString $proxyPassword -AsPlainText -Force
[pscredential]$proxyCredential= New-Object System.Management.Automation.PSCredential($proxyUser, $securePassword)

Invoke-WebRequest -Uri https://azuresdkforcpp.azurewebsites.net/get -Proxy $proxyAuthServer -ProxyCredential $proxyCredential
Invoke-WebRequest -Uri http://azuresdkforcpp.azurewebsites.net/get -Proxy $proxyAuthServer -ProxyCredential $proxyCredential
