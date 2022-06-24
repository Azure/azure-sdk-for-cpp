# Azure SDK for C++ - Libcurl Transport Adapter

## Azure SDK for C++

The [Azure SDK for C++](https://github.com/Azure/azure-sdk-for-cpp) enables an easy and consistent experience for consuming Azure services.
Azure services offer customers a simple way to perform many kinds of operations online. As such, the operations work as a client/server communication where the operations are requested by a client (customer) and executed by an Azure service over a network.

### HTTP

Most Azure services use the HTTP protocol for client/service communication. The protocol defines the structure of how a request and a response must look like.
 
Modern programming languages like C#, Java, and JS have an HTTP client as part of the language itself.  For these languages, the runtime library provides classes making HTTP easy to use.
 
However, for C++, the runtime library does not include any HTTP classes; you either need to write an HTTP implementation yourself or you could use some third-party library.  Keep in mind, that you also have to consider the OSes (Windows, Linux, Mac, etc.) you plan to run your application on. Libcurl, for example, is one of the most popular cross-OS libraries.

### Replaceable HTTP Transport

One of the more interesting features of the Azure SDK for C++ is that a customer has the ability to choose the HTTP implementation to be used to communicate with Azure services. That implementation is known by the SDK as an HTTP _transport adapter_. The azure-core library includes an HTTP namespace where it defines classes to abstract the HTTP components (request, response, send, etc.). While the Request and the Response are well defined based on the HTTP protocol [RFC](https://datatracker.ietf.org/doc/html/rfc7230), the send operation is left as a virtual method in the HttpTransport abstract class. Then, a transport adapter is a derived class that implements the Send operation. Learn more about HTTP transport adapter [here](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/HttpTransportAdapter.md#http-transport-adapter).

## Libcurl Transport Adapter

The azure-core-cpp library provides a transport adapter implemented with libcurl. The next paragraphs mentions some of the limitations of libcurl and the code that is part of the LibcurlTransportAdapter  (LTA) that satisfy the expectations for an SDK client.

### Easy handle

Let's start by talking about libcurl fundamentals. Libcurl is written in C, as such it is fast and practical. It provides two main modes of operation, the easy handle and the multi handle. The easy handle is the starting point for both. The multi handle is nothing but a group of easy handles.
 
An easy handle represents a network request to be performed. For a developer, the typical implementation flow is:

1.	Create an easy handle (as a raw pointer).
2.	Ask libcurl to init the handle. Libcurl handles memory allocation.
3.	Configure the handle according to what needs to be requested to service.
4.	Tell libcurl to take the easy handle and perform the request.
 
As simple as that, libcurl hides a ton of complexity from you. Libcurl asks the Operating System for a network socket and establishes a connection. It even takes care of securing the connection with TLS when requested. It sends the HTTP request to the service, and it receives the HTTP response. It sounds magical! However, it is not enough for Azure SDK's requirements.

### Azure SDK requirements

From all the operations supported by Azure services, there's one that can't be fully completed by following the libcurl typical flow mentioned before. It is the download operation from the Azure Storage service which requires something else. When the size of the downloaded item is bigger than the amount of memory in the system ( i.e. downloading 1Tb to an Android device with only 8GB of memory, or to a Windows PC with 32 GB of memory), there should be no complications for an SDK client. However, based on the typical libcurl flow, during step 4, libcurl blocks the application until all the bytes from the server have been received, and that's not acceptable from the SDK perspective.

#### Stream response on demand

The expectation when using the SDK client to download bytes is to enable an application to read any amount of data from the network, on demand. An application must be able to read any amount of data from the network, then, it must be able to do something with that data and decide when to read some more. 
 
A simple example to visualize this requirement is to think about a big download (say 1Tb) as a media file with a high quality (say a 4K movie). Watching a few seconds of the movie, from any device, should not require downloading the entire movie. Downloading a few seconds would be equivalent to reading a few hundred bytes, which are played from the device while some more seconds (data) are downloaded. Whatever is already watched can be overridden by some more content.

#### No intermediate buffer

Older versions of the Azure SDK have worked around the libcurl programming model by using an intermediate buffer to fetch the entire response from the service and let the program continue from step 4 (from the steps mentioned above). By doing this, the maximum download size gets limited to what the system memory can handle. Older SDK versions would support 2GB as the maximum download size.
 
Another approach is to be able to request data from the server starting at some specific offset. Then, downloading big content would be equivalent to sending one request after another, asking for data starting on the last downloaded data. However, this strategy adds unnecessary extra messaging.  Each request is parsed by the server to produce a response that is sent back to the client. The client also parses the response. These server and client parsing would be happening one after each other, so it is not efficient.

### Manually implementing HTTP protocol

As mentioned before, using libcurl properly means delegating TCP and HTTP entirely to libcurl. In a nutshell, libcurl works as a state machine. Calling [curl_easy_perform(handle)](https://curl.se/libcurl/c/curl_easy_perform.html) is the way to start the machine (step 4 mentioned above). A loop is started and won't be completed until the request (set it up in the handle) is completed. If the request represents a GET operation to download data, and the application is expecting to read the data coming for the server, the handle needs to be configured with a _delegated function_ (a callback) that libcurl will invoke as soon as it has data from the server. Every time the callback is invoked, libcurl will inform how big it is the chunk of data that has arrived from the server, and a pointer to the buffer where data can be read. The next flow would be an approximation summary of what is happening within libcurl while dispatching a GET request:
 
1.	Resolve DNS from request URL.
2.	Get a network socket from the OS.
3.	Establish TCP connection to the server.
4.	Secure connection with TLS when using HTTPS.
5.	Create an HTTP request from the handle and write it to the network socket.
6.	Wait for socket to be readable (means server has responded) and get an HTTP status line and headers.
7. If there's a payload body (data to be downloaded), a loop is started here as:
*  7.1 Read data from the network socket.
*  7.2 Invoke the application callback passing the data read from the socket.
* 7.3 Repeat until the entire response has been received.
 
This flow demonstrates how using libcurl to download data requires developers to use a callback pattern. For the example of reproducing a media file (mentioned above), playing media must happen as part of the callback that is invoked by libcurl, which is very unfortunate. 
 
There are valid alternatives for applications, but it might be complex approaches. For example, using a multi-threading strategy, an application can call curl_easy_perform from one thread and use the read callback to copy the data from the response into a memory buffer. Then, another thread can play the media from the buffer. This strategy requires threads synchronization and complicates the debugging experience.
 
#### Speaking HTTP

Libcurl supports not only the HTTP protocol. It can also be used to speak other protocols. What this means, in short, is that, for any supported protocol, libcurl will translate the configuration from the handle into the specifics of the protocol in a way that a user doesn't need to learn those specific details from the protocol. However, there is an alternative that libcurl offers to speak a custom protocol.  It is indeed a poor man's solution because it is now the customer who will be writing and reading messages to a server. Referring to the seven steps flow mentioned before, about how libcurl dispatches a request, when the handle is set it up for custom protocol, it would be reduced to:
 
1.	Resolve DNS from request URL.
2.	Get a network socket from the OS.
3.	Stablish TCP connection to the server.
4.	Secure connection with TLS when using HTTPS.
 
So, only the first four steps are managed by libcurl. A connection channel is established to the server, but no message is sent. Libcurl returns from curl_easy_perform leaving the connection ready for users to start calling [curl_easy_send](https://curl.se/libcurl/c/curl_easy_send.html) and [curl_easy_recv](https://curl.se/libcurl/c/curl_easy_recv.html). 
 
Most of the libcurl handle configuration becomes useless when the handle is changed to custom protocol. For example, it becomes irrelevant creating a headers list, or setting an HTTP method (GET, POST, etc.). Libcurl will ignore it, and it will be now the customer who takes connection ownership.  What this means for the libcurlTransportAdapter is that it has to learn how to speak raw HTTP and translate an Azure:: Core:: Http:: Request into a pure HTTP call.

#### Receiving raw HTTP

The most tedious task, while speaking raw HTTP, is reading and parsing responses. As mentioned before, the main purpose of using the poor man's substitute (custom protocol) to re-implement HTTP (even though libcurl provides HTTP support natively) is to enable an Azure SDK client to start a download operation and let customers to decide when to pull chunks of data from the response (directly from the network socket, without any intermediate buffers or using callback functions).
 
An HTTP request is nothing but a string with a specific format, it has special text delimiters to distinguish between the request line, headers and body. As soon as the server receives and parses the request, it would trigger an operation and eventually produce a response. The response needs to be HTTP formatted using text delimiters to indicate where the headers section starts and where it ends. Then the server will start sending small parts of it. On the client side, calling _curl_easy_recv()_ will get any bytes that have arrived. As soon as the function pulls the bytes from the socket, the socket will receive more data that will be written on top of the previous data. The function _curl_easy_recv()_ is a way to tell the OS, _“Hey, I got this data, you can write on top of it now"_.
 
Based on how _curl_easy_recv()_ behaves, and what the Azure SDK client is expecting, the libculTransportAdapter must keep calling _curl_easy_recv()_ until it finds the text delimiter that indicates the start of the HTTP body. When calling _curl_easy_recv()_, one of its input parameters is how many bytes to read from the socket. However, if there are not as many bytes in the socket at requested, the function will just get as much as it can. For example, if 1kb is requested to be pulled from the socket, the actual returned bytes might be 1kb (if that amount was ready in the socket) or less (if there were only 1023 bytes or less in the socket). This brings an interesting design question, how many bytes should the transport adapter request on every call to _curl_easy_recv()_?
 
On one side, the minimum number of bytes to pull from the socket would be one. It is, however, inefficient to read one byte at a time from the socket. On the other hand, trying to pull a big number of bytes might slow down things, as the bytes from the socket are copied to another buffer. As a side note, for a secured connection (TLS), messages are encrypted and sent through the network in chucks of typically 16k. The libcurl transport adapter is currently using 1kb as the size for puling data from the socket. By calling _curl_easy_recv()_ it is still using libcurl to handle the secure connection and decrypt message if necessary, so the bytes copied from the socket are a readable string.
 
It might happen that during the first call to _curl_easy_recv()_, the entire response from the server is downloaded within the 1kb. For example, a server response with no body payload and just a one header would look like:
 

> HTTP 1.0 200 OK\r\nheader:value\r\n\r\n

 
All it takes is 39 bytes for this response. But it could also be the case that within the first request for 1kb, the response would look like:

> HTTP 1.0 200 OK

or even only:

> HTTP

or even just:

> H

 
It is uncertain how many bytes are returned when calling _curl_easy_recv()_. The libcurl transport adapter uses a 1kb buffer (called InternalReadBuffer or IRB) and a parser component (called ResponseBufferParser or RBP). The IRB is filled by calling _curl_easy_recv()_, then the RBP parses the content in IRB. It accumulates bytes until an HTTP component can be created (like the HTTP status line or headers). It holds an internal state to indicate when the HTTP body has been detected. In summary, the RBP creates the Azure:: Core:: Http:: RawResponse as soon as the status line is accumulated. Then it sets http headers for the raw response. Finally, it will tell, if part of the response body has been copied from the socket to the IRB, which can happen if the last call to _curl_easy_recv()_, copies 1kb of data to the IRB, and contains the end of the headers plus the first bytes of the body.
 
Based on the results produced by the RBP, the libcurl transport adapter returns an Azure:: Core:: IO:: BodyStream inside the Azure:: Core:: Http:: RawResponse. If part of the response body is within the IRB, calling _Read()_ from the body stream would take the bytes from the IRB first. Once all the bytes from the IRB are read, _Read()_ invokes _curl_easy_recv()_, reading bytes directly from the socket network to a customer's buffer. The connection lives inside the body stream, which is part of the HttpRawResponse. If either the body stream or the raw response goes out scope, the connection to the server can be either, re-used, or terminated (see below for [Connection pool](#connection-pool)).
 
There are a few more variables to take into consideration. The HTTP protocol defines three ways of how a server can return a response to a client. The most popular way is when there is a header that indicates the size of the body to be downloaded by the client (content-length). A second, least popular but also used by Azure services is when, instead of mentioning the size of the entire body, the server will start sending chucks of data. Each chunk will start by saying what's the size of the chunk, followed by the data. The server will send a chunk of size zero to signal the end of the payload. And the last option is when the server will ask the client to keep pulling for data until the connection is closed. The libcurl transport adapter has learned how to read each type of response and abstracts those details away by returning a body stream which would behave the same for any type of response. 

#### Sending raw HTTP

Calling _curl_easy_send()_ is a similar experience. It takes an input parameter which defines how much data to put into the network socket, and it returns how many bytes it was able to send. The LTA (libcurl transport adapter) implements an upload operation by calling _curl_easy_send()_ sequentially until all bytes are sent.
The hardest thing to deal with, during an upload operation, is the speed that it takes the operating system to send bytes through the network. The OS sends bytes to the server and lets the socket in a state where it can take more data to be sent. The call to _curl_easy_send()_ returns as soon as the data is written to the socket, so, immediately calling the function again to send more data will most likely cause function to find the socket busy, because the OS is performing the I/O operation. This is another unfortunate side effect of using libcurl with custom protocol. By using the proper way, libcurl abstracts away all the operations between the application and the operating system sockets.
When working on Unix systems (Linux and macOS), the LTA needs to import `sys/socket.h` . And when running for Windows, `winsock2h.h` is imported. These headers are used to check for socket updates, especially to poll for state change. That’s how the LTA knows when a socket is ready to be used.
On the eyes of a customer, the LTA should provide the same level of experience, regardless of what libcurl mode it uses. If it is using custom protocol or proper way to consume libcurl, there should be no distinguish. This brings a complex challenge for the LTA. In terms of maintainability, libcurl will fix and maintain the code to support HTTP only for the proper mode. By using custom-protocol, LTA is manually implementing and supporting things that might have been already fixed in the past by the libcurl community. An example of a scenario like this can be found within the LTA Windows implementation, where _setsockopt()_ method must be called before writing to a socket to ensure the uploading chunk size of data is not reduced by Windows. The LTA goes as deep as the TCP implementation details to provide this patch. The side effect if this patch is not applied makes the LTA complete an upload operation __four times slower__ than using libcurl proper mode. And the adventure for discovering this patch requires running the proper mode, step by step (debugger), identifying the call stack deltas against the custom protocol mode. Or browsing the git commit history from libcurl, looking for tags like windows, performance, socket and/or uploading data. Trying to isolate related changes.

### Connection pool

There’s yet one more unfortunate detail for using libcurl with custom protocol, and that’s supporting the HTTP `keep alive` feature. Keep alive feature provides a huge performance improvement when using a secured connection (TLS). When a connection uses TLS to secure the data, there’s a time-consuming process where the server and the client both authenticate themselves and interchange valid certificates. This process starts with a handshake and completes when both server and client agree on how to encode and decode messages between them. Depending on the network speed, the entire process could take up to two seconds. If the request and response interchanged in the secured channel is just a few bytes, securing the connection could take more time than the actual data transferring. The keep alive feature is a mechanism to re-use a connection channel. In the case of a secured channel, an application can use the same connection to send requests, one after another, saving time and resources of securing a new connection for each request.

Libcurl can automatically support the keep alive feature when using the proper mode. When _curl_easy_init(handle)_ is called, libcurl first check if there is already an open connection for the request described in the handle. Once the request is performed, libcurl will keep the connection open for some time in case there’s a new request for the same server. Nevertheless, if a libcurl handle is configured for custom protocol, libcurl does not provide a keep alive feature.

The keep alive feature for libcurl is provided by the Azure SDK library directly. The library provides three main components to ensure keep alive connections for libcurl. The first one is the libcurl connection class, which is a wrapper for a libcurl handle. The second component is the pool of libcurl connections, which is a collection where connections can go in and out to be re-used. And the last component is a pool cleaner agent, which removes old connections from the pool. More details about each component are in the next paragraphs.

#### Libcurl Connection

Whenever a Libcurl Transport Adapter (LTA) is created, it is assigned with a libcurl connection. The connection contains a libcurl handle and the connection options. It offers an API for the LTA to read and write bytes to a socket. The connection defines an expiration time of 60 seconds. If a connection is not used within the expiration time, it is discarded from the pool. A server would typically close an HTTP connection after some time of inactivity.

The connections options describe elements like proxy, url, port, ca cert, etc. These options are used to let a connection be re-used only when the configuration is the same as the one used before for another connection.
The libcurl connection is typically wrapped within another container. In the case of the LTA, the connection is wrapped within a body stream, which becomes the connection owner. The connection owner is responsible for returning the connection to the pool when it is no longer required.

#### Connection Pool

The connection pool is a global and static singleton object. The pool provides an API to request a connection. Based on the set of options requested, the pool will search if there´s a connection which is not expired and was created with the same options. If one connection is found, it is moved out of the pool. When a libcurl connection owner (like the body stream from the LTA) goes out of scope, its destructor asks the pool to move the connection back to the pool.  The connection needs to satisfy some requirements before it comes back to the pool, such as:

-	The last message from the server is not an error message. A server would typically close a connection when the HTTP status code returned to the client is not within the two hundred codes. 
-	The connection is in a state where there are still bytes from the server response to be read from the socket. A connection should read an entire response before it can send a new request.
-	When the connection has been inactive and out of the pool for a long time, most likely it’s shut down and it won’t be moved back to the pool.

The connection pool contains a `mutex` for moving connections in and out of the pool. Multiple threads can request or return a libcurl connection at the same time. The pool uses a Last-in-First-out mechanism to move connections, making it certain to re-use the connections with less time seating in the pool.

The pool is indexed by the host name from the connection URL, plus a connection hash key calculated from the options. Within each index key, the pool can host up to a maximum of 1024 connections. If the index is full, the oldest connection gets removed before returning another connection to the pool.
The connection pool is responsible for creating and initializing a libcurl handle. The handle is configured to use custom protocol mode and with the connection options provided by a customer.

#### Pool Cleaner Agent

Whenever a libcurl connection is returned to the pool, the Pool Cleaner Agent (PCA) is started (if it is not already running). PCA is a thread which runs every 90 seconds while there are connections seating in the pool. Every time PCA runs, it checks connections, starting from the oldest connection returned to the pool. Since the connection pool works as one stack, the oldest connection is at the bottom. PCA will remove the connections which have expired. As soon as it finds a non-expired connection, PCA switch to inspect another index, until all indexes are reviewed.

PCA goes to sleep for 90 seconds only if a non-expired connection was found. If all connections were expired and removed, PCA is terminated and won’t be started again until a connection is moved to the pool.
PCA prevents applications from keeping expired connections objects in memory. A common scenario where the importance of the PCA can be observed is if the application needs to create a hundred connections and use them all at the same time. Eventually, all the connections would return to the pool and if the Azure SDK client is not used anymore by the application, there would be a hundred expired connections in heap memory until the application ends. 
