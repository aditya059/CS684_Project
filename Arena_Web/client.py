from websocket import create_connection
ws = create_connection("ws://127.0.0.1:8082")
print("Sending 'Hello, World'...")
ws.send("Hello, World")
print("Sent")
print("Receiving...")
result =  ws.recv()
print("Received '%s'" % result)
ws.close()