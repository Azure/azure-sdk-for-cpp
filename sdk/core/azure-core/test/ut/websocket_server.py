from array import array
import asyncio
#from curses import has_key
from operator import length_hint
import threading
from time import sleep
from urllib.parse import ParseResult, urlparse
 
import websockets
 
# create handler for each connection
customPaths = {}
stop = False

async def handleControlPath(websocket):
    while (1):
        data : str = await websocket.recv()
        parsedCommand = data.split(' ')
        if (parsedCommand[0] == "close"):
            print("Closing control channel")
            await websocket.send("ok")
            print("Terminating WebSocket server.")
            stop.set_result(0)
            break
        elif parsedCommand[0] == "newPath":
            print("Add path")
            newPath = parsedCommand[1]
            print(" Add path ", newPath)
            customPaths[newPath] = {"path": newPath,  "delay": int(parsedCommand[2]) }
            await websocket.send("ok")
        else:
            print("Unknown command, echoing it.")
            await websocket.send(data)

async def handleCustomPath(websocket, path:dict):
    print("Handle custom path", path)
    data : str = await websocket.recv()
    print("Received ", data)
    if ("delay" in path.keys()):
        sleep(path["delay"])
    print("Responding")
    await websocket.send(data)
    await websocket.close()

def HexEncode(data: bytes)->str:
    rv=""
    for val in data:
        rv+= '{:02X}'.format(val)
    return rv

def ParseQuery(url : ParseResult) -> dict:
    rv={}
    if len(url.query)!=0:
        args = url.query.split('&')
        for arg in args:
            vals=arg.split('=')
            rv[vals[0]]=vals[1]
    return rv

echo_count_lock = threading.Lock()
echo_count_recv = 0
echo_count_send = 0
client_count = 0
async def handleEcho(websocket, url:ParseResult):
    global client_count
    global echo_count_recv
    global echo_count_send
    global echo_count_lock
    queryValues = ParseQuery(url)
    while  websocket.open:
        try:
            data = await websocket.recv()
            with echo_count_lock:
                echo_count_recv+=1
            if 'delay' in queryValues:
                print(f"sleeping for {queryValues['delay']} seconds")
                await asyncio.sleep(float(queryValues['delay']))
                print("woken up.")

            if 'fragment' in queryValues and queryValues['fragment']=='true':
                await websocket.send(data.split())
            else:
                await websocket.send(data)
            with echo_count_lock:
                echo_count_send+=1
        except websockets.ConnectionClosedOK:
            print("Connection closed ok.")
            with echo_count_lock:
                client_count -= 1
                print(f"Echo count: {echo_count_recv}, {echo_count_send} client_count {client_count}")
                if client_count == 0:
                    echo_count_send = 0
                    echo_count_recv = 0
            return
        except websockets.ConnectionClosed as ex:
            if (ex.rcvd):
                print(f"Connection closed exception: {ex.rcvd.code} {ex.rcvd.reason}")
            else:
                print(f"Connection closed. No close information.")
            with echo_count_lock:
                client_count -= 1
                print(f"Echo count: recv: {echo_count_recv}, send: {echo_count_send} client_count {client_count}")
                if client_count == 0:
                    echo_count_send = 0
                    echo_count_recv = 0
            return
			 
async def handler(websocket, path : str):
    global client_count
    print("Socket handler: ", path)
    parsedUrl = urlparse(path)
    if (parsedUrl.path == '/openclosetest'):
        print("Open/Close Test")
        try:
            data = await websocket.recv()
            print(f"OpenCloseTest: Received {data}")
        except websockets.ConnectionClosedOK:
            print("OpenCloseTest: Connection closed ok.")
        except websockets.ConnectionClosed as ex:
            print(f"OpenCloseTest: Connection closed exception: {ex.rcvd.code} {ex.rcvd.reason}")
        return
    elif (parsedUrl.path == '/echotest'):
        with echo_count_lock:
            client_count+= 1
        await handleEcho(websocket, parsedUrl)
    elif (parsedUrl.path == '/closeduringecho'):
        data = await websocket.recv()
        await websocket.close(1001, 'closed')
    elif (parsedUrl.path =='/control'):
        await handleControlPath(websocket)
    elif (parsedUrl.path in customPaths.keys()):
        print("Found path ", path, "in control paths.")
        await handleCustomPath(websocket, customPaths[path])
    elif (parsedUrl.path == '/terminateserver'):
        print("Terminating WebSocket server.")
        stop.set_result(0)
    else:
        data = await websocket.recv()
        print("Received: ", data)
    
        reply = f"Data received as:  {data}!"
        await websocket.send(reply)
  
async def main():
    global stop
    print("Starting server")
    loop = asyncio.get_running_loop()
    stop = loop.create_future()
    async with websockets.serve(handler, "localhost", 8000, ping_interval=7):
        await stop # run forever.
 
if __name__=="__main__":
    asyncio.run(main())
    print("Ending server")
