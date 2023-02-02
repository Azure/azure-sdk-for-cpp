<!-- cspell:words gunicorn Nlcjpw userx Nlcng dvcm passwordx userc -->

# _Create squid proxy docker container

Running the proxy tests requires two docker containers, one hosting an anonymous squid proxy, the other hosting an authenticated squid proxy.

This container is derived from the ubuntu/squid container on dockerhub. This container is maintained by the Ubuntu team and has
a current version of Ubuntu in it, with a version of squid built to match.

## _Building the container

There are 4 docker images configured for testing proxy tests:

- Anonymous Squid Proxy to be run on localhost.
- Authenticated Squid Proxy to be run on localhost.
- Anonymous Squid Proxy to be run as an Azure Container Instance (used for live tests).
- Authenticated Squid Proxy to be run as an Azure Container Instance (used for live tests).

To create the local anonymous docker container execute the following command (tested in powershell, should work in all shells):

```powershell
cd sdk\core\azure-core\test\ut\proxy_tests\proxy
docker build -t squid-local .
```

To create the local authenticated docker container execute the following command (tested in powershell, should work in all shells):

```powershell

cd sdk\core\azure-core\test\ut\proxy_tests\proxy.passwd
docker build -t squid-local-passwd .
```

Alternatively, the powershell script `builddocker.ps1` located in the sdk\core\azure-core\test\ut\proxy_tests directory can be used to build all 4 docker images.

| Image Purpose       | Image Name                                   | Notes                                                                                                                         |
| ------------------- | -------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| squid-local         | Localhost Anonymous Proxy                    |
| squid-local.passwd  | Localhost Authenticated Proxy                | One user supported "user", password "password"                                                                                |
| squid-remote        | Azure Container Instance Anonymous Proxy     | Proxy only works for resources on the azuresdkforcpp.azurewebsites.net domain.                                                |
| squid-remote.passwd | Azure Container Instance Authenticated Proxy | Proxy only works for resources on the azuresdkforcpp.azurewebsites.net domain. One user supported "user", password "password" |

## _Executing the containers

There are two squid proxy configuration files used, one which supports anonymous proxy access, the other
which supports basic authentication.

### _Executing the anonymous proxy

The anonymous proxy configuration is found in the `proxy` subdirectory, the authenticated proxy is in `proxy.passwd`.
To start the anonymous proxy, execute the following command:

```powershell
 docker run --rm -d -p 3128:3128 squid-local
```

This will launch the anonymous proxy running on port 3128.

### _Executing the authenticated proxy

To start the authenticated proxy, execute the following command:

```powershell
docker run --rm -d -p 3129:3129 squid-local-passwd
```

This will launch the anonymous proxy running on port 3129.

Alternatively, the `runproxy.ps1` script will start the proxies on localhost.

### _Deploying the proxy in azure container instances

To deploy the proxy in an azure container instance, the new-testresource.ps1 script:

```powershell
..\..\eng\common\TestResources\New-TestResources.ps1 -ServiceDirectory core
2:03:00 PM - BaseName was not set. Using default base name 'larryocore'
2:03:00 PM - Attempting to select subscription 'Azure SDK Developer Playground (faa080af-c1d8-40ad-9cce-e1a450ca5b57)'
2:03:04 PM - Using subscription 'Azure SDK Developer Playground (faa080af-c1d8-40ad-9cce-e1a450ca5b57)'
WARNING: Any clean-up scripts running against subscription 'faa080af-c1d8-40ad-9cce-e1a450ca5b57' may delete resource group 'rg-larryocore' after 120 hours.
2:03:05 PM - Creating resource group 'rg-larryocore' in location 'westus'

2:03:09 PM - TestApplicationId was not specified; creating a new service principal in subscription 'faa080af-c1d8-40ad-9cce-e1a450ca5b57'
WARNING: Assigning role 'Owner' over scope '/subscriptions/faa080af-c1d8-40ad-9cce-e1a450ca5b57/resourceGroups/rg-larryocore' to the new service principal.
2:03:49 PM - Created service principal. AppId: '881c20cc-53d4-4b0b-945e-85753c19b215' ObjectId: '0b157efa-8eca-41a8-a624-88d9f735a9e8'
2:03:49 PM - Deployment template C:\Users\LARRYO~1.RED\AppData\Local\Temp\test-resources.94748e36-e777-4928-b69a-d8d3378acecf.compiled.json from G:\Az\LarryO\azure-sdk-for-cpp\sdk\core\test-resources.bicep to resource group rg-larryocore
2:04:46 PM - Persist the following environment variables based on your detected shell (PowerShell):

${env:ANONYMOUSCONTAINERIPV4ADDRESS} = '20.245.175.58'
${env:AUTHENTICATEDCONTAINERIPV4ADDRESS} = '20.253.174.195'

```

