import requests

# Specify the URL for the local server with the http:// scheme
url = "http://127.0.0.1:18000"

# Send a GET request to the local server
response = requests.get(url)

# Print the request being sent
request = response.request
print(f"Request Method: {request.method}")
print(f"Request URL: {request.url}")
print("Request Headers:")
for key, value in request.headers.items():
    print(f"{key}: {value}")

# Check for a successful response (status code 200)
if response.status_code == 200:
    # Print the response content from your local server
    print(response.text)
else:
    print(f"Request failed with status code: {response.status_code}")
