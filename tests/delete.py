import requests

# Specify the URL for the resource you want to delete
url = "http://127.0.0.1:18000/"  # Replace with the actual URL

# Send a DELETE request to the server
response = requests.delete(url)

# Check for a successful response (status code 204 typically indicates a successful deletion)
if response.status_code == 204:
    print("Resource successfully deleted.")
elif response.status_code == 404:
    print("Resource not found.")
else:
    print(f"Request failed with status code: {response.status_code}")
