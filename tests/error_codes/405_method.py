import sys

sys.path.append("../")
from http.client import HTTPResponse
import socket
import config


def send_request(request_header: str) -> str:
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((config.SERVER_ADDR, config.SERVER_PORT))
    client.send(request_header.encode())
    http_response = HTTPResponse(client)
    http_response.begin()
    return http_response

def method_not_allowed() -> str:
    request_header = "HEAD / HTTP/1.1\r\nHost: {}\r\n\r\n".format(config.SERVER_NAME)
    http_response = send_request(request_header)
    if http_response.status != 404:
        return "METHOD NOT ALLOWED: status code: {}, expected: {}".format(
            str(http_response.status), "405"
        )
    else:
        return "METHOD NOT ALLOWED: Correct status code: {}".format(
            str(http_response.status)
        )

response = method_not_allowed()
print(response)