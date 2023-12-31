import socket
import time

# Server details (replace with your server's information)
server_host = "127.0.0.1"
server_port = 18000  # Use the correct port for your server

# Create an HTTP request with a long timeout
http_request = (
    "GET / HTTP/1.1\r\n"
    "Host: {}\r\n"
    "Connection: keep-alive\r\n"
    "Timeout: 60\r\n"  # Simulate a 60-second timeout (adjust as needed)
    "\r\n"
).format(server_host)

# Connect to the server
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((server_host, server_port))

print("Connected to the server")

# Send the HTTP request
client_socket.send(http_request.encode())

# Receive and read the server's response
response_data = b""
while True:
    chunk = client_socket.recv(4096)  # Adjust buffer size as needed
    if not chunk:
        break
    response_data += chunk

# Print the server's response
print("Server Response:")
print(response_data.decode())

# Close the socket to end the connection
client_socket.close()

print("Client is now inactive.")