## _Testing the proxies

To verify that the anonymous proxies work, you can execute the following command:

```powershell
sdk\core\azure-core\test\ut\proxy_tests\verify_proxy.ps1
```

This will verify the running proxy - if the environment variables set by `New-TestResources` are set, it will use that instance,
otherwise it will attempt to verify the proxy running on localhost.

The following sections describe how to manually test the proxy.

### _Anonymous Proxy

#### _CURL Access

To verify that the anonymous proxy is working using CURL, you can execute the following command. It instructs CURL to retrieve the contents using the proxy running on localhost at port 3128.

```powershell
curl -v https://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3128/
```

This should result in something similar to the following

```powershell
*   Trying 127.0.0.1:3128...
* Connected to 127.0.0.1 (127.0.0.1) port 3128 (#0)
* allocate connect buffer
* Establish HTTP proxy tunnel to azuresdkforcpp.azurewebsites.net:443
> CONNECT azuresdkforcpp.azurewebsites.net:443 HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net:443
> User-Agent: curl/7.83.1
> Proxy-Connection: Keep-Alive
>
< HTTP/1.1 200 Connection established
<
* Proxy replied 200 to CONNECT request
* CONNECT phase completed
* schannel: disabled automatic use of client certificate
* ALPN: offers http/1.1
* ALPN: server accepted http/1.1
> GET /get HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Content-Length: 917
< Content-Type: application/json
< Date: Thu, 04 Aug 2022 18:03:29 GMT
< Server: gunicorn/19.9.0
< Access-Control-Allow-Credentials: true
< Access-Control-Allow-Origin: *
<
{
  "args": {},
  "headers": {
    "Accept": "*/*",
    "Client-Ip": "71.197.167.179:49347",
    "Disguised-Host": "azuresdkforcpp.azurewebsites.net",
    "Host": "azuresdkforcpp.azurewebsites.net",
    "Max-Forwards": "10",
    "User-Agent": "curl/7.83.1",
    "Was-Default-Hostname": "azuresdkforcpp.azurewebsites.net",
    "X-Appservice-Proto": "https",
    "X-Arr-Log-Id": "02467793-daf0-4663-9a89-fd076e5d5c83",
    "X-Arr-Ssl": "2048|256|CN=Microsoft Azure TLS Issuing CA 01, O=Microsoft Corporation, C=US|CN=*.azurewebsites.net, O=Microsoft Corporation, L=Redmond, S=WA, C=US",
    "X-Client-Ip": "71.197.167.179",
    "X-Client-Port": "49347",
    "X-Forwarded-Tlsversion": "1.2",
    "X-Original-Url": "/get",
    "X-Site-Deployment-Id": "azuresdkforcpp",
    "X-Waws-Unencoded-Url": "/get"
  },
  "origin": "71.197.167.179:49347",
  "url": "https://azuresdkforcpp.azurewebsites.net/get"
}
* Connection #0 to host 127.0.0.1 left intact
```

#### _WinHTTP Access

To test accessing the anonymous proxy using WinHTTP, from a Windows machine, enter the following:

```powershell
Invoke-WebRequest -Uri https://azuresdkforcpp.azurewebsites.net/get -Proxy http://127.0.0.1:3128
```

This assumes that the Powershell `Invoke-WebRequest` command is built on the `System.Net.Http.HttpClient` object, which (on Windows) is backed by WinHTTP.

The `Invoke-WebRequest` command should return something like the following:

```powershell
StatusCode        : 200
StatusDescription : OK
Content           : {
                      "args": {},
                      "headers": {
                        "Client-Ip": "67.183.134.44:51480",
                        "Disguised-Host": "azuresdkforcpp.azurewebsites.net",
                        "Host": "azuresdkforcpp.azurewebsites.net",
                        "Max-Forwards":...
RawContent        : HTTP/1.1 200 OK
                    Date: Fri, 12 Aug 2022 21:36:52 GMT
                    Server: gunicorn/19.9.0
                    Access-Control-Allow-Credentials: true
                    Access-Control-Allow-Origin: *
                    Content-Length: 964
                    Content-Type: application/json
Headers           : {[Date, System.String[]], [Server, System.String[]], [Access-Control-Allow-Credentials,
                    System.String[]], [Access-Control-Allow-Origin, System.String[]]...}
Images            : {}
InputFields       : {}
Links             : {}
RawContentLength  : 964
RelationLink      : {}
```

