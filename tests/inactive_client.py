import socket
import time

# Server details (replace with your server's information)
server_host = "127.0.0.1"
server_port = 18000

# Simulate inactivity for 30 seconds
inactive_duration = 30  # Change this as needed

# Connect to the server
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((server_host, server_port))

print("Connected to the server")

# Simulate inactivity
print("Simulating an inactive client for {} seconds...".format(inactive_duration))
time.sleep(inactive_duration)

# Close the socket to end the connection
client_socket.close()

print("Client is now inactive.")
