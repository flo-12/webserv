import requests

url = "http://127.0.0.1:18000"

print("testing status code 200:")
response = requests.get(url)
print(f"Status code: {response.status_code}")
print("")
print("")

# print("testing status code 404:")
# response = requests.get(url + "does_not_exist")
# print(f"Status code: {response.status_code}")
# print("")
# print("")