### _Authenticated proxy

#### _CURL access

Invoke the following command:

```powershell
curl -U user:password -v http://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3129/
```

If the proxy is successfully connected, it should result in something like:

```powershell
*   Trying 127.0.0.1:3129...
* Connected to 127.0.0.1 (127.0.0.1) port 3129 (#0)
* Proxy auth using Basic with user 'user'
> GET http://azuresdkforcpp.azurewebsites.net/get HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net
> Proxy-Authorization: Basic dXNlcjpwYXNzd29yZA==
> User-Agent: curl/7.83.1
> Accept: */*
> Proxy-Connection: Keep-Alive
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Content-Length: 725
< Content-Type: application/json
< Date: Thu, 04 Aug 2022 19:20:04 GMT
< Server: gunicorn/19.9.0
< Access-Control-Allow-Credentials: true
< Access-Control-Allow-Origin: *
< X-Cache: MISS from 65d2e3c7997a
< X-Cache-Lookup: MISS from 65d2e3c7997a:3129
< Via: 1.1 65d2e3c7997a (squid/3.5.27)
< Connection: keep-alive
<
{
  "args": {},
  "headers": {
    "Accept": "*/*",
    "Cache-Control": "max-age=259200",
    "Client-Ip": "71.197.167.179:51795",
    "Disguised-Host": "azuresdkforcpp.azurewebsites.net",
    "Host": "azuresdkforcpp.azurewebsites.net",
    "Max-Forwards": "10",
    "User-Agent": "curl/7.83.1",
    "Was-Default-Hostname": "azuresdkforcpp.azurewebsites.net",
    "X-Arr-Log-Id": "7e68e5d2-15bc-4f73-9424-ec19b5bd7d15",
    "X-Client-Ip": "71.197.167.179",
    "X-Client-Port": "51795",
    "X-Original-Url": "/get",
    "X-Site-Deployment-Id": "azuresdkforcpp",
    "X-Waws-Unencoded-Url": "/get"
  },
  "origin": "172.17.0.1, 71.197.167.179:51795",
  "url": "http://azuresdkforcpp.azurewebsites.net/get"
}
* Connection #0 to host 127.0.0.1 left intact
```

If you attempt an unauthenticated connection to the server:

```bash
 curl -v http://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3129/
```

This should result in something like:

```bash
*   Trying 127.0.0.1:3129...
* Connected to 127.0.0.1 (127.0.0.1) port 3129 (#0)
> GET http://azuresdkforcpp.azurewebsites.net/get HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net
> User-Agent: curl/7.83.1
> Accept: */*
> Proxy-Connection: Keep-Alive
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 407 Proxy Authentication Required
< Server: squid/3.5.27
< Mime-Version: 1.0
< Date: Thu, 04 Aug 2022 19:19:52 GMT
< Content-Type: text/html;charset=utf-8
< Content-Length: 3635
< X-Squid-Error: ERR_CACHE_ACCESS_DENIED 0
< Vary: Accept-Language
< Content-Language: en
< Proxy-Authenticate: Basic realm="Squid proxy-caching web server"
< X-Cache: MISS from 65d2e3c7997a
< X-Cache-Lookup: NONE from 65d2e3c7997a:3129
< Via: 1.1 65d2e3c7997a (squid/3.5.27)
< Connection: keep-alive
```

Similarly, if you attempt the wrong username/password combination:

```bash
 curl -v http://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3129/
```

you should see:

```powershell
*   Trying 127.0.0.1:3129...
* Connected to 127.0.0.1 (127.0.0.1) port 3129 (#0)
* Proxy auth using Basic with user 'userx'
> GET http://azuresdkforcpp.azurewebsites.net/get HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net
> Proxy-Authorization: Basic dXNlcng6cGFzc3dvcmQ=
> User-Agent: curl/7.83.1
> Accept: */*
> Proxy-Connection: Keep-Alive
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 407 Proxy Authentication Required
< Server: squid/3.5.27
< Mime-Version: 1.0
< Date: Thu, 04 Aug 2022 19:19:57 GMT
< Content-Type: text/html;charset=utf-8
< Content-Length: 3737
< X-Squid-Error: ERR_CACHE_ACCESS_DENIED 0
< Vary: Accept-Language
< Content-Language: en
* Authentication problem. Ignoring this.
< Proxy-Authenticate: Basic realm="Squid proxy-caching web server"
< X-Cache: MISS from 65d2e3c7997a
< X-Cache-Lookup: NONE from 65d2e3c7997a:3129
< Via: 1.1 65d2e3c7997a (squid/3.5.27)
< Connection: keep-alive
```

