import requests

# Specify the URL for your server
url = "http://127.0.0.1:18000"  # Update with your server's URL

# Define the data to be sent as a large payload
large_payload = "This is a very large payload. " * 10000  # Adjust the payload size as needed

# Define the headers for the request
headers = {
    'Content-Type': 'application/json',  # Adjust the content type as needed
}

# Use the data parameter to send the large payload in chunks
response = requests.post(url, data=large_payload, headers=headers, stream=True)

# Print the response status code
print(f"Response Status Code: {response.status_code}")

# Process the response content if needed
# For large responses, you may want to use response.iter_content() or response.iter_lines()
