from array import array
import asyncio
from operator import length_hint
from time import sleep
from urllib.parse import urlparse
 
import websockets
 
# create handler for each connection
customPaths = {}

async def handleControlPath(websocket):
    while (1):
        data : str = await websocket.recv()
        parsedCommand = data.split(' ')
        if (parsedCommand[0] == "close"):
            print("Closing control channel")
            await websocket.send("ok")
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
        websocket

		
async def handleCustomPath(websocket, path:dict):
    print("Handle custom path", path)
    data : str = await websocket.recv()
    print("Received ", data)
    if ("delay" in path.keys()):
        sleep(path["delay"])
    print("Responding")
    await websocket.send(data)
    await websocket.close()

    async def handleEcho(websocket, url):
    while  websocket.open:
        try:
            data = await websocket.recv()
#            print(f'Echo ', len(data),' bytes')
            await websocket.send(data)
        except websockets.ConnectionClosedOK:
            print("Connection closed ok.")
            return
        except websockets.ConnectionClosed as ex:
            print(f"Connection closed exception: {ex.rcvd.code} {ex.rcvd.reason}")
            return
			 
async def handler(websocket, path : str):
    print("Socket handler: ", path)
    parsedUrl = urlparse(path)
    if (parsedUrl.path == '/openclosetest'):
        print("Open/Close Test")
        try:
            data = await websocket.recv()
        except websockets.ConnectionClosedOK:
            print("Connection closed ok.")
        except websockets.ConnectionClosed as ex:
            print(f"Connection closed exception: {ex.rcvd.code} {ex.rcvd.reason}")
        return
    elif (parsedUrl.path == '/echotest'):
        await handleEcho(websocket, parsedUrl)
    elif (parsedUrl.path == '/closeduringecho'):
        data = await websocket.recv()
        await websocket.close(1001, 'closed')
    elif (parsedUrl.path =='control'):
        await handleControlPath(websocket)
    elif (parsedUrl.path in customPaths.keys()):
        print("Found path ", path, "in control paths.")
        await handleCustomPath(websocket, customPaths[path])
    else:
        data = await websocket.recv()
        print("Received: ", data)
    
        reply = f"Data received as:  {data}!"
        await websocket.send(reply)
  
async def main():
    print("Starting server")
    async with websockets.serve(handler, "localhost", 8000):
        await asyncio.Future() # run forever.
 
if __name__=="__main__":
    asyncio.run(main())
    print("Ending server")