Over HTTPS, the results are slightly different:
Correctly authenticated:

Request:

```powershell
curl -v https://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3129/ -U user:password
```

Response:

```powershell
*   Trying 127.0.0.1:3129...
* Connected to 127.0.0.1 (127.0.0.1) port 3129 (#0)
* allocate connect buffer
* Establish HTTP proxy tunnel to azuresdkforcpp.azurewebsites.net:443
* Proxy auth using Basic with user 'user'
> CONNECT azuresdkforcpp.azurewebsites.net:443 HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net:443
> Proxy-Authorization: Basic dXNlcjpwYXNzd29yZA==
> User-Agent: curl/7.83.1
> Proxy-Connection: Keep-Alive
>
< HTTP/1.1 200 Connection established
<
* Proxy replied 200 to CONNECT request
* CONNECT phase completed
* schannel: disabled automatic use of client certificate
* ALPN: offers http/1.1
* ALPN: server accepted http/1.1
> GET /get HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Content-Length: 917
< Content-Type: application/json
< Date: Thu, 04 Aug 2022 20:16:16 GMT
< Server: gunicorn/19.9.0
< Access-Control-Allow-Credentials: true
< Access-Control-Allow-Origin: *
<
{
  "args": {},
  "headers": {
    "Accept": "*/*",
    "Client-Ip": "71.197.167.179:52904",
    "Disguised-Host": "azuresdkforcpp.azurewebsites.net",
    "Host": "azuresdkforcpp.azurewebsites.net",
    "Max-Forwards": "10",
    "User-Agent": "curl/7.83.1",
    "Was-Default-Hostname": "azuresdkforcpp.azurewebsites.net",
    "X-Appservice-Proto": "https",
    "X-Arr-Log-Id": "ca2264b0-ff29-43a6-8297-ef8952270fac",
    "X-Arr-Ssl": "2048|256|CN=Microsoft Azure TLS Issuing CA 01, O=Microsoft Corporation, C=US|CN=*.azurewebsites.net, O=Microsoft Corporation, L=Redmond, S=WA, C=US",
    "X-Client-Ip": "71.197.167.179",
    "X-Client-Port": "52904",
    "X-Forwarded-Tlsversion": "1.2",
    "X-Original-Url": "/get",
    "X-Site-Deployment-Id": "azuresdkforcpp",
    "X-Waws-Unencoded-Url": "/get"
  },
  "origin": "71.197.167.179:52904",
  "url": "https://azuresdkforcpp.azurewebsites.net/get"
}
* Connection #0 to host 127.0.0.1 left intact
```

Unauthenticated:

Request:

```powershell
curl -v https://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3129/
```

Response:

```bash
*   Trying 127.0.0.1:3129...
* Connected to 127.0.0.1 (127.0.0.1) port 3129 (#0)
* allocate connect buffer
* Establish HTTP proxy tunnel to azuresdkforcpp.azurewebsites.net:443
> CONNECT azuresdkforcpp.azurewebsites.net:443 HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net:443
> User-Agent: curl/7.83.1
> Proxy-Connection: Keep-Alive
>
< HTTP/1.1 407 Proxy Authentication Required
< Server: squid/3.5.27
< Mime-Version: 1.0
< Date: Thu, 04 Aug 2022 20:15:02 GMT
< Content-Type: text/html;charset=utf-8
< Content-Length: 3616
< X-Squid-Error: ERR_CACHE_ACCESS_DENIED 0
< Vary: Accept-Language
< Content-Language: en
< Proxy-Authenticate: Basic realm="Squid proxy-caching web server"
< X-Cache: MISS from 65d2e3c7997a
< X-Cache-Lookup: NONE from 65d2e3c7997a:3129
< Via: 1.1 65d2e3c7997a (squid/3.5.27)
< Connection: keep-alive
<
* Ignore 3616 bytes of response-body
* Received HTTP code 407 from proxy after CONNECT
* CONNECT phase completed
* Closing connection 0
curl: (56) Received HTTP code 407 from proxy after CONNECT
```

Invalid Password:

```bash
curl -v https://azuresdkforcpp.azurewebsites.net/get -x http://127.0.0.1:3129/ -U user:passwordx
```

Response:

