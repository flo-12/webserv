import sys

sys.path.append("../")
from http.client import HTTPResponse
import socket
import config


def send_request(request_header: str) -> str:
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((config.SERVER_ADDR, config.SERVER_PORT))
    client.send(request_header.encode())
    # read and parse http response
    http_response = HTTPResponse(client)
    http_response.begin()
    return http_response


def test_get_dir_index() -> str:
    request_header = "GET / HTTP/1.1\r\nHost: {}\r\n\r\n".format(config.SERVER_NAME)
    http_response = send_request(request_header)
    if http_response.status != 200:
        return "Bad status code: {}, expected: {}".format(
            str(http_response.status), "200"
        )
    body = http_response.read().decode("UTF-8")
    if body != "google":
        return "Bad content: {}, expected: {}".format(str(body), "google")
    return ""

response = test_get_dir_index()

print(response)

# def test_missing_host() -> str:
#     request_header = "GET / HTTP/1.1\r\n\r\n"
#     http_response = send_request(request_header)
#     if http_response.status != 400:
#         return "Bad status code: {}, expected: {}".format(
#             str(http_response.status), "400"
#         )
#     return ""