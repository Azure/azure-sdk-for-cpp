from array import array
import asyncio
from time import sleep
 
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



async def handler(websocket, path : str):
    print("Socket handler: ", path)
    if (path == '/openclosetest'):
        print("Open/Close Test")
        try:
            data = await websocket.recv()
        except websockets.ConnectionClosedOK:
            print("Connection closed ok.")
        except websockets.ConnectionClosed as ex:
            print(f"Connection closed exception: {ex.rcvd.code} {ex.rcvd.reason}")
        return
    elif (path == '/echotest'):
        data = await websocket.recv()
        await websocket.send(data)
    elif (path == '/closeduringecho'):
        data = await websocket.recv()
        await websocket.close(1001, 'closed')
    elif (path=='control'):
        await handleControlPath(websocket)
    elif (path in customPaths.keys()):
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
