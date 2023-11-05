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

# here the server sent a request but does not perform a reading using
# recv() so after a while the server should send an error 408 to
# stop trying to write to the client

# Simulate a request timeout by not reading the server's response
print("Simulating a request timeout...")

# Sleep for a duration longer than the specified timeout
timeout_duration = 65  # Ensure this is longer than the Timeout header value
time.sleep(timeout_duration)

# Close the socket to end the connection
client_socket.close()

print("Client is now inactive.")
