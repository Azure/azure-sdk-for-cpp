#!/usr/bin/env python3
import asyncio
import websockets
from websockets.server import serve
import threading
from urllib.parse import urlparse, parse_qs

client_count = 0
echo_count_lock = threading.Lock()
stop = None

async def handleEcho(websocket, path):
    """Handle echo requests"""
    try:
        parsed_url = urlparse(path)
        query_values = parse_qs(parsed_url.query)
        
        async for data in websocket:
            print(f"Echo received: {data}")
            
            # Handle fragmentation if requested
            if 'fragment' in query_values and query_values['fragment'] == ['true']:
                words = data.split()
                for word in words:
                    await websocket.send(word)
            else:
                await websocket.send(data)
                
    except websockets.exceptions.ConnectionClosed:
        print("Echo connection closed")
    except Exception as e:
        print(f"Echo handler error: {e}")

async def handler(websocket, path):
    """Main WebSocket handler"""
    global client_count, stop
    
    print(f"WebSocket connection to path: {path}")
    
    try:
        parsed_path = urlparse(path).path
        
        if parsed_path == '/openclosetest':
            print("Open/Close Test")
            try:
                data = await websocket.recv()
                print(f"OpenCloseTest received: {data}")
            except websockets.ConnectionClosed as ex:
                print(f"OpenCloseTest connection closed: {ex}")
            return
            
        elif parsed_path == '/echotest':
            with echo_count_lock:
                client_count += 1
                print(f"Echo test client count: {client_count}")
            await handleEcho(websocket, path)
            
        elif parsed_path == '/closeduringecho':
            data = await websocket.recv()
            print(f"Close during echo, received: {data}")
            await websocket.close(1001, 'closed')
            
        elif parsed_path == '/terminateserver':
            print("Terminating WebSocket server")
            if stop:
                stop.set_result(0)
            return
            
        else:
            # Default handler
            data = await websocket.recv() 
            print(f"Default handler received: {data}")
            reply = f"Data received as: {data}!"
            await websocket.send(reply)
            
    except websockets.exceptions.ConnectionClosed as ex:
        print(f"Connection closed: {ex}")
    except Exception as e:
        print(f"Handler error: {e}")
        import traceback
        traceback.print_exc()

async def main():
    """Start the WebSocket server"""
    global stop
    
    print("Starting WebSocket server on localhost:8000")
    
    async with serve(handler, "localhost", 8000, ping_interval=7):
        loop = asyncio.get_running_loop()
        stop = loop.create_future()
        print("Server ready for connections")
        await stop

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Server stopped by user")
    print("Server ended")