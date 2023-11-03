import requests

# Specify the URL for the HEAD request
url = "http://127.0.0.1:18000/"  # Replace with the actual URL

# Send a HEAD request to the server
response = requests.head(url)

# Check for a successful response (status code 200 or 204 typically indicate success)
if response.status_code == 200 or response.status_code == 204:
    print("HEAD request was successful.")
else:
    print(f"HEAD request failed with status code: {response.status_code}")
