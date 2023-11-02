import requests

# Specify the URL to send GET requests to
url = "http://127.0.0.1:18000/"  # Replace with the actual URL

# Define the number of GET requests you want to send (change 'n' as needed)
n = 100

for i in range(n):
    response = requests.get(url)
    
    # Check for a successful response (status code 200)
    if response.status_code == 200:
        print(f"GET request {i + 1} was successful.")
    else:
        print(f"GET request {i + 1} failed with status code: {response.status_code}")
