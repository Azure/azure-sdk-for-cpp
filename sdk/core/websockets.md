# Websockets

Azure core should provide support for SDK client to use websocker protocol to comunicate through the network.

HTTP support is offered by exposing an abstraction called `the pipeline` which offers extensibility and customization for SDK services and final customers. The pipeline, however, is a unidirectional comunication strategy from a client to some Azure service. This means that the server can't never talk to a client without the client starting first the conversation.

When it comes to websockets [RFC 6455](https://datatracker.ietf.org/doc/html/rfc6455), a client must be able to create a communication channel to a server (handshake), and use that channel for bidirectional communication. The client can receive messages from the server at any moment while the channel is not interrupted or closed.

## Requirements

- Connect to server (ws:// , http://)
- Send message on-request
- Receive message event
- TLS ( openSSL, schannel, UWP?)
- One operation per thread

### Use cases
- SDK client provides operations implemented with Websockets, such a voice recognizer, or a conversation administration.
- websocket client is re-used by sdk client's descendents ?  ( similar to pipeline). Avoid one connection per client?
- replaceble websocket stack. Similar to the http transport adapter. SDK client options would take a `std::shared_ptr<WebsocketTransport>`
- WebsocketTransportOptions specific to each adapter.

## API  - `Customer code`

Base abstraction is a `WebsocketTransport` which defines the azure-core websocket API.
Azure-core would provide a Windows and a non-windows `WebsocketTransportAdapter`
Customers can implement its own adapter.

### Track 1

Samples from the [track1](https://github.com/Azure-Samples/cognitive-services-speech-sdk/tree/master/quickstart/cpp)

```cpp

    auto config = SpeechConfig::FromSubscription("YourSubscriptionKey", "YourServiceRegion");
    
    // Creates a speech recognizer.
    auto recognizer = SpeechRecognizer::FromConfig(config);
    cout << "Say something...\n";

    // Starts speech recognition, and returns after a single utterance is recognized. The end of a
    // single utterance is determined by listening for silence at the end or until a maximum of 15
    // seconds of audio is processed.  The task returns the recognition text as result. 
    // Note: Since RecognizeOnceAsync() returns only a single utterance, it is suitable only for single
    // shot recognition like command or query. 
    // For long-running multi-utterance recognition, use StartContinuousRecognitionAsync() instead.
    auto result = recognizer->RecognizeOnceAsync().get();

    // Checks result.
    if (result->Reason == ResultReason::RecognizedSpeech)
    {
        cout << "We recognized: " << result->Text << std::endl;
    }
    else if (result->Reason == ResultReason::NoMatch)
    {
        cout << "NOMATCH: Speech could not be recognized." << std::endl;
    }
    else if (result->Reason == ResultReason::Canceled)
    {
        // handle
    }
```

### Track 2 ( Not confirmed )

```cpp
    // Creating the client
    ServiceClientOptions options;
    options.WebsocketTransport.Transport = std::make_shared<WebsocketAdapter>(WebsocketAdapterOptions);
    // Transport allow `events subscription`, passing a callback and a void* to some data.
    UserModel model; // customer data to be hook to the callback
    options.WebsocketTransport.Transport->SubscribeOnMessage(
        [](AzureSDKWebsocketMessage message, void* data)->void{ 
         },
        static_cast<void*>(&model)); 
    
    ServiceClient client = ServiceClient::FromSubscription("YourSubscriptionKey", "YourServiceRegion", options);
    
    // XXXXXResponse is equivalent to Azure::Core::Response<T>
    // Contains raw messages and the ServiceResponseModel
    Azure::Core::XXXXXResponse<ServiceResponseModel> result = client.Foo();
    
    // any mensages that came from the service after or during `Foo`.
    // Think about `Foo` as an operation that start listening to a microphone and waits until a long silence window to stop
    // Or as an operation that will keep listening to a microphone until some key is pressed from the keyboard.
    // While the operation is running, all messages comming from the service are accumulated in a list as the rawResponse
    std::vector<AzureSDKWebsocketMessage> messages(result.RawMessages); // similar to rawResponse
    // Service would add any models they need
    auto ServiceModel = result.ServiceParsedModel;
```

### SDK Service - `Convenience+Protoccol layer`

#### Init client

```cpp
    class ServiceClient {
        private:
        shared_ptr<WebsocketTransport> m_websocketClient;
        Azure::Core::Url m_url;
        Credential m_credential;
        std::function<void(WebsocketMessage message, void* data)> m_onReceivedMessage;

        public:
        // Constructor uses the transport from the options for the client.
        // ServiceClientOptions.WebsocketTransport has a default value provided by azure-core
        ServiceClient(
            std::string url,
            Credential credential,
            ServiceClient options) : m_websocketClient(options.WebsocketTransport.Transport),
                m_url(url), m_credential(credential)
            {
                // client subscribe to on message event
                // As soon as connection is open / ready, client will start receiving server messages.
                m_websocketClient->SubscribeOnMessage(m_onReceivedMessage, &this);
            }

        Azure::Core::BidirectionalResponse<ServiceResponseModelA> RecognizeOnce() {
            ServiceResponseModelA returnModel;
            m_onReceivedMessage = [](WebsocketMessage message, void* data){
                
            };

            m_websocketClient->Connect();
            
            while(something) { // listening.
                // Size of datagra is defined by service
                m_websocketClient->Send(datagram, 0); // block - non final
            }
            
            m_websocketClient->Send(datagram, 1);  // last datagram
            m_websocketClient->doEvents();
            m_websocketClient->close();

            return Azure::Core::BidirectionalResponse<ServiceResponseModelA>(returnModel);
        }

        Azure::Core::BidirectionalResponse<ServiceResponseModelB> Foo2() {
            
        }
    };

```

### SDK Internal - `API Core layer`

#### Azure core

```cpp

    // to be included by the SDK client options as `WebsocketTransport`
    struct WebsocketTransportOptions {
        // impl specific
    };

    struct WebsocketMessage {
        // impl specific
    };

    class WebsocketTransport {
        private:
        // **************** PAL ***********************
        void SendImpl(WebsocketMessage message) = 0;
        void Connect(Azure::Core::Url url) = 0;
        void Close() = 0;
        void OnMessageReceive() = 0;


        struct Subscriber {
            std::function<void(WebsocketMessage message)> Handler;
            void* data;
        }
        // 
        std::vector<Subscriber> m_subscribers;

        public:
        WebsocketTransport(WebsocketTransportOptions options)

        // control the list of handlers
        void SubscribeOnMessage(
            std::function<void* buffer, int64_t bufferSize, void* userData)> handler,
            void* userData) {
            m_subscriber.emplace_back(Subscriber(handler, data));
        }

        Read()

        // 
        void Connect()
        void Send(void* data, int64_t size, Azure::Context context) {
            return SendImpl(message);
        }
    };

    class LibWebSocketTransportOptions {

    };
    class LibWebSocketTransport : public WebsocketTransport {
    };

    LibWebSocketTransport instance;
    instance.Send();
    


```


## Implementation

### multi-thread
?

### Namespace

    - WebSockets namespace within Azure::Core as a sibling of Azure::Core::Http
    - WebSockets namespace within Azure::Core::Http

### Header and folder

    - sdk/core/azure-core/inc/azure/core/websockets
    - sdk/core/azure-core/inc/azure/core/http/websockets

### Classes