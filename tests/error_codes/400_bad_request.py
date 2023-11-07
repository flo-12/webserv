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


def test_missing_host() -> str:
    request_header = "GET / HTTP/1.1\r\n\r\n"
    http_response = send_request(request_header)
    if http_response.status != 400:
        return "BAD REQUEST: status code: {}, expected: {}".format(
            str(http_response.status), "400"
        )
    else:
        return "BAD REQUEST: Correct status code: {}".format(
            str(http_response.status)
        )		

response = test_missing_host()
print(response)

# many more examples produce an error 400:
# - double host
# - invalid host e.g. hyeyoo@hyeyoo.com