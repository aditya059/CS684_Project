import flask
import threading
import socket

data = ""

# start the server
def server():
    app = flask.Flask(__name__)
    app.run(debug=True)

# for the website to get updates
@app.route("/")
def home():
    global data
    # return the new coordinates
    if len(data) > 0:
        return data

# listen to incoming connections and packets from driver.py
def listen():
    global data
    # use socket programming
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('127.0.0.1', 8082))
        s.listen()
        conn, addr = s.accept()
        while True:
            data = conn.recv(1024)



server_thead = threading.Thread(target=server)
commm_thread = threading.Thread(target=listen)