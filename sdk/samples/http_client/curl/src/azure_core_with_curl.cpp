// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 *
 */

#include <http/http.hpp>
#include <http/http_client.hpp>
#include <http/curl/curl.hpp>

#include <iostream>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

/*
int CustomerCode()
{

    auto myFactory = MyCustomHttpClientFactory();

    azure_core_SetHttpClientFactory(myClientFactory);

    auto storageClient = new StorageClient();
    storageClient.CreateBlob("blobname", optionalStruct);

}
*/

//class StorageClient {

    //class Uri {};
    //class Credentials {};

    //private:
        //Uri m_uri;
        /* const */ //HttpPolicy policies[];

    //public:
    //StorageClient(Uri Endpoint, Credentials credential, StorageClientOptions options) {
    
        //Pass transport the transport policy constructor
        //  transport = options.transport;
        //CreatePolicy array
      //auto transport = std::make_unique<Http::HttpTransport>();
      //auto retry = std::make_unique<Http::RetryPolicy>(std::move(transport));
      //auto pipeline = std::make_unique<Http::RequestIdPolicy>(std::move(retry));

      /*
            Customer can new Policy() and reuse
            StorageClientOptions
                PerRequest
                PerRetry
                TransportPolicy(transport at creation time)
      */

    //}

    //Response SomeOperation(/*Mandatory parameters*/ int x, SomeOperationOptions optionsWithOptionalContext)
    //{
      //Request request = Request();


      //policies[0].Process(new Context(), request);
      /*
        policyA , policy B.... transport (transport*)
      */
    //}

//};

//void MyApplication() {

    //Doesn't wnat our Http
    //  Defines a class MyHttpTransport: public HttpTransport();
    //  Defines a class MyHttpTransportFactory: public TransportFactory();
    //  Azure::Core::SetHttpTransport(new MyTransportFactory());

    //HttpTransport* mytransport = new HttpTransport();
    //StorageClientOptions options = {HttpTransport = mytransport};

    //StorageClient* client = new StorageClient(/* */, options);

    //Response response = client->SomeOperation(...);

    // DefaultTransport()

//}


//StorageClient
//StorageClient StorageClient(Uri Endpoint, Credentials credential, StorageClientOptions optionalWithContextInside )

int main()
{
  string host("https://httpbin.org/get");
  cout << "testing curl from transport" << endl << "Host: " << host << endl;

  try
  {
    auto request = Http::Request(Http::HttpMethod::Get, host);

    auto httpClientOptions = Http::HttpClientOptions();

    httpClientOptions.Transport = std::move(std::unique_ptr<CurlTransport>(new Http::CurlTransport()));
    auto httpClient = Http::HttpClient(httpClientOptions);

    auto context = Context();
    auto response = httpClient.Send(context, request);

    cout << response.getReasonPhrase();
  }
  catch (Http::CouldNotResolveHostException& e)
  {
    cout << e.what() << endl;
  }
  catch (Http::TransportException& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}
