#!/usr/bin/python2.7 -tt
import socket
import re

HOST = '127.0.0.1'
BUFFER_SIZE = 1024
END_OF_HTTP = '\n\n'
ERROR_CODE = 404
OK_CODE = 200
ERROR_RESPONSE = '''HTTP/1.1 404 Not Found
Content-type: text/html
Content-length: 135

<html><head><title>Not Found</title></head><body>
Sorry, the object you requested was not found.
</body><html>'''
counter = 0


def parse_uri_from_request(request):
    pattern = re.compile(ur'^GET (.*) HTTP/\d.\d', re.IGNORECASE)
    try:
        uri = re.search(pattern, request).group(1)
    except Exception as e:
        print "Error in HTTP request"
        return ''
    return uri


def handle_request(request):
    global counter
    uri = parse_uri_from_request(request)
    if uri == '/counter':
        counter += 1
        return OK_CODE
    else:
        return ERROR_CODE


def send_response(socket, response_code):
    if response_code == OK_CODE:
        response = 'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: ' + \
            str(len(str(counter))) + '\r\n\r\n' + str(counter) + '\r\n\r\n'
    else:
        response = ERROR_RESPONSE

    MSG_LEN = len(response)

    total_sent = 0
    while total_sent < MSG_LEN:
        sent = socket.send(response[total_sent:])
        if sent == 0:
            print "Error sending response"
        total_sent += sent


def listen_for_request(s):
    data = s.recv(BUFFER_SIZE)
    while True:
        data += s.recv(BUFFER_SIZE)
        if END_OF_HTTP in data:
            result = handle_request(data)
            send_response(s, result)
            data = re.sub('.*' + END_OF_HTTP, '', data)


def connect_to_lb(remote_port):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, remote_port))
        listen_for_request(s)
    except Exception as e:
        print "Error, couldn't open socket", e
        raise e
    finally:
        s.close()


def main():
    import sys
    if len(sys.argv) != 2:
        print('Usage: %s <Remote load balancer port>' % sys.argv[0])
        sys.exit(1)
    assert sys.argv[1].isdigit()
    port = int(sys.argv[1], 10)
    connect_to_lb(port)

if __name__ == "__main__":
    main()
