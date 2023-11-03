import requests

# Specify the URL for the local server with the http:// scheme
url = "http://127.0.0.1:18000"

# Data to send in the POST request (change this as needed)
data = {
    'key1': 'value1',
    'key2': 'value2'
}

# Send a POST request to the local server
response = requests.post(url, data=data)

# Print the request being sent
request = response.request
print("Request Sent:")
print(f"Request Method: {request.method}")
print(f"Request URL: {request.url}")
print("Request Headers:")
for key, value in request.headers.items():
    print(f"{key}: {value}")
print("Request Data:")
print(request.body)  # Print the request body

# Check for a successful response (status code 200)
if response.status_code == 200:
    print("\nResponse Received:")
    # Print the response content from your local server
    print(response.text)
else:
    print(f"\nRequest failed with status code: {response.status_code}")