```sh
*   Trying 127.0.0.1:3129...
* Connected to 127.0.0.1 (127.0.0.1) port 3129 (#0)
* allocate connect buffer
* Establish HTTP proxy tunnel to azuresdkforcpp.azurewebsites.net:443
* Proxy auth using Basic with user 'user'
> CONNECT azuresdkforcpp.azurewebsites.net:443 HTTP/1.1
> Host: azuresdkforcpp.azurewebsites.net:443
> Proxy-Authorization: Basic dXNlcjpwYXNzd29yZHg=
> User-Agent: curl/7.83.1
> Proxy-Connection: Keep-Alive
>
< HTTP/1.1 407 Proxy Authentication Required
< Server: squid/3.5.27
< Mime-Version: 1.0
< Date: Thu, 04 Aug 2022 20:16:54 GMT
< Content-Type: text/html;charset=utf-8
< Content-Length: 3718
< X-Squid-Error: ERR_CACHE_ACCESS_DENIED 0
< Vary: Accept-Language
< Content-Language: en
< Proxy-Authenticate: Basic realm="Squid proxy-caching web server"
* Authentication problem. Ignoring this.
< X-Cache: MISS from 65d2e3c7997a
< X-Cache-Lookup: NONE from 65d2e3c7997a:3129
< Via: 1.1 65d2e3c7997a (squid/3.5.27)
< Connection: keep-alive
<
* Received HTTP code 407 from proxy after CONNECT
* CONNECT phase completed
* Closing connection 0
curl: (56) Received HTTP code 407 from proxy after CONNECT
```

#### _WinHTTP access

```powershell
Invoke-WebRequest -Uri http://azuresdkforcpp.azurewebsites.net/get -Proxy http://127.0.0.1:3129 -ProxyCredential user
```

HTTPS Connections:

Using `InvokeWebRequest`, this time with the proxy:

```powershell
Invoke-WebRequest -Uri https://azuresdkforcpp.azurewebsites.net/get -Proxy http://127.0.0.1:3129 -ProxyCredential user
```

you should get something like:

```powershell
StatusCode        : 200
StatusDescription : OK
Content           : {
                      "args": {},
                      "headers": {
                        "Client-Ip": "71.197.167.179:52995",
                        "Disguised-Host": "azuresdkforcpp.azurewebsites.net",
                        "Host": "azuresdkforcpp.azurewebsites.net",
                        "Max-Forwards"...
RawContent        : HTTP/1.1 200 OK
                    Date: Thu, 04 Aug 2022 20:19:13 GMT
                    Server: gunicorn/19.9.0
                    Access-Control-Allow-Credentials: true
                    Access-Control-Allow-Origin: *
                    Content-Length: 967
                    Content-Type: application/json
Headers           : {[Date, System.String[]], [Server, System.String[]], [Access-Control-Allow-Credentials,
                    System.String[]], [Access-Control-Allow-Origin, System.String[]]...}
Images            : {}
InputFields       : {}
Links             : {}
RawContentLength  : 967
RelationLink      : {}
```

For cases where authorization fails you should get something like:
Request:

```powershell
 Invoke-WebRequest -Uri http://azuresdkforcpp.azurewebsites.net/get -Proxy http://127.0.0.1:3129 -ProxyCredential userc
```

Response:

```powershell
Invoke-WebRequest:
ERROR: Cache Access Denied
ERROR
Cache Access Denied.
The following error was encountered while trying to retrieve the URL: http://azuresdkforcpp.azurewebsites.net/get
Cache Access Denied.
Sorry, you are not currently allowed to request http://azuresdkforcpp.azurewebsites.net/get from this cache until you have authenticated yourself.
Please contact the cache administrator if you have difficulties authenticating yourself.

Generated Thu, 04 Aug 2022 20:23:48 GMT by 65d2e3c7997a (squid/3.5.27)
```

And on HTTPS:
For cases where authorization fails you should get something like:
Request:

```powershell
 Invoke-WebRequest -Uri https://azuresdkforcpp.azurewebsites.net/get -Proxy http://127.0.0.1:3129 -ProxyCredential userc
```

Response:

```powershell

Invoke-WebRequest:
ERROR: Cache Access Denied
ERROR
Cache Access Denied.
The following error was encountered while trying to retrieve the URL: http://azuresdkforcpp.azurewebsites.net/get
Cache Access Denied.
Sorry, you are not currently allowed to request http://azuresdkforcpp.azurewebsites.net/get from this cache until you have authenticated yourself.
Please contact the cache administrator if you have difficulties authenticating yourself.

Generated Thu, 04 Aug 2022 20:23:48 GMT by 65d2e3c7997a (squid/3.5.27)
```

Result in:

```powershell
Invoke-WebRequest: The proxy tunnel request to proxy 'http://127.0.0.1:3129/' failed with status code '407'."
```
